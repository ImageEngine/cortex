//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CINImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "IECore/CineonToLinearDataConversion.h"

#include "IECore/private/cineon.h"

#include "boost/format.hpp"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

struct CINImageReader::Header
{
	FileInformation m_fileInformation;
	ImageInformation m_imageInformation;
	ImageDataFormatInformation m_imageDataFormatInformation;
	ImageOriginationInformation m_imageOriginationInformation;
	
	/// Map from channel names to index into ImageInformation.channel_information array
	typedef map< string, int > ChannelOffsetMap;
	map< string, int > m_channelOffsets;
};

const Reader::ReaderDescription<CINImageReader> CINImageReader::m_readerDescription( "cin" );

CINImageReader::CINImageReader() :
		ImageReader( "CINImageReader", "Reads Kodak Cineon (CIN) files." ),
		m_header( 0 )
{
}

CINImageReader::CINImageReader( const string &fileName ) :
		ImageReader( "CINImageReader", "Reads Kodak Cineon (CIN) files." ),
		m_header( 0 )
{
	m_fileNameParameter->setTypedValue(fileName);
}

CINImageReader::~CINImageReader()
{
	delete m_header;
}

// partial validity check: assert that the file begins with the CIN magic number
bool CINImageReader::canRead( const string &fileName )
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
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	if ( in.fail() )
	{
		return false;
	}
	
	return magic == 0xd75f2a80 || magic == 0x802a5fd7;
}

void CINImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );
	
	assert( m_header );
	
	for ( Header::ChannelOffsetMap::const_iterator it = m_header->m_channelOffsets.begin(); it != m_header->m_channelOffsets.end(); ++it )
	{
		names.push_back( it->first );
	}
}

bool CINImageReader::isComplete()
{
	return open( false );
}

