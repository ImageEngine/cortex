//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/TriangulateOp.h"
#include "IECore/FileNameParameter.h"

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
	
	// create a renderer with automatic instancing disabled, as we want to explicitly create an instanceable object:
	RendererPtr renderer = new IECoreRI::Renderer();
	renderer->setAttribute( "ri:automaticInstancing", new IECore::BoolData( false ) );
	
	RtObjectHandle objectHandle = RiObjectBegin();
	
		primitive->render( renderer.get() );
	
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
	GxFreeGeometry( m_geo );
	RiEnd();
	if( c!=m_context )
	{
		RiContext( c );
	}
}

unsigned GXEvaluator::numFaces() const
{
	ScopedContext context( m_context );
	return GxGetFaceCount( m_geo );
}

template<typename T>
static DataPtr evaluatePrimitiveVariable( std::vector<GxSurfacePoint> &points, const std::string &primVarName )
{
	typename T::Ptr result = new T;
	result->writable().resize( points.size() );
	if( points.size() )
	{
		GxEvaluateSurface( points.size(), &(points[0]), primVarName.c_str(), result->baseSize() / points.size(), result->baseWritable() );
	}
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
	
	validatePrimVarNames( primVarNames );
	
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

IECore::CompoundDataPtr GXEvaluator::evaluate( const IECore::FloatVectorData *s, const IECore::FloatVectorData *t, const std::vector<std::string> &primVarNames ) const
{
	size_t numPoints = s->readable().size();
	if( t->readable().size() != numPoints )
	{
		throw InvalidArgumentException( "s and t must have the same length" );
	}
		
	buildSTEvaluator();
	
	MeshPrimitiveEvaluator::ResultPtr evaluatorResult = boost::static_pointer_cast<MeshPrimitiveEvaluator::Result>( m_stEvaluator->createResult() );
	IntVectorDataPtr fData = new IntVectorData;
	FloatVectorDataPtr uData = new FloatVectorData;
	FloatVectorDataPtr vData = new FloatVectorData;
	BoolVectorDataPtr statusData = new BoolVectorData;
	std::vector<int> &fWritable = fData->writable(); fWritable.resize( numPoints );
	std::vector<float> &uWritable = uData->writable(); uWritable.resize( numPoints );
	std::vector<float> &vWritable = vData->writable(); vWritable.resize( numPoints );
	std::vector<bool> &statusWritable = statusData->writable(); statusWritable.resize( numPoints );
	
	const std::vector<float> &sReadable = s->readable();
	const std::vector<float> &tReadable = t->readable();
	
	const PrimitiveVariable &uPrimVar = m_stEvaluator->primitive()->variables.find( "u" )->second;
	const PrimitiveVariable &vPrimVar = m_stEvaluator->primitive()->variables.find( "v" )->second;
	for( size_t i=0; i<numPoints; i++ )
	{
		bool success = m_stEvaluator->pointAtUV( Imath::V2f( sReadable[i], tReadable[i] ), evaluatorResult.get() );
		// dividing by 2 maps from the triangle index to the original face index of the mesh before it
		// was triangulated - we can guarantee this because the original mesh was all quads.
		fWritable[i] = success ? evaluatorResult->triangleIndex() / 2 : 0;
		uWritable[i] = success ? evaluatorResult->floatPrimVar( uPrimVar ) : 0;
		vWritable[i] = success ? evaluatorResult->floatPrimVar( vPrimVar ) : 0;
		statusWritable[i] = success;
	}

	CompoundDataPtr result = evaluate( fData.get(), uData.get(), vData.get(), primVarNames );
	result->writable()["gxStatus"] = statusData;
	
	return result;
}

void GXEvaluator::validatePrimVarNames( const std::vector<std::string> &primVarNames ) const
{
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
}

void GXEvaluator::buildSTEvaluator() const
{
	Mutex::scoped_lock lock( m_stEvaluatorMutex, false ); // read only lock
	if( m_stEvaluator )
	{
		return;
	}
	lock.upgrade_to_writer();
	if( m_stEvaluator )
	{
		return;
	}
		
	size_t nFaces = numFaces();
	size_t nVertices = nFaces * 4;
	
	IntVectorDataPtr faceIndicesData = new IntVectorData;
	FloatVectorDataPtr uData = new FloatVectorData;
	FloatVectorDataPtr vData = new FloatVectorData;
	std::vector<int> &faceIndicesWritable = faceIndicesData->writable(); faceIndicesWritable.resize( nVertices );
	std::vector<float> &uWritable = uData->writable(); uWritable.resize( nVertices );
	std::vector<float> &vWritable = vData->writable(); vWritable.resize( nVertices );
	
	size_t vi = 0;
	for( size_t fi=0; fi<nFaces; fi++ )
	{
		faceIndicesWritable[vi] = fi;
		faceIndicesWritable[vi+1] = fi;
		faceIndicesWritable[vi+2] = fi;
		faceIndicesWritable[vi+3] = fi;
		uWritable[vi] = 0;
		uWritable[vi+1] = 0;
		uWritable[vi+2] = 1;
		uWritable[vi+3] = 1;
		vWritable[vi] = 0;
		vWritable[vi+1] = 1;
		vWritable[vi+2] = 1;
		vWritable[vi+3] = 0;
		vi += 4;
	}
	
	std::vector<std::string> primVarNames;
	primVarNames.push_back( "P" );
	primVarNames.push_back( "s" );
	primVarNames.push_back( "t" );
	CompoundDataPtr vertexData = evaluate( faceIndicesData.get(), uData.get(), vData.get(), primVarNames );
	
	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	IntVectorDataPtr vertexIdsData = new IntVectorData;
	std::vector<int> &verticesPerFace = verticesPerFaceData->writable(); verticesPerFace.resize( nFaces, 4 );
	std::vector<int> &vertexIds = vertexIdsData->writable(); vertexIds.resize( nVertices );
	for( size_t i=0; i<nVertices; i++ )
	{
		vertexIds[i] = i;
	}
	MeshPrimitivePtr mesh = new MeshPrimitive( verticesPerFaceData, vertexIdsData );
	mesh->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, vertexData->member<V3fVectorData>( "P" ) );
	mesh->variables["s"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, vertexData->member<FloatVectorData>( "s" ) );
	mesh->variables["t"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, vertexData->member<FloatVectorData>( "t" ) );
	mesh->variables["u"] = PrimitiveVariable( PrimitiveVariable::Vertex, uData );
	mesh->variables["v"] = PrimitiveVariable( PrimitiveVariable::Vertex, vData );
	
	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( mesh );
	op->copyParameter()->setTypedValue( false );
	op->throwExceptionsParameter()->setTypedValue( false );
	op->operate();
	
	m_stEvaluator = new MeshPrimitiveEvaluator( mesh );
}
