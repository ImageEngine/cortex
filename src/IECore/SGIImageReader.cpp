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

#include "IECore/SGIImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/ScaledDataConversion.h"

#include "boost/format.hpp"
#include "boost/static_assert.hpp"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( SGIImageReader );

struct SGIImageReader::Header
{
	struct FileHeader 
	{
			uint16_t  m_magic;
			uint8_t   m_storageFormat;
			uint8_t   m_bytesPerChannel;
			uint16_t  m_dimension;
			uint16_t  m_xSize;
			uint16_t  m_ySize;
			uint16_t  m_zSize;		
			uint32_t  m_pixMin;
			uint32_t  m_pixMax;
		private:	
			uint32_t  m_dummy1;
		public:
			char      m_imageName[80];
			uint32_t  m_colorMap;
		private:
			char      m_dummy2[404];
	};
	
	BOOST_STATIC_ASSERT( (sizeof( FileHeader ) == 512 ) );
	
	FileHeader m_fileHeader;
	
	/// Buffer-relative indices and lengths for the RLE scanlines. Note that the file stores them 
	/// as file-relative offsets, and we transform them on load.
	std::vector< uint32_t > m_offsetTable;
	std::vector< uint32_t > m_lengthTable;
	
	typedef map< string, int > ChannelOffsetMap;
	map< string, int > m_channelOffsets;	
};

const Reader::ReaderDescription<SGIImageReader> SGIImageReader::m_readerDescription( "sgi rgb rgba bw" );

SGIImageReader::SGIImageReader() :
		ImageReader( "SGIImageReader", "Reads SGI RGB files." )
{
}

SGIImageReader::SGIImageReader( const string &fileName ) :
		ImageReader( "SGIImageReader", "Reads SGI RGB  files." )
{
	m_fileNameParameter->setTypedValue(fileName);
}

SGIImageReader::~SGIImageReader()
{
}

bool SGIImageReader::canRead( const string &fileName )
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if ( !in.is_open() || in.fail() )
	{
		return false;
	}

	// seek to start
	in.seekg(0, ios_base::beg);
	if ( in.fail() )
	{
		return false;
	}

	// check magic number
	uint16_t magic;
	in.read((char *) &magic, sizeof(uint16_t));
	if ( in.fail() )
	{
		return false;
	}
	
	/// SGI files are written big-endian. Nuke seems to support little-endian files, but this isn't in the spec.
	if ( littleEndian() )
	{
		if ( reverseBytes( magic ) == 474 )
		{
			return true;
		} 
	}
	else
	{
		assert( bigEndian() );
		
		if ( magic == 474 )
		{
			return true;
		}
	}
	
	return false;
}

void SGIImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );
	
	assert( m_header );

	for ( Header::ChannelOffsetMap::const_iterator it = m_header->m_channelOffsets.begin(); it != m_header->m_channelOffsets.end(); ++it )
	{
		names.push_back( it->first );
	}
}

bool SGIImageReader::isComplete()
{
	return open( false );
}

