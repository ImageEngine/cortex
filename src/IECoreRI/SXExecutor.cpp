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

#include "IECoreRI/SXExecutor.h"
#include "IECoreRI/SXTypeTraits.h"

using namespace IECoreRI;
using namespace IECore;
using namespace Imath;
using namespace std;

SXExecutor::SXExecutor( const ShaderVector *shaders, const ShaderVector *coshaders, const ShaderVector *lights )
	:	m_shaders( shaders ), m_coshaders( coshaders ), m_lights( lights )
{
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
	
	bool haveGrid = gridSize.x > 0 && gridSize.y > 0;
	if( haveGrid )
	{
		if( numPoints != (size_t)(gridSize.x * gridSize.y) )
		{
			throw Exception( boost::str( boost::format( "Wrong number of points (%d) for grid (%dx%d)." ) % numPoints % gridSize.x % gridSize.y ) );
		}
	}
	
	// create parameter list and set topology if we can.
	// we use a shared_ptr with a custom deleter to ensure that SxDestroyParameterList is called
	// no matter how we exit this function.
	
	boost::shared_ptr<void> vars( SxCreateParameterList( 0, numPoints, "current" ), SxDestroyParameterList );
	
	if( haveGrid )
	{
		unsigned nu = gridSize.x; unsigned nv = gridSize.y;
		SxSetParameterListGridTopology( vars.get(), 1, &nu, &nv );
	}

	// set input variables for coshaders
	if( m_coshaders )
	{
		for( unsigned shaderIndex = 0; shaderIndex < m_coshaders->size(); shaderIndex++ )
		{
			setVariables( vars.get(), (*m_coshaders)[shaderIndex], 0, points, numPoints );
		}
	}

	// loop over the shaders, running them and extracting results as we go
	CompoundDataPtr result = new CompoundData();
	
	SxShader previousShader = 0;
	for( unsigned shaderIndex = 0; shaderIndex < m_shaders->size(); shaderIndex++ )
	{
	
		// set parameters from input data 
		
		setVariables( vars.get(), (*m_shaders)[shaderIndex], previousShader, points, numPoints );
		
		// run shader

		SxCallShader( 
			(*m_shaders)[shaderIndex],
			vars.get(),
			m_lights ? (void **)&((*m_lights)[0]) : 0,
			m_lights ? m_lights->size() : 0,
			m_coshaders ? (void **)&((*m_coshaders)[0]) : 0,
			m_coshaders ? m_coshaders->size() : 0
		);

		// extract and store output data

		unsigned numParameters = SxGetNumParameters( (*m_shaders)[shaderIndex] );
		for( unsigned i=0; i<numParameters; i++ )
		{
			SxType type;
			bool varying;
			SxData defaultValue;
			unsigned arraySize;
			const char *spaceName;
			bool output;
			const char *name = SxGetParameterInfo( (*m_shaders)[shaderIndex], i, &type, &varying, &defaultValue, &arraySize, &spaceName, &output );

			if( output )
			{
				switch( type )
				{
					case SxFloat :
					{
						getVariable<FloatVectorData>( (*m_shaders)[shaderIndex], name, result );
						break;	
					}
					case SxColor :
					{
						getVariable<Color3fVectorData>( (*m_shaders)[shaderIndex], name, result );
						break;	
					}
					case SxVector :
					case SxPoint :
					case SxNormal :
					{
						getVariable<V3fVectorData>( (*m_shaders)[shaderIndex], name, result );
						break;	
					}
					default :
						throw Exception( boost::str( boost::format( "Output parameter \"%s\" has unsupported type." ) % name ) );

				}
			}
		}

		getVariable<Color3fVectorData>( (*m_shaders)[shaderIndex], "Ci", result );
		getVariable<Color3fVectorData>( (*m_shaders)[shaderIndex], "Oi", result );
		getVariable<V3fVectorData>( (*m_shaders)[shaderIndex], "P", result );
		getVariable<V3fVectorData>( (*m_shaders)[shaderIndex], "N", result );
		
		previousShader = (*m_shaders)[shaderIndex];
		
	}
				
	return result;
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

void SXExecutor::setVariables( SxParameterList parameterList, SxShader targetShader, SxShader previousShader, const IECore::CompoundData *points, size_t expectedSize ) const
{

	// first set the predefined parameters (like "P" etc)
	
	unsigned numPredefinedParameters = SxGetPredefinedParameters( targetShader, 0, 0 );
	std::vector<const char *> predefinedParameters( numPredefinedParameters );
	SxGetPredefinedParameters( targetShader, &(predefinedParameters[0]), numPredefinedParameters );

	for( unsigned i=0; i<numPredefinedParameters; i++ )
	{				
		SxType type = predefinedParameterType( predefinedParameters[i] );
		if( type==SxInvalid )
		{
			throw Exception( boost::str( boost::format( "Unknown predefined parameter \"%s\"" ) % predefinedParameters[i] ) ); 
		}
		setVariable( parameterList, predefinedParameters[i], type, true, previousShader, points, expectedSize );
	}
	
	// then set the arbitrary parameters defined by the shader.
	
	unsigned numParameters = SxGetNumParameters( targetShader );
	for( unsigned i=0; i<numParameters; i++ )
	{
		SxType type;
		bool varying;
		SxData defaultValue;
		unsigned arraySize;
		const char *spaceName;
		bool output;
		const char *name = SxGetParameterInfo( targetShader, i, &type, &varying, &defaultValue, &arraySize, &spaceName, &output );
		if( varying && !output )
		{
			setVariable( parameterList, name, type, false, 0, points, expectedSize );
		}
	}
}

void SXExecutor::setVariable( SxParameterList parameterList, const char *name, SxType type, bool predefined, SxShader previousShader, const IECore::CompoundData *points, size_t expectedSize ) const
{
	switch( type )
	{
		case SxFloat :
			setVariable2<SxFloat>( parameterList, name, predefined, previousShader, points, expectedSize );
			break;
		case SxColor :
			setVariable2<SxColor>( parameterList, name, predefined, previousShader, points, expectedSize );
			break;
		case SxPoint :
			setVariable2<SxPoint>( parameterList, name, predefined, previousShader, points, expectedSize );
			break;
		case SxVector :
			setVariable2<SxVector>( parameterList, name, predefined, previousShader, points, expectedSize );
			break;
		case SxNormal :
			setVariable2<SxNormal>( parameterList, name, predefined, previousShader, points, expectedSize );
			break;	
		default :
			throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % name ) );
	}
}

