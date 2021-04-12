//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/PDCParticleReader.h"

#include "IECoreScene/ParticleReader.inl"

#include "IECore/ByteOrder.h"
#include "IECore/FileNameParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Timer.h"
#include "IECore/VectorTypedData.h"

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace IECore;
using namespace IECoreScene;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PDCParticleReader );

const Reader::ReaderDescription<PDCParticleReader> PDCParticleReader::m_readerDescription( "pdc" );

PDCParticleReader::PDCParticleReader( )
	:	ParticleReader( "Reads Maya .pdc format particle caches" ), m_iStream( nullptr ), m_idAttribute( nullptr )
{
}

PDCParticleReader::PDCParticleReader( const std::string &fileName )
	:	ParticleReader( "Reads Maya .pdc format particle caches" ), m_iStream( nullptr ), m_idAttribute( nullptr )
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
		m_iStream = new ifstream( fileName().c_str(), std::ios_base::binary | std::ios_base::in );
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
		m_idAttribute = nullptr;
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

DataPtr PDCParticleReader::readAttribute( const std::string &name )
{
	if( !open() )
	{
		return nullptr;
	}

	map<string, Record>::const_iterator it = m_header.attributes.find( name );
	if( it==m_header.attributes.end() )
	{
		return nullptr;
	}

	const Data *idAttr = idAttribute();
	if ( !idAttr && particlePercentage() < 100.0f )
	{
		msg( Msg::Warning, "PDCParticleReader::filterAttr", format( "Percentage filtering requested but file \"%s\" contains no particle Id attribute." ) % fileName() );
	}

	DataPtr result = nullptr;
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
				result = filterAttr<IntVectorData, IntVectorData>( d.get(), particlePercentage(), idAttr );
			}
			break;
		case Double :
			{
				DoubleDataPtr d( new DoubleData );
				readElements( &d->writable(), it->second.position, 1 );
				switch( realType() )
				{
					case PDCParticleReader::RealType::Native :
					case PDCParticleReader::RealType::Double :
						result = d;
						break;
					case PDCParticleReader::RealType::Float :
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
					case PDCParticleReader::RealType::Native :
					case PDCParticleReader::RealType::Double :
						result = filterAttr<DoubleVectorData, DoubleVectorData>( d.get(), particlePercentage(), idAttr );
						break;
					case PDCParticleReader::RealType::Float :
						result = filterAttr<FloatVectorData, DoubleVectorData>( d.get(), particlePercentage(), idAttr );
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
					case PDCParticleReader::RealType::Native :
					case PDCParticleReader::RealType::Double :
						result = d;
						break;
					case PDCParticleReader::RealType::Float :
						result = new V3fData( d->readable() );
						break;
				}
			}
			break;
		case VectorArray :
			{
				V3dVectorDataPtr d( new V3dVectorData );
				/// \todo
				/// by all accounts the line below should be this :
				///
				/// d->writable().resize( numParticles() );
				///
				/// ie it shouldn't initialize the memory. but for some reason
				/// that runs far far slower for us (at least an order of magnitude
				/// slower) when we're in maya or python. we don't know why but it seems
				/// to be related to libstdc++ (maya has it's own). so we're opting for
				/// the initialized version - this seems slightly (~10%) slower when the planets
				/// are aligned correctly, but so much faster when the planets are aligned against
				/// us, as they seem to be whenever we're coding in maya. testing seems to show that
				/// this resize problem only occurs with V3d, and not with V3f, or double, or even
				/// a struct with 3 doubles in, or even a template struct with 3 doubles in.
				d->writable().resize( numParticles(), V3d( 0 ) );
				readElements( (double *)&d->writable()[0], it->second.position, numParticles() * 3 );
				switch( realType() )
				{
					case PDCParticleReader::RealType::Native :
					case PDCParticleReader::RealType::Double :
						result = filterAttr<V3dVectorData, V3dVectorData>( d.get(), particlePercentage(), idAttr );
						break;
					case PDCParticleReader::RealType::Float :
						result = filterAttr<V3fVectorData, V3dVectorData>( d.get(), particlePercentage(), idAttr );
						break;
				}
			}
			break;
		default :
			assert( it->second.type < 6 ); // unknown type

	}
	return result;
}

const Data * PDCParticleReader::idAttribute()
{
	if( !open() )
	{
		return nullptr;
	}
	if( !m_idAttribute )
	{
		map<string, Record>::const_iterator it = m_header.attributes.find( "particleId" );
		if( it == m_header.attributes.end() )
		{
			it = m_header.attributes.find( "id" );
		}

		if( it!=m_header.attributes.end() )
		{
			if( it->second.type==DoubleArray )
			{
				DoubleVectorDataPtr doubleVec = new DoubleVectorData;
				doubleVec->writable().resize( numParticles() );
				readElements( &doubleVec->writable()[0], it->second.position, numParticles() );
				m_idAttribute = doubleVec;
			}
			if( it->second.type==IntegerArray )
			{
				IntVectorDataPtr intVec = new IntVectorData;
				intVec->writable().resize( numParticles() );
				readElements( &intVec->writable()[0], it->second.position, numParticles() );
				m_idAttribute = intVec;
			}
		}
	}
	return m_idAttribute.get();
}

std::string PDCParticleReader::positionPrimVarName()
{
	return "position";
}

