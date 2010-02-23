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

//////////////////////////////////////////////////////////////////////////
// InputVariables implementation
//////////////////////////////////////////////////////////////////////////

struct SXExecutor::InputVariables
{
	public :
	
		InputVariables( SxShader shader, IECore::ConstCompoundDataPtr points )
			:	m_sizeOfPoint( 0 )
		{
			const V3fVectorData *p = points->member<V3fVectorData>( "P", true /* throw */ );
			m_numPoints = p->readable().size();
		
			unsigned numPredefinedParameters = SxGetPredefinedParameters( shader, 0, 0 );
			std::vector<const char *> predefinedParameters( numPredefinedParameters );
			SxGetPredefinedParameters( shader, &(predefinedParameters[0]), numPredefinedParameters );
		
			for( unsigned i=0; i<numPredefinedParameters; i++ )
			{
				const Data *d = points->member<Data>( predefinedParameters[i] );
				if( !d )
				{
					continue;
				}

				IECore::TypeId expectedType = predefinedParameterType( predefinedParameters[i] );
				if( !d->isInstanceOf( expectedType ) )
				{
					throw Exception( boost::str( boost::format( "Variable \"%s\" has wrong type (\"%s\" but should be \"%s\")." ) % predefinedParameters[i] % d->typeName() % RunTimeTyped::typeNameFromTypeId( expectedType ) ) );
				}

				switch( expectedType )
				{
					case FloatVectorDataTypeId :
					{
						addVariable<float>( d, predefinedParameters[i] );
						break;
					}
					case V3fVectorDataTypeId :
					{
						addVariable<V3f>( d, predefinedParameters[i] );
						break;
					}
					case Color3fVectorDataTypeId :
					{
						addVariable<Color3f>( d, predefinedParameters[i] );
						break;
					}
					default :
						throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % predefinedParameters[i] ) );
				}

			}
		
		}

		size_t numPoints() const { return m_numPoints; }

		void fill( SxParameterList parameters, std::vector<char> &storage, size_t pointIndex ) const
		{
			storage.resize( m_sizeOfPoint );
			char *shadingPointData = &(storage[0]);
			for( unsigned j=0; j<m_predefinedVarNames.size(); j++ )
			{
				memcpy( shadingPointData, m_varData[j] + m_varSizes[j] * pointIndex, m_varSizes[j] );
				SxSetPredefinedParameter( parameters, m_predefinedVarNames[j].c_str(), shadingPointData );
				shadingPointData += m_varSizes[j];
			}
		}
		
	private :
	
		template<class T>
		void addVariable( const Data *d, const char *name )
		{
			typedef IECore::TypedData<std::vector<T> > Data;
			const Data *td = static_cast<const Data *>( d );
			m_predefinedVarNames.push_back( name );
			m_varData.push_back( reinterpret_cast<const char *>( &(td->readable()[0]) ) );
			m_varSizes.push_back( sizeof( T ) );
			m_sizeOfPoint += sizeof( T);
		}
	
		static IECore::TypeId predefinedParameterType( const char *name )
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

			throw Exception( boost::str( boost::format( "Unknown predefined parameter \"%s\"" ) % name ) ); 
		}

		size_t m_numPoints;		

		std::vector<std::string> m_predefinedVarNames;
		std::vector<const char *> m_varData;
		std::vector<size_t> m_varSizes;
		size_t m_sizeOfPoint; // size of data requirements for all variables for a single point

};

//////////////////////////////////////////////////////////////////////////
// OutputVariables implementation
//////////////////////////////////////////////////////////////////////////

struct SXExecutor::OutputVariables
{

	public :
	
		OutputVariables( SxShader shaderInfo, size_t numPoints )
			:	m_numPoints( numPoints ), m_result( new CompoundData )
		{
			unsigned numParameters = SxGetNumParameters( shaderInfo );
			for( unsigned i=0; i<numParameters; i++ )
			{
				SxType type;
				bool varying;
				SxData defaultValue;
				unsigned arraySize;
				const char *spaceName;
				bool output;
				const char *name = SxGetParameterInfo( shaderInfo, i, &type, &varying, &defaultValue, &arraySize, &spaceName, &output );
				if( output )
				{
					switch( type )
					{
						case SxFloat :
						{
							addVariable<float>( name );
							break;	
						}
						case SxColor :
						{
							addVariable<Color3f>( name );
							break;	
						}
						case SxVector :
						{
							addVariable<V3f>( name );
							break;	
						}
						case SxPoint :
						{
							addVariable<V3f>( name );
							break;	
						}
						case SxNormal :
						{
							addVariable<V3f>( name );
							break;	
						}
						default :
							throw Exception( boost::str( boost::format( "Output parameter \"%s\" has unsupported type." ) % name ) );

					}
				}
			}
		
			addVariable<Color3f>( "Ci" );
			addVariable<Color3f>( "Oi" );
		}

		void fill( SxShader shader, size_t pointIndex )
		{
			for( unsigned j=0; j<m_names.size(); j++ )
			{
				void *d;
				SxGetWritableParameterInfo( shader, m_names[j].c_str(), &d );
				memcpy( m_data[j] + m_sizes[j] * pointIndex, d, m_sizes[j] );
			}
		}
		
		IECore::CompoundDataPtr result() { return m_result; };

	private :
	
		template<class T>
		void addVariable( const char *name )
		{
			typedef IECore::TypedData<std::vector<T> > Data;
			typename Data::Ptr data = new Data;
			data->writable().resize( m_numPoints );
			m_result->writable()[name] = data;
			m_names.push_back( name );
			m_data.push_back( reinterpret_cast<char *>( &(data->writable()[0]) ) );
			m_sizes.push_back( sizeof( T ) );
		}
	
		SxShader m_shader;
		
		size_t m_numPoints;
		
		std::vector<std::string> m_names;
		std::vector<char *> m_data;
		std::vector<size_t> m_sizes;
	
		IECore::CompoundDataPtr m_result;
		
};

//////////////////////////////////////////////////////////////////////////
// Main implementation
//////////////////////////////////////////////////////////////////////////

SXExecutor::SXExecutor( SxShader shader, SxShader shaderInfo )
	:	m_shader( shader ), m_shaderInfo( shaderInfo )
{
}

IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points )
{
	InputVariables inputVariables( m_shader, points );

	OutputVariables outputVariables( m_shaderInfo, inputVariables.numPoints() );
	
	SxParameterList vars = SxCreateParameterList( 0, 1, 1, "current" );
	
	vector<char> storage;
	for( unsigned i=0; i<inputVariables.numPoints(); i++ )
	{
		inputVariables.fill( vars, storage, i );
		SxCallShader( m_shader, vars, 0, 0 );
		outputVariables.fill( m_shader, i );
	}	

	SxDestroyParameterList( vars );
	
	return outputVariables.result();
}
