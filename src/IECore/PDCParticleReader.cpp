//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PDCParticleReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"

#include "OpenEXR/ImathRandom.h"

#include <algorithm>


#include <fstream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<PDCParticleReader> PDCParticleReader::m_readerDescription( "pdc" );

PDCParticleReader::PDCParticleReader( )
	:	ParticleReader( "PDCParticleReader", "Reads Maya .pdc format particle caches" ), m_iStream( 0 ), m_idAttribute( 0 )
{
}

PDCParticleReader::PDCParticleReader( const std::string &fileName )
	:	ParticleReader( "PDCParticleReader", "Reads Maya .pdc format particle caches" ), m_iStream( 0 ), m_idAttribute( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
}

PDCParticleReader::~PDCParticleReader()
{
	delete m_iStream;
}

bool PDCParticleReader::canRead( const std::string &fileName )
{
	ifstream i( fileName.c_str() );
	if( !i.is_open() )
	{
		return false;
	}
	char id[4];
	i.read( id, 4 );
	if( strncmp( "PDC ", id, 4 ) )
	{
		return false;
	}
	return true;
}

bool PDCParticleReader::open()
{
	if( !m_iStream || m_streamFileName!=fileName() )
	{
		delete m_iStream;
		m_iStream = new ifstream( fileName().c_str() );
		if( !m_iStream->is_open() || !m_iStream->good() )
		{
			return false;
		}
		
		char pdc[4];
		m_iStream->read( pdc, 4 );
		if( strncmp( "PDC ", pdc, 4 ) )
		{
			m_header.valid = false;
			return false;
		}
		
		m_iStream->read( (char *)&m_header.version, sizeof( m_header.version ) );
		
		int endian = 0;
		m_iStream->read( (char *)&endian, sizeof( endian ) );
		if( endian!=1 )
		{
			m_header.reverseBytes = true;
			m_header.version = reverseBytes( m_header.version );
		}
		else
		{
			m_header.reverseBytes = false;
		}
		
		if( m_header.version > 1 )
		{
			msg( Msg::Warning, "PDCParticleReader::open()", format( "File \"%s\" has unknown version %d." ) % fileName() % m_header.version ); 
		}
		
		int unused = 0;
		m_iStream->read( (char *)&unused, sizeof( unused ) );
		m_iStream->read( (char *)&unused, sizeof( unused ) );
		
		m_iStream->read( (char *)&m_header.numParticles, sizeof( m_header.numParticles ) );
		if( m_header.reverseBytes )
		{
			m_header.numParticles = reverseBytes( m_header.numParticles );
		}
				
		int numAttributes;
		m_iStream->read( (char *)&numAttributes, sizeof( numAttributes ) );
		if( m_header.reverseBytes )
		{
			numAttributes = reverseBytes( numAttributes );
		}
		for( int i=0; i<numAttributes; i++ )
		{
			int nameLength;
			m_iStream->read( (char *)&nameLength, sizeof( nameLength ) );
			if( m_header.reverseBytes )
			{
				nameLength = reverseBytes( nameLength );
			}
			string attrName; char c;
			for( int j=0; j<nameLength; j++ )
			{
				m_iStream->read( &c, 1 );
				attrName += c;
			}
			if( attrName=="ghostFrames" )
			{
				// alias' own pdc files don't match their own spec.
				// they have a junk attributes on the end with no
				// type and no data. it's called ghostframes and
				// we need to skip it to prevent our stream from
				// going bad.
				assert( i==numAttributes-1 ); // we're assuming the bad attribute is always the last one
				continue;
			}
			Record r;
			m_iStream->read( (char *)&r.type, sizeof( r.type ) );
			if( m_header.reverseBytes )
			{
				r.type = reverseBytes( r.type );
			}
			r.position = m_iStream->tellg();
			m_header.attributes[attrName] = r;
			switch( r.type )
			{
				case Integer :
					m_iStream->seekg( sizeof( int ), ios_base::cur );
					break;
				case IntegerArray :
					m_iStream->seekg( sizeof( int ) * m_header.numParticles, ios_base::cur );
					break;
				case Double :
					m_iStream->seekg( sizeof( double ), ios_base::cur );
					break;
				case DoubleArray :
					m_iStream->seekg( sizeof( double ) * m_header.numParticles, ios_base::cur );
					break;
				case Vector :
					m_iStream->seekg( sizeof( double ) * 3, ios_base::cur );
					break;
				case VectorArray :
					m_iStream->seekg( sizeof( double ) * 3 * m_header.numParticles, ios_base::cur );
					break;					
				default :
					assert( r.type < 6 ); // unknown type
			}
			
		}
		
		m_header.valid = m_iStream->good();
		m_streamFileName = fileName();
		m_idAttribute = 0;
	}
	return m_iStream->good() && m_header.valid;
}

unsigned long PDCParticleReader::numParticles()
{
	if( open() )
	{
		return m_header.numParticles;
	}
	return 0;
}