Box2i SGIImageReader::dataWindow()
{
	open( true );

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Box2i SGIImageReader::displayWindow()
{
	open( true );

	return dataWindow();
}

DataPtr SGIImageReader::readChannel( const string &name, const Imath::Box2i &dataWindow )
{
	if ( !open() )
	{
		return 0;
	}
	
	assert( m_header );
	
	if ( m_header->m_fileHeader.m_bytesPerChannel == 1 )
	{
		return readTypedChannel< unsigned char >( name, dataWindow );
	}	
	else
	{
		assert ( m_header->m_fileHeader.m_bytesPerChannel == 2 );
		return readTypedChannel< unsigned short >( name, dataWindow );
	}	
}

template<typename T>
DataPtr SGIImageReader::readTypedChannel( const std::string &name, const Box2i &dataWindow )
{
	assert( open() );
	
	assert( m_header );
	
	typename TypedData< std::vector<T > >::ConstPtr dataBuffer = assertedStaticCast< TypedData< std::vector<T > > >( m_buffer );
	assert( dataBuffer );
	
	const typename TypedData< std::vector<T > >::ValueType &buffer = dataBuffer->readable();
		
	assert( m_header->m_channelOffsets.find( name ) != m_header->m_channelOffsets.end() );
	int channelOffset = m_header->m_channelOffsets[name];
	
	FloatVectorDataPtr dataContainer = new FloatVectorData();
	
	FloatVectorData::ValueType &data = dataContainer->writable();
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;	

	Box2i wholeDataWindow = this->dataWindow();
	
	int wholeDataHeight = 1 + wholeDataWindow.size().y;	
	int wholeDataWidth = 1 + wholeDataWindow.size().x;		
	
	const int yMin = dataWindow.min.y - wholeDataWindow.min.y;	
	const int yMax = dataWindow.max.y - wholeDataWindow.min.y;	
	
	const int xMin = dataWindow.min.x - wholeDataWindow.min.x;
	const int xMax = dataWindow.max.x - wholeDataWindow.min.x;
	
	ScaledDataConversion<T, float> converter;
		
	if ( m_header->m_fileHeader.m_storageFormat >= 1 ) /// RLE
	{				
		int dataY = 0;
		for ( int y = yMin ; y <= yMax ; ++y, ++dataY )
		{
			assert( yMin >= 0 );
			assert( yMin < wholeDataHeight );			
			
			/// Images are unencoded "upside down" (flipped in Y), for our purposes
			uint32_t rowOffset = m_header->m_offsetTable[ channelOffset * wholeDataHeight + ( wholeDataHeight - 1 - y ) ];
			
			if ( rowOffset >= buffer.size() )
			{
				throw IOException( "SGIImageReader: Invalid RLE row offset found while reading " + fileName() );
			}
			
			std::vector<T> scanline( wholeDataWidth );
			
			uint32_t scanlineOffset = 0;
			bool done = false;
			
			while ( !done )
			{
				T pixel = buffer[ rowOffset ++ ];

				T count = pixel & 0x7f;

				if ( count == 0 )
				{
					done = true;
				}
				else
				{
					if ( pixel & 0x80 )
					{					
						if ( scanlineOffset + count - 1 >= scanline.size() || rowOffset + count - 1 >= buffer.size() )
						{
							throw IOException( "SGIImageReader: Invalid RLE data found while reading " + fileName() );
						}
							
						while ( count -- )
						{
							assert( scanlineOffset < scanline.size() );
							assert( rowOffset < buffer.size() );							
							scanline[ scanlineOffset++ ] = buffer[ rowOffset ++ ];
						}
					}
					else
					{
						if ( scanlineOffset + count - 1 >= scanline.size() || rowOffset - 1 >= buffer.size() )
						{
							throw IOException( "SGIImageReader: Invalid RLE data found while reading " + fileName() );						
						}
						
						assert( rowOffset < buffer.size() );													
						pixel = buffer[ rowOffset ++ ];
																		
						while ( count -- )
						{			
							assert( scanlineOffset < scanline.size() );										
							scanline[ scanlineOffset++ ] = pixel;
						}
					}	
				}			
			}
			
			if ( scanlineOffset != scanline.size() )
			{
				throw IOException( "SGIImageReader: Error occurred during RLE decode while reading " + fileName() );
			}
			
			int dataOffset = dataY * dataWidth;
			
			for ( int x = xMin; x <= xMax ; ++x, ++dataOffset  )
			{
				data[dataOffset] = converter( scanline[x] );
			}
		}
	}
	else /// Not RLE
	{		
		int dataY = 0;

		for ( int y = dataWindow.min.y; y <= dataWindow.max.y; ++y, ++dataY )
		{
			HalfVectorData::ValueType::size_type dataOffset = dataY * dataWidth;

			for ( int x = dataWindow.min.x; x <= dataWindow.max.x; ++x, ++dataOffset )
			{
				assert( dataOffset < data.size() );

				/// Images are unencoded "upside down" (flipped in Y), for our purposes
				data[dataOffset] = converter( buffer[ ( channelOffset * wholeDataHeight * wholeDataWidth ) + ( wholeDataHeight - 1 - y  ) * wholeDataWidth + x  ] );
			}
		}
	}
	
	return dataContainer;
}

bool SGIImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_bufferFileName )
	{
		assert( m_header );
		assert( m_buffer );
		
		return true;
	}

	try
	{
		m_bufferFileName = fileName();
		m_buffer = 0;
		m_header = boost::shared_ptr< Header > ( new Header() );

		ifstream in( m_bufferFileName.c_str() );

		if ( !in.is_open() || in.fail() )
		{
			throw IOException( "SGIImageReader: Could not open " + fileName() );
		}
		
		in.read( reinterpret_cast<char*>(&m_header->m_fileHeader), sizeof(Header::FileHeader) );
		if ( in.fail() )
		{
			throw IOException( "SGIImageReader: Error reading " + fileName() );
		}
		
		m_reverseBytes = false;
		if ( reverseBytes( m_header->m_fileHeader.m_magic ) == 474 )
		{
			m_reverseBytes = true;
			
			m_header->m_fileHeader.m_dimension = reverseBytes( m_header->m_fileHeader.m_dimension );						
			m_header->m_fileHeader.m_xSize = reverseBytes( m_header->m_fileHeader.m_xSize );
			m_header->m_fileHeader.m_ySize = reverseBytes( m_header->m_fileHeader.m_ySize );
			m_header->m_fileHeader.m_zSize = reverseBytes( m_header->m_fileHeader.m_zSize );									
			m_header->m_fileHeader.m_pixMin = reverseBytes( m_header->m_fileHeader.m_pixMin );									
			m_header->m_fileHeader.m_pixMax = reverseBytes( m_header->m_fileHeader.m_pixMax );															
			m_header->m_fileHeader.m_colorMap = reverseBytes( m_header->m_fileHeader.m_colorMap );			
		} 
		else if ( m_header->m_fileHeader.m_magic != 474 )
		{		
			throw IOException( "SGIImageReader: Invalid magic number while reading " + fileName() );
		}
		
		if ( m_header->m_fileHeader.m_dimension != 1 && m_header->m_fileHeader.m_dimension != 2 && m_header->m_fileHeader.m_dimension != 3 )
		{
			throw IOException( "SGIImageReader: Invalid dimension while reading " + fileName() );
		}
		
		m_bufferWidth = m_header->m_fileHeader.m_xSize;
		
		if (  m_header->m_fileHeader.m_dimension == 1 )
		{
			m_bufferHeight = 1;
		}
		else
		{
			m_bufferHeight = m_header->m_fileHeader.m_ySize;		
		}
		
		if ( m_header->m_fileHeader.m_bytesPerChannel != 1 && m_header->m_fileHeader.m_bytesPerChannel != 2 )
		{
			throw IOException( "SGIImageReader: Invalid bitdepth while reading " + fileName() );
		}
		
		if ( m_header->m_fileHeader.m_colorMap != 0 )
		{
			/// Other color maps are obsolete
			throw IOException( "SGIImageReader: Unsupported color map while reading " + fileName() );
		}
		
		if ( m_header->m_fileHeader.m_dimension == 1 )
		{
			m_header->m_channelOffsets["Y"] = 0;
		}
		else if ( m_header->m_fileHeader.m_dimension == 2 )
		{
			m_header->m_channelOffsets["Y"] = 0;
		}
		else if ( m_header->m_fileHeader.m_dimension == 3 )
		{
			if ( m_header->m_fileHeader.m_zSize >= 1 )
			{
				m_header->m_channelOffsets["R"] = 0;
			}
			if ( m_header->m_fileHeader.m_zSize >= 2 )
			{
				m_header->m_channelOffsets["G"] = 1;
			}
			if ( m_header->m_fileHeader.m_zSize >= 3 )
			{
				m_header->m_channelOffsets["B"] = 2;
			}
			if ( m_header->m_fileHeader.m_zSize >= 4 )
			{
				m_header->m_channelOffsets["A"] = 3;
			}
		}
		
		in.seekg( 512, std::ios_base::beg );
		if ( in.fail() )
		{
			throw IOException( "SGIImageReader: Cannot read truncated image file " + fileName() );
		}
		
		/// Number of elements in the buffer
		uint32_t bufferSize = 0;		
		
		if ( m_header->m_fileHeader.m_storageFormat >= 1 ) 
		{
			uint32_t tableLength = m_header->m_fileHeader.m_ySize * m_header->m_fileHeader.m_zSize;
			
			m_header->m_offsetTable.resize( tableLength );
			m_header->m_lengthTable.resize( tableLength );
			
			in.read( reinterpret_cast<char*>( &m_header->m_offsetTable[0] ), sizeof( uint32_t ) * tableLength );
			if ( in.fail() )
			{
				throw IOException( "SGIImageReader: Error reading scanline offset table in " + fileName() );
			}
			
			in.read( reinterpret_cast<char*>( &m_header->m_lengthTable[0] ), sizeof( uint32_t ) * tableLength );
			if ( in.fail() )
			{
				throw IOException( "SGIImageReader: Error reading scanline length table in " + fileName() );
			}
			
			if ( m_reverseBytes )
			{
				std::transform( m_header->m_offsetTable.begin(), m_header->m_offsetTable.end(), m_header->m_offsetTable.begin(), reverseBytes<uint32_t> );			
				std::transform( m_header->m_lengthTable.begin(), m_header->m_lengthTable.end(), m_header->m_lengthTable.begin(), reverseBytes<uint32_t> );				
			}
			
			const uint32_t tableEnd = in.tellg();	
					
			for ( uint32_t i = 0; i < tableLength; i ++ )
			{
				/// Convert char-sized offsets which are relative to the start of the file into buffer-sized offsets relative to the start of the image buffer
				m_header->m_offsetTable[i] -= tableEnd;
				assert( m_header->m_offsetTable[i] % m_header->m_fileHeader.m_bytesPerChannel == 0 );				
				m_header->m_offsetTable[i] /= m_header->m_fileHeader.m_bytesPerChannel;

				/// Convert lengths, also
				assert( m_header->m_lengthTable[i] % m_header->m_fileHeader.m_bytesPerChannel == 0 );				
				m_header->m_lengthTable[i] /= m_header->m_fileHeader.m_bytesPerChannel;
								
				bufferSize = std::max( bufferSize, m_header->m_offsetTable[i] + m_header->m_lengthTable[i] );
			}								
		}
		else
		{
			if ( m_header->m_fileHeader.m_dimension == 1 )
			{
				bufferSize = m_header->m_fileHeader.m_xSize;
			}
			else if ( m_header->m_fileHeader.m_dimension == 2 )
			{
				bufferSize = m_header->m_fileHeader.m_xSize * m_header->m_fileHeader.m_ySize;
			}
			else if ( m_header->m_fileHeader.m_dimension == 3 )
			{
				bufferSize = m_header->m_fileHeader.m_xSize * m_header->m_fileHeader.m_ySize * m_header->m_fileHeader.m_zSize;
			}			
		}
		
		if ( m_header->m_fileHeader.m_bytesPerChannel == 1 )
		{
			UCharVectorDataPtr buffer = new UCharVectorData();
			buffer->writable().resize( bufferSize, 0 );
			
			in.read( reinterpret_cast<char*>( &buffer->writable()[0] ), bufferSize );
			if ( in.fail() )
			{
				throw IOException( "SGIImageReader: Error reading image data in " + fileName() );
			}
			
			m_buffer = buffer;
		}
		else
		{
			assert( m_header->m_fileHeader.m_bytesPerChannel == 2 );
			UShortVectorDataPtr buffer = new UShortVectorData();
			buffer->writable().resize( bufferSize, 0 );
			
			in.read( reinterpret_cast<char*>( &buffer->writable()[0] ), bufferSize * sizeof( uint16_t ) );
			
			if ( m_reverseBytes )
			{
				std::transform( buffer->writable().begin(), buffer->writable().end(), buffer->writable().begin(), reverseBytes<UShortVectorData::ValueType::value_type> );
			}
			
			m_buffer = buffer;
		}		
	}
	catch (...)
	{
		m_bufferFileName.clear();
		m_header.reset();
		m_buffer = 0;
		
		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	assert( m_header );
	return true;
}

