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

#include "boost/shared_ptr.hpp"

#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

#include "IECoreRI/SXExecutor.h"
#include "IECoreRI/SXTypeTraits.h"

using namespace IECoreRI;
using namespace IECore;
using namespace Imath;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// SXExecutor::Implementation
//////////////////////////////////////////////////////////////////////////

class SXExecutor::Implementation : public IECore::RefCounted
{
	public :

		Implementation( const ShaderVector &shaders, SxContext context, const ShaderVector &coshaders, const ShaderVector &lights )
			:	m_context( context ), m_shaders( shaders )
		{
			// build the map from parameter names to the expected type for that parameter
			for( ShaderVector::const_iterator it=shaders.begin(); it!=shaders.end(); ++it )
			{
				storeParameterInfo( *it );
			}
			for( ShaderVector::const_iterator it=coshaders.begin(); it!=coshaders.end(); ++it )
			{
				storeParameterInfo( *it, false );
			}
			for( ShaderVector::const_iterator it=lights.begin(); it!=lights.end(); ++it )
			{
				storeParameterInfo( *it );
			}
		}

		IECore::CompoundDataPtr execute( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const
		{
			// decide how many points we're shading and validate the grid size

			const V3fVectorData *p = points->member<V3fVectorData>( "P", true /* throw */ );
			size_t numPoints = p->readable().size();
			if( !numPoints )
			{
				throw Exception( "\"P\" has zero length." );
			}

			bool haveGrid = gridSize.x > 0 && gridSize.y > 0;
			int numGrids = 0;
			if( haveGrid )
			{
				numGrids = numPoints / (size_t)(gridSize.x * gridSize.y);
				if( numPoints != (size_t)( numGrids * gridSize.x * gridSize.y) )
				{
					throw Exception( boost::str( boost::format( "Wrong number of points (%d) for grid (%dx%d)." ) % numPoints % gridSize.x % gridSize.y ) );
				}
			}

			// create parameter list for the grid and set topology if we can.

			boost::shared_ptr<void> vars( SxCreateParameterList( m_context, numPoints, "current" ), SxDestroyParameterList );

			std::vector<unsigned> nu;
			std::vector<unsigned> nv;
			if( haveGrid )
			{
				nu.resize( numGrids, gridSize.x );
				nv.resize( numGrids, gridSize.y );
				SxSetParameterListGridTopology( vars.get(), numGrids, &nu.front(), &nv.front() );
			}

			// fill the grid from our input data.

			setVariables( vars.get(), points, numPoints );

			// run all the shaders in sequence on the grid

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

	private :

		struct ParameterInfo
		{
			ParameterInfo()
				:	type( SxInvalid ), arraySize( 0 ), varying( true )
			{
			}

			ParameterInfo( SxType t, unsigned as, bool v )
				:	type( t ), arraySize( as ), varying( v )
			{
			}

			bool operator == ( const ParameterInfo &other ) const
			{
				return type == other.type && arraySize == other.arraySize;
			}

			bool operator != ( const ParameterInfo &other ) const
			{
				return ! ( *this == other );
			}

			SxType type;
			unsigned arraySize;
			bool varying;
		};

		void storeParameterInfo( SxShader shader, bool printWarnings = true )
		{
			unsigned numParameters = SxGetNumParameters( shader );
			for( unsigned i=0; i<numParameters; i++ )
			{
				ParameterInfo info;
				SxData defaultValue;
				const char *spaceName;
				bool output;
				const char *name = SxGetParameterInfo( shader, i, &info.type, &info.varying, &defaultValue, &info.arraySize, &spaceName, &output );
				TypeMap &typeMap = output ? m_outputParameterTypes : m_inputParameterTypes;

				TypeMap::const_iterator it = typeMap.find( name );
				if( it != typeMap.end() )
				{
					if( it->second != info && printWarnings )
					{
						msg( Msg::Warning, "SXExecutor::storeParameterTypes", boost::format( "Shaders request conflicting types for parameter \"%s\"" ) % name );
					}
					continue;
				}
				typeMap[name] = info;
			}
		}

		ParameterInfo predefinedParameterInfo( const char *name ) const
		{
			if(
				strcmp( name, "P" )==0
			)
			{
				return ParameterInfo( SxPoint, 0, true );
			}
			else if(
				strcmp( name, "N" )==0 ||
				strcmp( name, "Ng" )==0
			)
			{
				return ParameterInfo( SxNormal, 0, true );
			}
			else if(
				strcmp( name, "I" )==0 ||
				strcmp( name, "dPdu" )==0 ||
				strcmp( name, "dPdv" )==0
			)
			{
				return ParameterInfo( SxVector, 0, true );
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
				return ParameterInfo( SxFloat, 0, true );
			}
			else if(
				strcmp( name, "Cs" )==0 ||
				strcmp( name, "Os" )==0 ||
				strcmp( name, "Ci" )==0 ||
				strcmp( name, "Oi" )==0 ||
				strcmp( name, "Cl" )==0
			)
			{
				return ParameterInfo( SxColor, 0, true );
			}

			return ParameterInfo();
		}

		ParameterInfo assumedParameterInfo( IECore::TypeId type ) const
		{
			switch( type )
			{
				case FloatVectorDataTypeId :
					return ParameterInfo( SxFloat, 0, true );
				case V3fVectorDataTypeId :
					return ParameterInfo( SxVector, 0, true );
				case Color3fVectorDataTypeId :
					return ParameterInfo( SxColor, 0, true );

				case StringVectorDataTypeId :
				case StringDataTypeId :
					return ParameterInfo( SxString, 0, false );
				case FloatDataTypeId :
					return ParameterInfo( SxFloat, 0, false );
				case V3fDataTypeId :
					return ParameterInfo( SxVector, 0, false );
				case Color3fDataTypeId :
					return ParameterInfo( SxColor, 0, false );

				default :
					return ParameterInfo( SxInvalid, 0, true );
			}
		}

		bool parameterTypesConsistent( SxType type, IECore::TypeId typeId ) const
		{
			switch( typeId )
			{
				case FloatVectorDataTypeId :
					return type == SxFloat;
				case V3fVectorDataTypeId :
				case V3fDataTypeId :
					return type == SxVector || type == SxNormal || type == SxPoint;
				case Color3fVectorDataTypeId :
				case Color3fDataTypeId :
					return type == SxColor;
				case StringVectorDataTypeId :
				case StringDataTypeId :
					return type == SxString;
				case FloatDataTypeId :
					return type == SxFloat;
				default :
					return type == SxInvalid;
			}
		}

		void setVariables( SxParameterList parameterList, const IECore::CompoundData *points, size_t numPoints ) const
		{
			for( CompoundDataMap::const_iterator it=points->readable().begin(); it!=points->readable().end(); it++ )
			{
				ParameterInfo info = predefinedParameterInfo( it->first.value().c_str() );
				if( info.type != SxInvalid && parameterTypesConsistent( info.type, it->second->typeId() ) )
				{
					setVariable( parameterList, it->first.value().c_str(), info, true, it->second.get(), numPoints );
				}
				else
				{
					TypeMap::const_iterator tIt = m_inputParameterTypes.find( it->first );
					if( tIt != m_inputParameterTypes.end() )
					{
						setVariable( parameterList, it->first.value().c_str(), tIt->second, false, it->second.get(), numPoints );
					}
					else
					{
						// variable isn't a parameter to any of the shaders, but we'd like to pass it in anyway, so it can
						// be used with getvar(). guess a type and go for it.
						ParameterInfo info = assumedParameterInfo( it->second->typeId() );
						if( info.type != SxInvalid )
						{
							setVariable( parameterList, it->first.value().c_str(), info, false, it->second.get(), numPoints );
						}
						else
						{
							throw Exception( boost::str( boost::format( "Unsupported parameter type \"%s\"" ) % it->second->typeName() ) );
						}
					}
				}
			}
		}

		void setVariable( SxParameterList parameterList, const char *name, const ParameterInfo &info, bool predefined, const IECore::Data *data, size_t numPoints ) const
		{
			if( info.varying )
			{
				switch( info.type )
				{
					case SxFloat :
						if( info.arraySize==3 && data->isInstanceOf( IECore::V3fVectorData::staticTypeId() ) )
						{
							setVaryingVariable<SxFloat, V3fVectorData>( parameterList, name, predefined, data, numPoints, info.arraySize );
						}
						else
						{
							setVaryingVariable<SxFloat, SXTypeTraits<SxFloat>::VectorDataType>( parameterList, name, predefined, data, numPoints, info.arraySize );
						}
						break;
					case SxColor :
						setVaryingVariable<SxColor, SXTypeTraits<SxColor>::VectorDataType>( parameterList, name, predefined, data, numPoints, info.arraySize );
						break;
					case SxPoint :
						setVaryingVariable<SxPoint, SXTypeTraits<SxPoint>::VectorDataType>( parameterList, name, predefined, data, numPoints, info.arraySize );
						break;
					case SxVector :
						setVaryingVariable<SxVector, SXTypeTraits<SxVector>::VectorDataType>( parameterList, name, predefined, data, numPoints, info.arraySize );
						break;
					case SxNormal :
						setVaryingVariable<SxNormal, SXTypeTraits<SxNormal>::VectorDataType>( parameterList, name, predefined, data, numPoints, info.arraySize );
						break;
					default :
						throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % name ) );
				}
			}
			else
			{
				switch( info.type )
				{
					case SxFloat :
						if( info.arraySize==3 && data->isInstanceOf( IECore::V3fData::staticTypeId() ) )
						{
							setUniformVariable<SxFloat, V3fData>( parameterList, name, data, info.arraySize );
						}
						else
						{
							setUniformVariable<SxFloat, SXTypeTraits<SxFloat>::DataType>( parameterList, name, data, info.arraySize );
						}
						break;
					case SxColor :
						setUniformVariable<SxColor, SXTypeTraits<SxColor>::DataType>( parameterList, name, data, info.arraySize );
						break;
					case SxPoint :
						setUniformVariable<SxPoint, SXTypeTraits<SxPoint>::DataType>( parameterList, name, data, info.arraySize );
						break;
					case SxVector :
						setUniformVariable<SxVector, SXTypeTraits<SxVector>::DataType>( parameterList, name, data, info.arraySize );
						break;
					case SxNormal :
						setUniformVariable<SxNormal, SXTypeTraits<SxNormal>::DataType>( parameterList, name, data, info.arraySize );
						break;
					case SxString :
						setStringVariable( parameterList, name, data );
						break;
					default :
						throw Exception( boost::str( boost::format( "Input parameter \"%s\" has unsupported type." ) % name ) );
				}
			}
		}