Box2i CINImageReader::dataWindow()
{
	open( true );

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Box2i CINImageReader::displayWindow()
{
	open( true );

	return dataWindow();
}

/// \todo
/// we assume here CIN coding in the 'typical' configuration (output by film dumps, nuke, etc).
/// this is RGB 10bit log for film, pixel-interlaced data.  we convert this to a linear 16-bit (half)
/// format in the ImagePrimitive.
DataPtr CINImageReader::readChannel( const string &name, const Imath::Box2i &dataWindow )
{
	if ( !open() )
	{
		return 0;
	}
	
	assert( m_header );
	
	// figure out the offset into the bitstream for the given channel
	assert( m_header->m_channelOffsets.find( name ) != m_header->m_channelOffsets.end() );
	int channelOffset = m_header->m_channelOffsets[name];

	assert( (int)m_header->m_imageInformation.channel_information[channelOffset].bpp == 10 );
	int bpp = m_header->m_imageInformation.channel_information[channelOffset].bpp;
	
	// form the mask for this channel
	unsigned int mask = 0;
	for (int pi = 0; pi < bpp; ++pi)
	{
		mask = 1 + (mask << 1);
	}
	mask <<= ((32 - bpp) - channelOffset * bpp);
	
	CineonToLinearDataConversion< unsigned short, half > converter;

	HalfVectorDataPtr dataContainer = new HalfVectorData();
	HalfVectorData::ValueType &data = dataContainer->writable();
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;
	
	Box2i wholeDataWindow = this->dataWindow();
	
	const int yMin = dataWindow.min.y - wholeDataWindow.min.y;	
	const int yMax = dataWindow.max.y - wholeDataWindow.min.y;	
	
	const int xMin = dataWindow.min.x - wholeDataWindow.min.x;
	const int xMax = dataWindow.max.x - wholeDataWindow.min.x;			

	int dataY = 0;
	for ( int y = yMin ; y <= yMax ; ++y, ++dataY )
	{
		HalfVectorData::ValueType::size_type dataOffset = dataY * dataWidth;
		std::vector<unsigned int>::size_type bufferOffset = y * m_bufferWidth + xMin;

		for ( int x = xMin;  x <= xMax ; ++x, ++dataOffset, ++bufferOffset  )
		{
			assert( dataOffset < data.size() );

			unsigned int cell = m_buffer[ bufferOffset ];
			if ( m_reverseBytes )
			{
				cell = reverseBytes( cell );
			}

			// assume we have 10bit log, two wasted bits aligning to 32 longword
			unsigned short cv = (unsigned short) ( ( mask & cell ) >> ( 2 + ( 2 - channelOffset ) * bpp ) );
			data[dataOffset] = converter( cv );
		}
	}

	return dataContainer;
}

bool CINImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_bufferFileName )
	{
		assert( m_header );
		
		return true;
	}

	try
	{
		m_bufferFileName = fileName();
		m_buffer.clear();
		delete m_header;
		m_header = new Header();

		ifstream in( m_bufferFileName.c_str() );

		if ( !in.is_open() || in.fail() )
		{
			throw IOException( "CINImageReader: Could not open " + fileName() );
		}
		
		in.read( reinterpret_cast<char*>(&m_header->m_fileInformation), sizeof(m_header->m_fileInformation) );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}
		
		/// This code works correctly on both big- and little-endian platforms
		m_reverseBytes = false;
		if ( m_header->m_fileInformation.magic == 0xd75f2a80 )
		{
			m_reverseBytes = true;
		} 
		else if ( m_header->m_fileInformation.magic != 0x802a5fd7 )
		{		
			throw IOException( "CINImageReader: Invalid Cineon magic number while reading " + fileName() );
		}
		
		in.read( reinterpret_cast<char*>(&m_header->m_imageInformation), sizeof(m_header->m_imageInformation) );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}
		
		in.read( reinterpret_cast<char*>(&m_header->m_imageDataFormatInformation), sizeof(m_header->m_imageDataFormatInformation) );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}
		
		if ( (int)m_header->m_imageDataFormatInformation.packing != 5 )
		{
			throw IOException( "CINImageReader: Unsupported data packing in file " + fileName() );
		}
		
		if ( (int)m_header->m_imageDataFormatInformation.interleave != 0 )
		{
			throw IOException( "CINImageReader: Unsupported data interleaving in file " + fileName() );
		}
		
		if ( (int)m_header->m_imageDataFormatInformation.data_signed != 0 )
		{
			throw IOException( "CINImageReader: Unsupported data signing in file " + fileName() );
		}
		
		if ( (int)m_header->m_imageDataFormatInformation.sense != 0 )
		{
			throw IOException( "CINImageReader: Unsupported data sense in file " + fileName() );
		}		
		
		if ( (int)m_header->m_imageDataFormatInformation.eol_padding != 0 || (int)m_header->m_imageDataFormatInformation.eoc_padding != 0 )
		{
			throw IOException( "CINImageReader: Unsupported data padding in file " + fileName() );
		}				
		
		in.read( reinterpret_cast<char*>(&m_header->m_imageOriginationInformation), sizeof(m_header->m_imageOriginationInformation) );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}

		if ( m_reverseBytes )
		{
			m_header->m_fileInformation.image_data_offset = reverseBytes( m_header->m_fileInformation.image_data_offset );
			m_header->m_fileInformation.industry_header_length = reverseBytes( m_header->m_fileInformation.industry_header_length );
			m_header->m_fileInformation.variable_header_length = reverseBytes( m_header->m_fileInformation.variable_header_length );
		}
		
		if ( (int)m_header->m_imageInformation.orientation != 0 )
		{
			throw IOException( "CINImageReader: Unsupported image orientation in file " + fileName() );
		}

		for ( int i = 0; i < (int)m_header->m_imageInformation.channel_count; ++i )
		{
			ImageInformationChannelInformation &channelInformation = m_header->m_imageInformation.channel_information[i];

			if ( m_reverseBytes )
			{
				channelInformation.pixels_per_line = reverseBytes( channelInformation.pixels_per_line );	
				channelInformation.lines_per_image = reverseBytes( channelInformation.lines_per_image );
			}

			if ( i == 0 )
			{
				m_bufferWidth = channelInformation.pixels_per_line;
				m_bufferHeight = channelInformation.lines_per_image;
			}
			else
			{
				if ( channelInformation.pixels_per_line != m_bufferWidth || channelInformation.lines_per_image != m_bufferHeight )
				{
					throw IOException( "CINImageReader: Cannot read channels of differing dimensions in file " + fileName() );
				}
			}
						
			if ( (int)channelInformation.bpp != 10 )
			{
				throw IOException( ( boost::format( "CINImageReader: Unsupported bits-per-pixel (%d) in file %s ") % (int)channelInformation.bpp % fileName() ).str() );
			}

			if ( channelInformation.byte_0 == 1 )
			{
				throw IOException( "CINImageReader: Cannot read vendor specific Cineon file " + fileName() );
			}

			if ( channelInformation.byte_1 == 0 )
			{
				m_header->m_channelOffsets["Y"] = i;
			}
			else if ( channelInformation.byte_1 == 1 )
			{
				m_header->m_channelOffsets["R"] = i;
			}
			else if ( channelInformation.byte_1 == 2 )
			{
				m_header->m_channelOffsets["G"] = i;
			} 
			else if ( channelInformation.byte_1 == 3 )
			{
				m_header->m_channelOffsets["B"] = i;
			} 
			else
			{
				throw IOException( "CINImageReader: Unsupported channel type while reading " + fileName() );
			}
			
			/// \todo Because we only deal with 10-bit values packed into 32-bit structures, we can only handle a maximum of 3
			/// channels for the time being.
			if ( m_header->m_channelOffsets.size() > 3 )
			{
				throw IOException( "CINImageReader: Unsupported number of channels while reading " + fileName() );
			}
		}

		// seek to the image data offset
		in.seekg( m_header->m_fileInformation.image_data_offset, ios_base::beg );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}

		// Read the data into the buffer - remember that we're currently packing upto 3 channels into each 32-bit "cell"
		int bufferSize = ( std::max<unsigned int>( 1u, m_header->m_channelOffsets.size() / 3 ) ) * m_bufferWidth * m_bufferHeight;
		m_buffer.resize( bufferSize, 0 );
		
		in.read( reinterpret_cast<char*>(&m_buffer[0]), sizeof(unsigned int) * bufferSize );
		if ( in.fail() )
		{
			throw IOException( "CINImageReader: Error reading " + fileName() );
		}		
	}
	catch (...)
	{
		delete m_header;
		m_header = 0;
		
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

