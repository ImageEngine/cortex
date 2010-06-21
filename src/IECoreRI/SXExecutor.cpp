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

IECore::TypeId SXExecutor::predefinedParameterType( const char *name ) const
{
	if(
		strcmp( name, "P" )==0 ||
		strcmp( name, "N" )==0 ||
		strcmp( name, "Ng" )==0 ||
		strcmp( name, "I" )==0
	)
	{
		return V3fVectorDataTypeId;
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
		return FloatVectorDataTypeId;
	}
	else if(
		strcmp( name, "Cs" )==0 ||
		strcmp( name, "Os" )==0 ||
		strcmp( name, "Ci" )==0 ||
		strcmp( name, "Oi" )==0 ||
		strcmp( name, "Cl" )==0 
	)
	{
		return Color3fVectorDataTypeId;
	}
	
	throw Exception( boost::str( boost::format( "Unknown predefined parameter \"%s\"" ) % name ) ); 
}

void SXExecutor::setVariables( SxParameterList parameterList, SxShader targetShader, SxShader previousShader, const IECore::CompoundData *points, size_t expectedSize ) const
{
	unsigned numPredefinedParameters = SxGetPredefinedParameters( targetShader, 0, 0 );
	std::vector<const char *> predefinedParameters( numPredefinedParameters );
	SxGetPredefinedParameters( targetShader, &(predefinedParameters[0]), numPredefinedParameters );

	for( unsigned i=0; i<numPredefinedParameters; i++ )
	{				
		IECore::TypeId expectedType = predefinedParameterType( predefinedParameters[i] );

		switch( expectedType )
		{
			case FloatVectorDataTypeId :
			{
				setVariable<FloatVectorData>( parameterList, predefinedParameters[i], previousShader, points, expectedSize );
				break;
			}
			case V3fVectorDataTypeId :
			{
				setVariable<V3fVectorData>( parameterList, predefinedParameters[i], previousShader, points, expectedSize );
				break;
			}
			case Color3fVectorDataTypeId :
			{
				setVariable<Color3fVectorData>( parameterList, predefinedParameters[i], previousShader, points, expectedSize );
				break;
			}
			default :
				throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % predefinedParameters[i] ) );
		}
	}
}

template<class T>
void SXExecutor::setVariable( SxParameterList parameterList, const char *name, SxShader previousShader, const CompoundData *points, size_t expectedSize ) const
{
	// try to set the variable based on the output of the previous shader
	if( previousShader )
	{
		void *d = 0;
		size_t numPoints = SxGetWritableParameterInfo( previousShader, name, &d );
		if( d && numPoints==expectedSize )
		{
			SxSetPredefinedParameter( parameterList, name, d );
			return;
		}
	}

	// if that failed for any reason then set the variable based on the input data provided by the caller
	const Data *d = points->member<Data>( name );
	if( d )
	{
		const T *td = IECore::runTimeCast<const T>( d );
		if( !td )
		{
			throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be %s)." ) % d->typeName() % T::staticTypeName() ) ) );
		}
		
		if( td )
		{
			size_t s = td->readable().size();
			if( s != expectedSize )
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong size (%d but should be %d)." ) % name % s % expectedSize ) ) );
			}
			
			SxSetPredefinedParameter( parameterList, name, (void *)&(td->readable()[0]) );
		}
	}
	else
	{
		// no data available, fall back to default values if we don't think they'll
		// be provided by attributes. for some things (like "t") the Sx library will
		// crash if they're not specified, but for others (like "Cs") it'll provide
		// a default value from the attribute state and we don't want to overwrite that.
		// others still (like "Ci") are for output only as far as i know so we don't
		// want to set them either. others still ("u", "du" etc) are provided by 3delight
		// when there is a grid topology specified. all this should go away in 3delight 9.0.44
		// where the library itself will be providing default values and the crashes will
		// be gone.
		if(
			0==strcmp( name, "Cs" ) || 0==strcmp( name, "Os" ) ||
			0==strcmp( name, "Ci" ) || 0==strcmp( name, "Oi" ) ||
			0==strcmp( name, "u" ) || 0==strcmp( name, "v" ) ||
			0==strcmp( name, "du" ) || 0==strcmp( name, "dv" )
		)
		{
			return;
		}
		typename T::ValueType defaultValue; defaultValue.resize( expectedSize, typename T::ValueType::value_type( 0 ) );
		SxSetPredefinedParameter( parameterList, name, &(defaultValue[0]) );
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