template<SxType sxType>
void SXExecutor::setVariable2( SxParameterList parameterList, const char *name, bool predefined, SxShader previousShader, const IECore::CompoundData *points, size_t expectedSize ) const
{
	void *data = 0;
	size_t size = 0;
		
	// try to get data based on the output of the previous shader.
	// this shouldn't be necessary when we get a version of 3delight which places the output of the
	// shader in the parameter list itself.
	if( previousShader )
	{
		size = SxGetWritableParameterInfo( previousShader, name, &data );
	}
	
	// if that failed for any reason then get data based on the input points provided by the caller
	if( !data )
	{
		const Data *d = points->member<Data>( name );
		if( d )
		{
			typedef typename SXTypeTraits<sxType>::VectorDataType DataType;
			const DataType *td = IECore::runTimeCast<const DataType>( d );
			if( !td )
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be %s)." ) % name % d->typeName() % DataType::staticTypeName() ) ) );
			}

			size = td->readable().size();
			data = (void *)&(td->readable()[0]);
		}
	}
	
	// check data and size and set variable from data if possible
	if( !data )
	{
		return;
	}
	else if( size != expectedSize )
	{
		throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong size (%d but should be %d)." ) % name % size % expectedSize ) ) );
	}
	
	if( predefined )
	{
		SxSetPredefinedParameter( parameterList, name, data );
	}
	else
	{
		SxSetParameter( parameterList, name, sxType, data, true );
	}
	
}
		
template<class T>
void SXExecutor::getVariable( SxShader shader, const char *name, IECore::CompoundData *container ) const
{
	void *d = 0;
	int numPoints = SxGetWritableParameterInfo( shader, name, &d );
	if( d && numPoints )
	{
		typename T::Ptr data = new T;
		data->writable().resize( numPoints );
		memcpy( &(data->writable()[0]), d, numPoints * sizeof( typename T::ValueType::value_type ) );
		container->writable()[name] = data;
	}
}
