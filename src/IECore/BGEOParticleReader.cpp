//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/BGEOParticleReader.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"
#include "IECore/Timer.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TestTypedData.h"

#include "OpenEXR/ImathRandom.h"

#include <algorithm>

#include <ostream>
#include <fstream>
#include <cassert>
#include <string>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( BGEOParticleReader );

const Reader::ReaderDescription<BGEOParticleReader> BGEOParticleReader::m_readerDescription( "bgeo" );

BGEOParticleReader::BGEOParticleReader( )
	:	ParticleReader( "BGEOParticleReader", "Reads Houdini .bgeo format particle caches" ), m_iStream( 0 )
{
}

BGEOParticleReader::BGEOParticleReader( const std::string &fileName )
	:	ParticleReader( "BGEOParticleReader", "Reads Houdini .bgeo  format particle caches" ), m_iStream( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
}

BGEOParticleReader::~BGEOParticleReader()
{
	delete m_iStream;
}

bool BGEOParticleReader::canRead( const std::string &fileName )
{
	ifstream i( fileName.c_str() );
	if( !i.is_open() )
	{
		return false;
	}
	char id[5];
	i.read( id, 5 );
	if( strncmp( "BgeoV ", id, 5 ) )
	{
		return false;
	}
	return true;
}

bool BGEOParticleReader::open()
{
	if( !m_iStream || m_streamFileName!=fileName() )
	{
		delete m_iStream;
		m_iStream = new ifstream( fileName().c_str() );
		if( !m_iStream->is_open() || !m_iStream->good() )
		{
			return false;
		}
		
		m_header.attributes.clear();
		
		char bgeo[5];
		m_iStream->read( bgeo, 5 );
		if( strncmp( "BgeoV", bgeo, 5 ) )
		{
			m_header.valid = false;
			return false;
		}

		m_iStream->read( (char *)&m_header.version, sizeof( m_header.version ) );
		m_header.version = asBigEndian( m_header.version );
		
		if( m_header.version > 5 )
		{
			msg( Msg::Warning, "BGEOParticleReader::open()", format( "File \"%s\" has unknown version %d." ) % fileName() % m_header.version );
		}

		m_iStream->read( (char *)&m_header.numPoints, sizeof( m_header.numPoints ) );
		m_header.numPoints = asBigEndian( m_header.numPoints );
		
		m_iStream->read( (char *)&m_header.numPrims, sizeof( m_header.numPrims ) );
		m_header.numPrims = asBigEndian( m_header.numPrims );
		
		m_iStream->read( (char *)&m_header.numPointGroups, sizeof( m_header.numPointGroups ) );
		m_header.numPointGroups = asBigEndian( m_header.numPointGroups );
		
		m_iStream->read( (char *)&m_header.numPrimGroups, sizeof( m_header.numPrimGroups ) );
		m_header.numPrimGroups = asBigEndian( m_header.numPrimGroups );
		
		m_iStream->read( (char *)&m_header.numPointAttribs, sizeof( m_header.numPointAttribs ) );
		m_header.numPointAttribs = asBigEndian( m_header.numPointAttribs );
		
		m_iStream->read( (char *)&m_header.numVertexAttribs, sizeof( m_header.numVertexAttribs ) );
		m_header.numVertexAttribs = asBigEndian( m_header.numVertexAttribs );
		
		m_iStream->read( (char *)&m_header.numPrimAttribs, sizeof( m_header.numPrimAttribs ) );
		m_header.numPrimAttribs = asBigEndian( m_header.numPrimAttribs );
		
		m_iStream->read( (char *)&m_header.numDetailAttribs, sizeof( m_header.numDetailAttribs ) );
		m_header.numDetailAttribs = asBigEndian( m_header.numDetailAttribs );
		
		// add P since it is always present but not listed in PointAttribs
		Record r;
		r.name = "P";
		r.size = 3;
		r.type = Vector;
		m_header.attributes.push_back( r );
		
		for( int i=0; i<m_header.numPointAttribs; i++ )
		{
			short nameLength;
			m_iStream->read( (char *)&nameLength, sizeof( nameLength ) );
			nameLength = asBigEndian( nameLength );
			
			Record r;
			char c;
			for( int j=0; j < nameLength; j++ )
			{
				m_iStream->read( &c, 1 );
				r.name += c;
			}
			
			m_iStream->read( (char *)&r.size, sizeof( r.size ) );
			r.size = asBigEndian( r.size );
			
			int type;
			m_iStream->read( (char *)&type, sizeof( type ) );
			type = asBigEndian( type );
			r.type = (AttributeType)type;
			
			switch( r.type )
			{
				case Float :
					m_iStream->seekg( sizeof( float ) * r.size, ios_base::cur );
					break;
				case Integer :
					m_iStream->seekg( sizeof( int ) * r.size, ios_base::cur );
					break;
				case Index :
					int size;
					m_iStream->read( (char *)&size, sizeof( size ) );
					size = asBigEndian( size );
					for ( int j=0; j < size; j++ )
					{
						m_iStream->read( (char *)&nameLength, sizeof( nameLength ) );
						nameLength = asBigEndian( nameLength );
						string value;
						for( int k=0; k < nameLength; k++ )
						{
							m_iStream->read( &c, 1 );
							value += c;
						}
						r.indexableValues.push_back( value );
					}
					break;
				case Vector :
					m_iStream->seekg( sizeof( float ) * r.size, ios_base::cur );
					break;
				default :
					assert( r.type < 6 ); // unknown type
			}
			
			m_header.attributes.push_back( r );
		}
		
		m_header.firstPointPosition = m_iStream->tellg();
		m_header.valid = m_iStream->good();
		m_streamFileName = fileName();
	}
	return m_iStream->good() && m_header.valid;
}

