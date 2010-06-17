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

#include "IECore/VectorTypedData.h"

#include "IECoreRI/SXExecutor.h"

using namespace IECoreRI;
using namespace IECore;
using namespace Imath;
using namespace std;

SXExecutor::SXExecutor( SxShader shader )
	:	m_shader( shader )
{
}
		
IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points )
{
	// decide how many points we're shading

	const V3fVectorData *p = points->member<V3fVectorData>( "P", true /* throw */ );
	size_t numPoints = p->readable().size();
	
	// create parameter list and fill it from input data

	unsigned numPredefinedParameters = SxGetPredefinedParameters( m_shader, 0, 0 );
	std::vector<const char *> predefinedParameters( numPredefinedParameters );
	SxGetPredefinedParameters( m_shader, &(predefinedParameters[0]), numPredefinedParameters );

	SxParameterList vars = SxCreateParameterList( 0, numPoints, "current" );
		
	for( unsigned i=0; i<numPredefinedParameters; i++ )
	{				
		IECore::TypeId expectedType = predefinedParameterType( predefinedParameters[i] );

		const Data *d = points->member<Data>( predefinedParameters[i] );

		if( d && !d->isInstanceOf( expectedType ) )
		{
			throw Exception( boost::str( boost::format( "Variable \"%s\" has wrong type (\"%s\" but should be \"%s\")." ) % predefinedParameters[i] % d->typeName() % RunTimeTyped::typeNameFromTypeId( expectedType ) ) );
		}

		switch( expectedType )
		{
			case FloatVectorDataTypeId :
			{
				setVariable<FloatVectorData>( vars, predefinedParameters[i], d, numPoints );
				break;
			}
			case V3fVectorDataTypeId :
			{
				setVariable<V3fVectorData>( vars, predefinedParameters[i], d, numPoints );
				break;
			}
			case Color3fVectorDataTypeId :
			{
				setVariable<Color3fVectorData>( vars, predefinedParameters[i], d, numPoints );
				break;
			}
			default :
				throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % predefinedParameters[i] ) );
		}

	}
		
	// run shader

	SxCallShader( m_shader, vars, 0, 0, 0, 0 );
	
	// create and return output data
			
	CompoundDataPtr result = new CompoundData();
	
	unsigned numParameters = SxGetNumParameters( m_shader );
	for( unsigned i=0; i<numParameters; i++ )
	{
		SxType type;
		bool varying;
		SxData defaultValue;
		unsigned arraySize;
		const char *spaceName;
		bool output;
		const char *name = SxGetParameterInfo( m_shader, i, &type, &varying, &defaultValue, &arraySize, &spaceName, &output );
		if( output )
		{
			switch( type )
			{
				case SxFloat :
				{
					result->writable()[name] = getVariable<FloatVectorData>( vars, name, numPoints );
					break;	
				}
				case SxColor :
				{
					result->writable()[name] = getVariable<Color3fVectorData>( vars, name, numPoints );
					break;	
				}
				case SxVector :
				case SxPoint :
				case SxNormal :
				{
					result->writable()[name] = getVariable<V3fVectorData>( vars, name, numPoints );
					break;	
				}
				default :
					throw Exception( boost::str( boost::format( "Output parameter \"%s\" has unsupported type." ) % name ) );

			}
		}
	}

	result->writable()["Ci"] = getVariable<Color3fVectorData>( vars, "Ci", numPoints );
	result->writable()["Oi"] = getVariable<Color3fVectorData>( vars, "Oi", numPoints );
				
	return result;
}

IECore::TypeId SXExecutor::predefinedParameterType( const char *name )
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
		strcmp( name, "t" )==0
	)
	{
		return FloatVectorDataTypeId;
	}
	else if(
		strcmp( name, "Cs" )==0 ||
		strcmp( name, "Os" )==0 ||
		strcmp( name, "Ci" )==0 ||
		strcmp( name, "Oi" )==0
	)
	{
		return Color3fVectorDataTypeId;
	}

	throw Exception( boost::str( boost::format( "Unknown predefined parameter \"%s\"" ) % name ) ); 
}

template<class T>
void SXExecutor::setVariable( SxParameterList parameterList, const char *name, const Data *d, size_t expectedSize )
{

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
		// want to set them either.
		if(
			0==strcmp( name, "Cs" ) || 0==strcmp( name, "Os" ) ||
			0==strcmp( name, "Ci" ) || 0==strcmp( name, "Oi" )
		)
		{
			return;
		}
		typename T::ValueType defaultValue; defaultValue.resize( expectedSize, typename T::ValueType::value_type( 0 ) );
		SxSetPredefinedParameter( parameterList, name, &(defaultValue[0]) );
	}
}
		
template<class T>
DataPtr SXExecutor::getVariable( SxParameterList parameterList, const char *name, size_t numPoints )
{
	typename T::Ptr result = new T;
	result->writable().resize( numPoints );
	
	void *d;
	SxGetWritableParameterInfo( m_shader, name, &d );
	memcpy( &(result->writable()[0]), d, numPoints * sizeof( typename T::ValueType::value_type ) );
	
	return result;
}
