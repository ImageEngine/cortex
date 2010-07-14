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

#include "boost/shared_ptr.hpp"

#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

#include "IECoreRI/SXExecutor.h"
#include "IECoreRI/SXTypeTraits.h"

using namespace IECoreRI;
using namespace IECore;
using namespace Imath;
using namespace std;

SXExecutor::SXExecutor( const ShaderVector &shaders, SxContext context, const ShaderVector &coshaders, const ShaderVector &lights )
	:	m_shaders( shaders ), m_context( context ), m_coshaders( coshaders ), m_lights( lights )
{
	// build the map from parameter names to the expected type for that parameter
	for( ShaderVector::const_iterator it=shaders.begin(); it!=shaders.end(); ++it )
	{
		storeParameterTypes( *it );
	}
	for( ShaderVector::const_iterator it=coshaders.begin(); it!=coshaders.end(); ++it )
	{
		storeParameterTypes( *it );
	}
	for( ShaderVector::const_iterator it=lights.begin(); it!=lights.end(); ++it )
	{
		storeParameterTypes( *it );
	}
}

IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points ) const
{
	V2i gridSize( 0 );
	return execute( points, gridSize );
}
		
IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const
{
	// decide how many points we're shading and validate the grid size

	const V3fVectorData *p = points->member<V3fVectorData>( "P", true /* throw */ );
	size_t numPoints = p->readable().size();
	if( !numPoints )
	{
		throw Exception( "\"P\" has zero length." );
	}
	
	bool haveGrid = gridSize.x > 0 && gridSize.y > 0;
	if( haveGrid )
	{
		if( numPoints != (size_t)(gridSize.x * gridSize.y) )
		{
			throw Exception( boost::str( boost::format( "Wrong number of points (%d) for grid (%dx%d)." ) % numPoints % gridSize.x % gridSize.y ) );
		}
	}
	
	// create parameter list for the grid and set topology if we can.
		
	boost::shared_ptr<void> vars( SxCreateParameterList( m_context, numPoints, "current" ), SxDestroyParameterList );
	
	if( haveGrid )
	{
		unsigned nu = gridSize.x; unsigned nv = gridSize.y;
		SxSetParameterListGridTopology( vars.get(), 1, &nu, &nv );
	}

	// fill the grid from our input data.

	setVariables( vars.get(), points, numPoints );
	
	// run all the shaders in sequence on the grid
	
	CompoundDataPtr result = new CompoundData();
	for( unsigned shaderIndex = 0; shaderIndex < m_shaders.size(); ++shaderIndex )
	{
	
		SxCallShader( 
			m_shaders[shaderIndex],
			vars.get()
		);

	}

	// and retrieve the results

	return getVariables( vars.get() );

}

void SXExecutor::storeParameterTypes( SxShader shader )
{
	unsigned numParameters = SxGetNumParameters( shader );
	for( unsigned i=0; i<numParameters; i++ )
	{
		SxType type;
		bool varying;
		SxData defaultValue;
		unsigned arraySize;
		const char *spaceName;
		bool output;
		const char *name = SxGetParameterInfo( shader, i, &type, &varying, &defaultValue, &arraySize, &spaceName, &output );
		if( varying )
		{
			TypeMap &typeMap = output ? m_outputParameterTypes : m_inputParameterTypes;
			
			TypeMap::const_iterator it = typeMap.find( name );
			if( it != typeMap.end() )
			{
				if( it->second != type )
				{
					msg( Msg::Warning, "SXExecutor::storeParameterTypes", boost::format( "Shaders request conflicting types for parameter \"%s\"" ) % name );
				}
				continue;
			}
			typeMap[name] = type;
		}
	}
}

SxType SXExecutor::predefinedParameterType( const char *name ) const
{
	if(
		strcmp( name, "P" )==0
	)
	{
		return SxPoint;
	}
	else if( 
		strcmp( name, "N" )==0 ||
		strcmp( name, "Ng" )==0
	)
	{
		return SxNormal;
	}
	else if(
		strcmp( name, "I" )==0
	)
	{
		return SxVector;
	}	
	else if(
		strcmp( name, "s" )==0 ||
		strcmp( name, "t" )==0 ||
		strcmp( name, "u" )==0 ||
		strcmp( name, "v" )==0 ||
		strcmp( name, "du" )==0 ||
		strcmp( name, "dv" )==0
	)
	{
		return SxFloat;
	}
	else if(
		strcmp( name, "Cs" )==0 ||
		strcmp( name, "Os" )==0 ||
		strcmp( name, "Ci" )==0 ||
		strcmp( name, "Oi" )==0 ||
		strcmp( name, "Cl" )==0 
	)
	{
		return SxColor;
	}
	
	return SxInvalid;
}

SxType SXExecutor::assumedParameterType( IECore::TypeId type ) const
{
	switch( type )
	{
		case FloatVectorDataTypeId :
			return SxFloat;
		case V3fVectorDataTypeId :
			return SxVector;
		case Color3fVectorDataTypeId :
			return SxColor;
		default :
			return SxInvalid;
	}
}