		template<SxType sxType, typename DataType>
		void setVaryingVariable( SxParameterList parameterList, const char *name, bool predefined, const IECore::Data *data, size_t numPoints, unsigned arraySize ) const
		{

			const DataType *td = IECore::runTimeCast<const DataType>( data );
			if( !td )
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be %s)." ) % name % data->typeName() % DataType::staticTypeName() ) ) );
			}

			size_t size = td->readable().size();
			if( size != numPoints )
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong size (%d but should be %d)." ) % name % size % numPoints ) ) );
			}

			void *rawData = (void *)&(td->readable()[0]);
			if( predefined )
			{
				SxSetPredefinedParameter( parameterList, name, rawData );
			}
			else
			{
				SxSetParameter( parameterList, name, sxType, rawData, true, arraySize );
			}

		}

		template<SxType sxType, typename DataType>
		void setUniformVariable( SxParameterList parameterList, const char *name, const IECore::Data *data, unsigned arraySize ) const
		{
			const DataType *td = IECore::runTimeCast<const DataType>( data );
			if( !td )
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be %s)." ) % name % data->typeName() % DataType::staticTypeName() ) ) );
			}

			void *rawData = (void *)&(td->readable());
			SxSetParameter( parameterList, name, sxType, rawData, false, arraySize );

		}

		void setStringVariable( SxParameterList parameterList, const char *name, const IECore::Data *data ) const
		{
			if( const StringData *td = IECore::runTimeCast<const StringData>( data ) )
			{
				const char* str = td->readable().c_str();
				SxSetParameter( parameterList, name, SxString, (void *)&str, false, 0 );
			}
			else if( const StringVectorData *td = IECore::runTimeCast<const StringVectorData>( data ) )
			{
				std::vector<const char*> strings;
				for( size_t i=0; i < td->readable().size(); ++i )
				{
					strings.push_back( td->readable()[i].c_str() );
				}

				SxSetParameter( parameterList, name, SxString, &strings[0], false, strings.size() );
			}
			else
			{
				throw( Exception( boost::str( boost::format( "Input parameter \"%s\" has wrong type (%s but should be StringData or StringVectorData)." ) % name % data->typeName() ) ) );
			}
		}

		IECore::CompoundDataPtr getVariables( SxParameterList parameterList ) const
		{
			CompoundDataPtr result = new CompoundData;
			for( TypeMap::const_iterator it=m_outputParameterTypes.begin(); it!=m_outputParameterTypes.end(); ++it )
			{
				switch( it->second.type )
				{
					case SxFloat :
						getVariable<SxFloat>( parameterList, it->first.value().c_str(), result.get() );
						break;
					case SxColor :
						getVariable<SxColor>( parameterList, it->first.value().c_str(), result.get() );
						break;
					case SxPoint :
						getVariable<SxPoint>( parameterList, it->first.value().c_str(), result.get() );
						break;
					case SxVector :
						getVariable<SxVector>( parameterList, it->first.value().c_str(), result.get() );
						break;
					case SxNormal :
						getVariable<SxNormal>( parameterList, it->first.value().c_str(), result.get() );
						break;
					default :
						throw Exception( boost::str( boost::format( "Output parameter \"%s\" has unsupported type." ) % it->first.value() ) );
				}
			}

			getVariable<SxColor>( parameterList, "Ci", result.get() );
			getVariable<SxColor>( parameterList, "Oi", result.get() );
			getVariable<SxPoint>( parameterList, "P", result.get() );
			getVariable<SxNormal>( parameterList, "N", result.get() );

			return result;
		}

		template<SxType sxType>
		void getVariable( SxParameterList parameterList, const char *name, IECore::CompoundData *result ) const
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

		SxContext m_context;
		const ShaderVector &m_shaders;

		typedef std::map<IECore::InternedString, ParameterInfo> TypeMap;
		TypeMap m_inputParameterTypes;
		TypeMap m_outputParameterTypes;

}; // class SXExecutor::Implementation

//////////////////////////////////////////////////////////////////////////
// SXExecutor
//////////////////////////////////////////////////////////////////////////

SXExecutor::SXExecutor( const ShaderVector &shaders, SxContext context, const ShaderVector &coshaders, const ShaderVector &lights )
	:	m_implementation( new Implementation( shaders, context, coshaders, lights ) )
{
}

SXExecutor::~SXExecutor()
{
}

IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points ) const
{
	V2i gridSize( 0 );
	return m_implementation->execute( points, gridSize );
}

IECore::CompoundDataPtr SXExecutor::execute( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const
{
	return m_implementation->execute( points, gridSize );
}