void PDCParticleReader::attributeNames( std::vector<std::string> &names )
{
	names.clear();
	if( open() )
	{
		map<string, Record>::const_iterator it;
		for( it=m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
		{
			names.push_back( it->first );
		}
	}
}

template<typename T>
void PDCParticleReader::readElements( T *buffer, std::streampos pos, unsigned long n ) const
{
	m_iStream->seekg( pos );
	m_iStream->read( (char *)buffer, n * sizeof( T ) );
	if( m_header.reverseBytes )
	{
		for( unsigned long i=0; i<n; i++ )
		{
			*buffer = reverseBytes( *buffer );
			buffer++;
		}
	}
	assert( m_iStream->good() );
}

template<typename T, typename F>
boost::intrusive_ptr<T> PDCParticleReader::filterAttr( boost::intrusive_ptr<F> attr, float percentage )
{
	if( percentage < 100.0f )
	{
		ConstDoubleVectorDataPtr idAttr = idAttribute();
		if( idAttr )
		{	
			// percentage filtering (and type conversion if necessary)
			boost::intrusive_ptr<T> result( new T );
			const typename F::ValueType &in = attr->readable();
			typename T::ValueType &out = result->writable();
			const vector<double> &ids = idAttr->readable();
			int seed = particlePercentageSeed();
			float fraction = percentage / 100.0f;
			Rand48 r;
			for( typename F::ValueType::size_type i=0; i<in.size(); i++ )
			{
				r.init( seed + (int)ids[i] );
				if( r.nextf() <= fraction )
				{
					out.push_back( in[i] );
				}
			}
			return result;
		}
		else
		{
			msg( Msg::Warning, "PDCParticleReader::filterAttr", format( "Percentage filtering requested but file \"%s\" contains no particleId attribute." ) % fileName() );
			// fall through to allow basic loading to happen anyway.
		}
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

DataPtr PDCParticleReader::readAttribute( const std::string &name )
{
	if( !open() )
	{
		return 0;
	}

	map<string, Record>::const_iterator it = m_header.attributes.find( name );
	if( it==m_header.attributes.end() )
	{
		return 0;
	}
	
	DataPtr result = 0;
	switch( it->second.type )
	{
		case Integer :
			{
				IntDataPtr d( new IntData );
				readElements( &d->writable(), it->second.position, 1 );
				result = d;			
			}
			break;
		case IntegerArray :
			{
				IntVectorDataPtr d( new IntVectorData );
				d->writable().resize( numParticles() );
				readElements( &d->writable()[0], it->second.position, numParticles() );
				result = filterAttr<IntVectorData, IntVectorData>( d, particlePercentage() );
			}
			break;
		case Double :
			{
				DoubleDataPtr d( new DoubleData );
				readElements( &d->writable(), it->second.position, 1 );
				switch( realType() )
				{
					case Native :
					case Double :
						result = d;
						break;
					case Float :
						result = new FloatData( d->readable() );
						break;
				}
			}
			break;
		case DoubleArray :
			{
				DoubleVectorDataPtr d( new DoubleVectorData );
				d->writable().resize( numParticles() );
				readElements( &d->writable()[0], it->second.position, numParticles() );
				switch( realType() )
				{
					case Native :
					case Double :
						result = filterAttr<DoubleVectorData, DoubleVectorData>( d, particlePercentage() );
						break;
					case Float :
						result = filterAttr<FloatVectorData, DoubleVectorData>( d, particlePercentage() );
						break;
				}
			}
			break;
		case Vector :
			{
				V3dDataPtr d( new V3dData );
				readElements( (double *)&d->writable(), it->second.position, 3 );
				switch( realType() )
				{
					case Native :
					case Double :
						result = d;
						break;
					case Float :
						result = new V3fData( d->readable() );
						break;
				}
			}
			break;
		case VectorArray :
			{
				V3dVectorDataPtr d( new V3dVectorData );
				d->writable().resize( numParticles() );
				readElements( (double *)&d->writable()[0], it->second.position, numParticles() * 3 );
				switch( realType() )
				{
					case Native :
					case Double :
						result = filterAttr<V3dVectorData, V3dVectorData>( d, particlePercentage() );
						break;
					case Float :
						result = filterAttr<V3fVectorData, V3dVectorData>( d, particlePercentage() );
						break;
				}
			}
			break;	
		default :
			assert( it->second.type < 6 ); // unknown type
	
	}
	return result;
}
	
ConstDoubleVectorDataPtr PDCParticleReader::idAttribute()
{
	if( !open() )
	{
		return 0;
	}
	if( !m_idAttribute )
	{
		map<string, Record>::const_iterator it = m_header.attributes.find( "particleId" );
		if( it!=m_header.attributes.end() )
		{
			if( it->second.type==DoubleArray )
			{
				m_idAttribute = new DoubleVectorData;
				m_idAttribute->writable().resize( numParticles() );
				readElements( &m_idAttribute->writable()[0], it->second.position, numParticles() );
			}
		}
	}
	return m_idAttribute;
}