void SXExecutor::setVariables( SxParameterList parameterList, const IECore::CompoundData *points, size_t expectedSize ) const
{
	for( CompoundDataMap::const_iterator it=points->readable().begin(); it!=points->readable().end(); it++ )
	{
		SxType type = predefinedParameterType( it->first.value().c_str() );
		if( type != SxInvalid )
		{
			setVariable( parameterList, it->first.value().c_str(), type, true, it->second, expectedSize );
		}
		else
		{
			TypeMap::const_iterator tIt = m_inputParameterTypes.find( it->first );
			if( tIt != m_inputParameterTypes.end() )
			{
				setVariable( parameterList, it->first.value().c_str(), tIt->second, false, it->second, expectedSize );
			}
			else
			{
				// variable isn't a parameter to any of the shaders, but we'd like to pass it in anyway, so it can
				// be used with getvar(). guess a type and go for it.
				type = assumedParameterType( it->second->typeId() );
				if( type!=SxInvalid )
				{
					setVariable( parameterList, it->first.value().c_str(), type, false, it->second, expectedSize );
				}
				else
				{
					throw Exception( boost::str( boost::format( "Unsupported parameter type \"%s\"" ) % it->second->typeName() ) );
				}
			}
		}
	}
}

void SXExecutor::setVariable( SxParameterList parameterList, const char *name, SxType type, bool predefined, const IECore::Data *data, size_t expectedSize ) const
{
	switch( type )
	{
		case SxFloat :
			setVariable2<SxFloat>( parameterList, name, predefined, data, expectedSize );
			break;
		case SxColor :
			setVariable2<SxColor>( parameterList, name, predefined, data, expectedSize );
			break;
		case SxPoint :
			setVariable2<SxPoint>( parameterList, name, predefined, data, expectedSize );
			break;
		case SxVector :
			setVariable2<SxVector>( parameterList, name, predefined, data, expectedSize );
			break;
		case SxNormal :
			setVariable2<SxNormal>( parameterList, name, predefined, data, expectedSize );
			break;	
		default :
			throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % name ) );
	}
}

template<SxType sxType>
void SXExecutor::setVariable2( SxParameterList parameterList, const char *name, bool predefined, const IECore::Data *data, size_t expectedSize ) const
{
	
	typedef typename SXTypeTraits<sxType>::VectorDataType DataType;
	const DataType *td = IECore::runTimeCast<const DataType>( data );
	if( !td )
	{
		throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be %s)." ) % name % data->typeName() % DataType::staticTypeName() ) ) );
	}

	size_t size = td->readable().size();
	if( size != expectedSize )
	{
		throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong size (%d but should be %d)." ) % name % size % expectedSize ) ) );
	}
	
	void *rawData = (void *)&(td->readable()[0]);
	if( predefined )
	{
		SxSetPredefinedParameter( parameterList, name, rawData );
	}
	else
	{
		SxSetParameter( parameterList, name, sxType, rawData, true );
	}
	
}

IECore::CompoundDataPtr SXExecutor::getVariables( SxParameterList parameterList ) const
{
	CompoundDataPtr result = new CompoundData;
	for( TypeMap::const_iterator it=m_outputParameterTypes.begin(); it!=m_outputParameterTypes.end(); ++it )
	{
		switch( it->second )
		{
			case SxFloat :
				getVariable<SxFloat>( parameterList, it->first.value().c_str(), result );
				break;
			case SxColor :
				getVariable<SxColor>( parameterList, it->first.value().c_str(), result );
				break;
			case SxPoint :
				getVariable<SxPoint>( parameterList, it->first.value().c_str(), result );
				break;
			case SxVector :
				getVariable<SxVector>( parameterList, it->first.value().c_str(), result );
				break;
			case SxNormal :
				getVariable<SxNormal>( parameterList, it->first.value().c_str(), result );
				break;				
			default :
				throw Exception( boost::str( boost::format( "Output parameter \"%s\" has unsupported type." ) % it->first.value() ) );
		}
	}

	getVariable<SxColor>( parameterList, "Ci", result );
	getVariable<SxColor>( parameterList, "Oi", result );
	getVariable<SxPoint>( parameterList, "P", result );
	getVariable<SxNormal>( parameterList, "N", result );
	
	return result;						
}
										
template<SxType sxType>
void SXExecutor::getVariable( SxParameterList parameterList, const char *name, IECore::CompoundData *result ) const
{
	void *d = 0;
	int numPoints = SxGetParameter( parameterList, name, &d );
	if( d && numPoints )
	{
		typedef typename SXTypeTraits<sxType>::VectorDataType DataType;
		typename DataType::Ptr data = new DataType;
		data->writable().resize( numPoints );
		memcpy( &(data->writable()[0]), d, numPoints * sizeof( typename DataType::ValueType::value_type ) );
		result->writable()[name] = data;
	}
}

