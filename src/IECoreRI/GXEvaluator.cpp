//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECoreRI/GXEvaluator.h"
#include "IECoreRI/Renderer.h"
#include "IECoreRI/ScopedContext.h"

using namespace std;
using namespace IECore;
using namespace IECoreRI;

GXEvaluator::GXEvaluator( const IECore::Primitive *primitive )
{
	RtContextHandle prevContext = RiGetContext();
	RiBegin( 0 );
	m_context = RiGetContext();
	
	RendererPtr renderer = new IECoreRI::Renderer();
	RtObjectHandle objectHandle = RiObjectBegin();
	
		primitive->render( renderer );
	
	RiObjectEnd();
	
	m_geo = GxGetGeometry( objectHandle );

	RiContext( prevContext );
	
	for( PrimitiveVariableMap::const_iterator it=primitive->variables.begin(); it!=primitive->variables.end(); it++ )
	{
		if( !it->second.data )
		{
			continue;
		}
		IECore::TypeId type = it->second.data->typeId();
		if( type!=V3fVectorDataTypeId && type!=Color3fVectorDataTypeId && type!=FloatVectorDataTypeId )
		{
			m_primitiveVariableTypes[it->first] = InvalidTypeId;
		}
		else
		{
			m_primitiveVariableTypes[it->first] = type;
		}
	}
	m_primitiveVariableTypes["N"] = V3fVectorDataTypeId;     // these can be queried even if a primitive variable
	m_primitiveVariableTypes["Ng"] = V3fVectorDataTypeId;    // of that name hasn't been supplied.
	m_primitiveVariableTypes["dPdu"] = V3fVectorDataTypeId;
	m_primitiveVariableTypes["dPdv"] = V3fVectorDataTypeId;
}

GXEvaluator::~GXEvaluator()
{
	RtContextHandle c = RiGetContext();
	RiContext( m_context );
	RiEnd();
	if( c!=m_context )
	{
		RiContext( c );
	}
}

unsigned GXEvaluator::numFaces()
{
	ScopedContext context( m_context );
	return GxGetFaceCount( m_geo );
}

template<typename T>
static DataPtr evaluatePrimitiveVariable( std::vector<GxSurfacePoint> &points, const std::string &primVarName )
{
	typename T::Ptr result = new T;
	result->writable().resize( points.size() );
	GxEvaluateSurface( points.size(), &(points[0]), primVarName.c_str(), result->baseSize() / points.size(), result->baseWritable() );
	return result;
}

IECore::CompoundDataPtr GXEvaluator::evaluate( const IECore::IntVectorData *faceIndices, const IECore::FloatVectorData *u, const IECore::FloatVectorData *v, const std::vector<std::string> &primVarNames ) const
{
	// do some basic validation of our input

	size_t numPoints = faceIndices->readable().size();
	if( u->readable().size() != numPoints || v->readable().size() != numPoints )
	{
		throw InvalidArgumentException( "faceIndices, u and v must all have the same length" );
	}
	for( vector<string>::const_iterator it=primVarNames.begin(); it!=primVarNames.end(); it++ )
	{
		PrimitiveVariableTypeMap::const_iterator pvIt = m_primitiveVariableTypes.find( *it );
		if( pvIt == m_primitiveVariableTypes.end() )
		{
			throw InvalidArgumentException( boost::str( boost::format( "Primitive variable \"%s\" does not exist" ) % *it ) );
		}
		if( pvIt->second==InvalidTypeId )
		{
			throw InvalidArgumentException( boost::str( boost::format( "Primitive variable \"%s\" has unsupported type" ) % *it ) );
		}
	}

	// create surface points
	ScopedContext context( m_context );

	const std::vector<int> faceIndicesReadable = faceIndices->readable();
	const std::vector<float> uReadable = u->readable();
	const std::vector<float> vReadable = v->readable();
	
	std::vector<GxSurfacePoint> surfacePoints( numPoints );
	for( unsigned i=0; i<numPoints; i++ )
	{
		int status = GxCreateSurfacePoint( m_geo, faceIndicesReadable[i], uReadable[i], vReadable[i], 0.0f, &(surfacePoints[i]) );
		if( status==RIE_RANGE )
		{
			for( unsigned j=0; j<i; j++ )
			{
				GxFreeSurfacePoint( surfacePoints[j] );				
			}
			throw InvalidArgumentException( boost::str( boost::format( "faceIndex %d out of range" ) % faceIndicesReadable[i] ) );
		}
	}

	// query all the primvars

	IECore::CompoundDataPtr result = new IECore::CompoundData;
	for( vector<string>::const_iterator it=primVarNames.begin(); it!=primVarNames.end(); it++ )
	{
		switch( m_primitiveVariableTypes.find( *it )->second )
		{
			case FloatVectorDataTypeId :
				result->writable()[*it] = evaluatePrimitiveVariable<FloatVectorData>( surfacePoints, *it );
				break;
			case V3fVectorDataTypeId :
				result->writable()[*it] = evaluatePrimitiveVariable<V3fVectorData>( surfacePoints, *it );
				break;
			case Color3fVectorDataTypeId :
				result->writable()[*it] = evaluatePrimitiveVariable<Color3fVectorData>( surfacePoints, *it );
				break;
			default :
				assert( 0 ); // shouldnt be here as we checked types earlier
		}
	}

	// clean up

	for( unsigned i=0; i<numPoints; i++ )
	{
		GxFreeSurfacePoint( surfacePoints[i] );
	}
	
	return result;
}