unsigned long BGEOParticleReader::numParticles()
{
	if( open() )
	{
		return m_header.numPoints;
	}
	return 0;
}

void BGEOParticleReader::attributeNames( std::vector<std::string> &names )
{
	if( open() )
	{
		vector<Record>::const_iterator it;
		for( it=m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
		{
			names.push_back( it->name );
		}
	}
}

ObjectPtr BGEOParticleReader::doOperation( ConstCompoundObjectPtr operands )
{
	size_t nParticles = numParticles();
	PointsPrimitivePtr result = new PointsPrimitive( nParticles );

	CompoundDataPtr attributeData = readAttributes();
	if ( !result )
	{
		throw Exception( ( format( "Failed to load \"%s\"." ) % fileName() ).str() );

	}

	bool haveNumPoints = false;
	for( vector<Record>::const_iterator it = m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
	{
		CompoundDataMap::const_iterator itData = attributeData->readable().find( it->name );
		if ( itData == attributeData->readable().end() )
		{
			msg( Msg::Warning, "ParticleReader::doOperation", format( "Attribute %s expected but not found." ) % it->name );
			continue;
		}
		
		DataPtr d = itData->second;
		
		if ( testTypedData<TypeTraits::IsVectorTypedData>( d ) )
		{
			size_t s = despatchTypedData< TypedDataSize, TypeTraits::IsVectorTypedData >( d );
			if( !haveNumPoints )
			{
				result->setNumPoints( s );
				haveNumPoints = true;
			}
			if( s==result->getNumPoints() )
			{
				result->variables.insert( PrimitiveVariableMap::value_type( it->name, PrimitiveVariable( PrimitiveVariable::Vertex, d ) ) );
			}
			else
			{
				msg( Msg::Warning, "ParticleReader::doOperation", format( "Ignoring attribute \"%s\" due to insufficient elements (expected %d but found %d)." ) % it->name % result->getNumPoints() % s );
			}
		}
		else if ( testTypedData<TypeTraits::IsSimpleTypedData>( d ) )
		{
			result->variables.insert( PrimitiveVariableMap::value_type( it->name, PrimitiveVariable( PrimitiveVariable::Constant, d ) ) );
		}
	}

	return result;
}

template<typename T, typename F>
boost::intrusive_ptr<T> BGEOParticleReader::filterAttr( boost::intrusive_ptr<F> attr, float percentage )
{
	if( percentage < 100.0f )
	{
		// percentage filtering (and type conversion if necessary)
		boost::intrusive_ptr<T> result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		int seed = particlePercentageSeed();
		float fraction = percentage / 100.0f;
		Rand48 r;
		r.init( seed );
		
		for( typename F::ValueType::size_type i=0; i<in.size(); i++ )
		{
			if( r.nextf() <= fraction )
			{
				out.push_back( in[i] );
			}
		}
		
		return result;
	}

	if( T::staticTypeId()!=F::staticTypeId() )
	{
		// type conversion only
		boost::intrusive_ptr<T> result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		out.resize( in.size() );
		copy( in.begin(), in.end(), out.begin() );
		return result;
	}

	// no filtering of any sort needed
	return boost::intrusive_ptr<T>( (T *)attr.get() );
}

template<typename T>
void BGEOParticleReader::readAttributeData( T *buffer, unsigned long n ) const
{
	m_iStream->read( (char *)buffer, n * sizeof( T ) );
	for( unsigned long i=0; i<n; i++ )
	{
		*buffer = asBigEndian( *buffer );
		buffer++;
	}
	assert( m_iStream->good() );
}

DataPtr BGEOParticleReader::readAttribute( const std::string &name )
{
	CompoundDataPtr result = readAttributes();
	if (!result)
	{
		return 0;
	}
	CompoundDataMap::const_iterator it = result->readable().find( name );
	if ( it == result->readable().end() )
	{
		return 0;
	}
	return it->second;
}

CompoundDataPtr BGEOParticleReader::readAttributes()
{
	if( !open() )
	{
		return 0;
	}
	
	CompoundDataPtr result = new CompoundData();
	
	std::vector< struct AttrInfo > attrInfo;
	
	int intAttribBuffer[ 3 ];
	int *intAttributePtr = &intAttribBuffer[0];
	float floatAttribBuffer[ 3 ];
	float *floatAttributePtr = &floatAttribBuffer[0];
	
	vector<Record>::const_iterator it;
	for( it=m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
	{
		V3fVectorDataPtr v3fVector = 0;
		V2fVectorDataPtr v2fVector = 0;
		FloatVectorDataPtr floatVector = 0;
		IntVectorDataPtr intVector = 0;
		StringVectorDataPtr stringVector = 0;
		DataPtr dataVector = 0;

		switch( it->size )
		{
		case 1:
			if ( it->type == Float )
			{
				floatVector = new FloatVectorData();
				floatVector->writable().resize( numParticles() );
				dataVector = floatVector;
				break;
			}
			else if ( it->type == Integer )
			{
				intVector = new IntVectorData();
				intVector->writable().resize( numParticles() );
				dataVector = intVector;
				break;
			}
			else if ( it->type == Index )
			{
				stringVector = new StringVectorData();
				stringVector->writable().resize( numParticles() );
				dataVector = stringVector;
				break;
			}
		case 2:
			v2fVector = new V2fVectorData();
			v2fVector->writable().resize( numParticles() );
			dataVector = v2fVector;
			break;
		case 3:
			v3fVector = new V3fVectorData();
			v3fVector->writable().resize( numParticles() );
			dataVector = v3fVector;
			break;
		default:
			msg( Msg::Error, "BGEOParticleReader::readAttributes()", format( "Internal error. Unrecognized type '%d' of size '%d' while loading attribute %s." ) % it->type % it->size % it->name );
			return 0;
		}

		AttrInfo info = {
			*it,
			dataVector,
		};

		attrInfo.push_back( info );
	}
		
	m_iStream->seekg( ios_base::beg + m_header.firstPointPosition );
	
	for ( int i = 0; i < m_header.numPoints; i++)
	{
		std::vector< struct AttrInfo >::iterator it;
		for (it = attrInfo.begin(); it != attrInfo.end(); it++)
		{
			// P contains an additional byte in the BGEO
			if ( it->info.name == "P" )
			{
				readAttributeData( floatAttributePtr, it->info.size+1 );
			}
			else if ( it->info.type == Integer || it->info.type == Index )
			{
				readAttributeData( intAttributePtr, it->info.size );
			}
			else
			{
				readAttributeData( floatAttributePtr, it->info.size );
			}
			
			switch (it->targetData->typeId())
			{
			case V3fVectorDataTypeId:
				{
					V3f &p = boost::static_pointer_cast<V3fVectorData>(it->targetData)->writable()[ i ];
					p[0] = floatAttributePtr[0];
					p[1] = floatAttributePtr[1];
					p[2] = floatAttributePtr[2];
					break;
				}
			case V2fVectorDataTypeId:
				{
					V2f &p = boost::static_pointer_cast<V2fVectorData>(it->targetData)->writable()[ i ];
					p[0] = floatAttributePtr[0];
					p[1] = floatAttributePtr[1];
					break;
				}
			case FloatVectorDataTypeId:
				boost::static_pointer_cast<FloatVectorData>(it->targetData)->writable()[ i ] = floatAttributePtr[0];
				break;
			case IntVectorDataTypeId:
				boost::static_pointer_cast<IntVectorData>(it->targetData)->writable()[ i ] = intAttributePtr[0];
				break;
			case StringVectorDataTypeId:
				{
					std::string value = it->info.indexableValues.at( intAttributePtr[0] );
					boost::static_pointer_cast<StringVectorData>(it->targetData)->writable()[ i ] = value;
					break;
				}
			default:
				msg( Msg::Error, "BGEOParticleReader::readAttributes()", format( "Internal error. Unrecognized typeId '%d'." ) % it->targetData->typeId() );
				return 0;
			}
		}
	}
	DataPtr filteredData = 0;
	// filter and convert each attribute individually.
	std::vector< struct AttrInfo >::const_iterator attrIt;
	for( attrIt=attrInfo.begin(); attrIt!=attrInfo.end(); attrIt++ )
	{
		switch( attrIt->info.size )
		{
		case 1:
			if ( attrIt->info.type == Float )
			{
				switch( realType() )
				{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<FloatVectorData, FloatVectorData>( boost::static_pointer_cast<FloatVectorData>(attrIt->targetData), particlePercentage() );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<DoubleVectorData, FloatVectorData>( boost::static_pointer_cast<FloatVectorData>(attrIt->targetData), particlePercentage() );
					break;
				}
				break;
			}
			else if ( attrIt->info.type == Integer )
			{
				filteredData = filterAttr<IntVectorData, IntVectorData>( boost::static_pointer_cast<IntVectorData>(attrIt->targetData), particlePercentage() );
				break;
			}
			else if ( attrIt->info.type == Index )
			{
				filteredData = filterAttr<StringVectorData, StringVectorData>( boost::static_pointer_cast<StringVectorData>(attrIt->targetData), particlePercentage() );
				break;
			}
		case 2:
			if ( attrIt->info.type == Float )
			{
				switch( realType() )
				{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<V2fVectorData, V2fVectorData>( boost::static_pointer_cast<V2fVectorData>(attrIt->targetData), particlePercentage() );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<V2dVectorData, V2fVectorData>( boost::static_pointer_cast<V2fVectorData>(attrIt->targetData), particlePercentage() );
					break;
				}
				break;
			}
		case 3:
			if ( ( attrIt->info.type == Float ) || ( attrIt->info.type == Vector ) )
			{
				switch( realType() )
				{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<V3fVectorData, V3fVectorData>( boost::static_pointer_cast<V3fVectorData>(attrIt->targetData), particlePercentage() );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<V3dVectorData, V3fVectorData>( boost::static_pointer_cast<V3fVectorData>(attrIt->targetData), particlePercentage() );
					break;
				}
				break;
			}
		default:
			msg( Msg::Error, "BGEOParticleReader::readAttributes()", format( "Internal error. Unrecognized type '%d' of size '%d' while converting attribute %s." ) % attrIt->info.type % attrIt->info.size % attrIt->info.name );
			return 0;
		}
		
		result->writable()[attrIt->info.name] = filteredData;
	}
	
	return result;
}

