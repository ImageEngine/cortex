//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DPXImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/CineonToLinearDataConversion.h"

#include "IECore/private/dpx.h"

#include "boost/format.hpp"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED(DPXImageReader);

struct DPXImageReader::Header
{
	DPXFileInformation m_fileInformation;
	DPXImageInformation m_imageInformation;
	DPXImageOrientation m_imageOrientation;
};

const Reader::ReaderDescription<DPXImageReader> DPXImageReader::m_readerDescription("dpx");

DPXImageReader::DPXImageReader() :
		ImageReader("DPXImageReader", "Reads Digital Picture eXchange (DPX) files."),
		m_header( 0 )
{
}

DPXImageReader::DPXImageReader(const string & fileName) :
		ImageReader("DPXImageReader", "Reads Digital Picture eXchange (DPX) files."),
		m_header( 0 )
{
	m_fileNameParameter->setTypedValue(fileName);
}

DPXImageReader::~DPXImageReader()
{
	delete m_header;
}

// partial validity check: assert that the file begins with the DPX magic number
bool DPXImageReader::canRead( const string &fileName )
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// seek to start
	in.seekg(0, ios_base::beg);

	// check magic number
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	return magic == 0x53445058 || magic == 0x58504453;
}

void DPXImageReader::channelNames(vector<string> & names)
{
	names.clear();

	/// \todo read the channel names from the DPX header
	// form channel names - hardcoded for now, assume RGB (in 10bit log)
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
}

bool DPXImageReader::isComplete()
{
	return open( false );
}

Box2i DPXImageReader::dataWindow()
{
	open( true );

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Box2i DPXImageReader::displayWindow()
{
	open( true );

	return dataWindow();
}

/// \todo
/// we assume here DPX coding in the 'typical' configuration (output by film dumps, nuke, etc).
/// this is RGB 10bit log for film, pixel-interlaced data.  we convert this to a linear 16-bit (half)
/// format in the ImagePrimitive.
DataPtr DPXImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	if (!open())
	{
		return 0;
	}

	/// \todo
	// kinda useless here, we have implicitly assumed log 10 bit in the surrounding code
	int bpp = 10;

	/// \todo
	// figure out the offset into the bitstream for the given channel
	int channelOffset = name == "R" ? 0 : name == "G" ? 1 : 2;

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
			assert( cv < 1024 );
			data[dataOffset] = converter( cv );
		}
	}

	return dataContainer;
}

bool DPXImageReader::open( bool throwOnFailure )
{
	if (m_bufferFileName == fileName())
	{
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
			throw IOException( "DPXImageReader: Could not open " + fileName() );
		}

		in.read(reinterpret_cast<char*>(&m_header->m_fileInformation), sizeof(m_header->m_fileInformation));
		if ( in.fail() )
		{
			throw IOException( "DPXImageReader: Error reading " + fileName() );
		}

		in.read(reinterpret_cast<char*>(&m_header->m_imageInformation), sizeof(m_header->m_imageInformation));
		if ( in.fail() )
		{
			throw IOException( "DPXImageReader: Error reading " + fileName() );
		}

		in.read(reinterpret_cast<char*>(&m_header->m_imageOrientation), sizeof(m_header->m_imageOrientation));
		if ( in.fail() )
		{
			throw IOException( "DPXImageReader: Error reading " + fileName() );
		}

		m_reverseBytes = false;
		if ( m_header->m_fileInformation.magic == 0x58504453 )
		{
			m_reverseBytes = true;
		}
		else if ( m_header->m_fileInformation.magic != 0x53445058 )
		{
			throw IOException( "DPXImageReader: Invalid DPX magic number while reading " + fileName() );
		}

		if ( m_reverseBytes )
		{
			m_header->m_fileInformation.image_data_offset    = reverseBytes(m_header->m_fileInformation.image_data_offset);
			m_header->m_imageInformation.element_number      = reverseBytes(m_header->m_imageInformation.element_number);
			m_header->m_imageInformation.pixels_per_line     = reverseBytes(m_header->m_imageInformation.pixels_per_line);
			m_header->m_imageInformation.lines_per_image_ele = reverseBytes(m_header->m_imageInformation.lines_per_image_ele);
			
			for ( unsigned i = 0; i < 8; i ++)
			{
				m_header->m_imageInformation.image_element[i].packing = reverseBytes(m_header->m_imageInformation.image_element[i].packing);
				m_header->m_imageInformation.image_element[i].encoding = reverseBytes(m_header->m_imageInformation.image_element[i].encoding);
			}
		}
		
		if ( m_header->m_imageInformation.element_number != 1 )
		{
			throw IOException( "DPXImageReader: Invalid number of elements in image while reading " + fileName() );
		}
		
		if ( m_header->m_imageInformation.image_element[0].bit_size != 10 )
		{
			throw IOException( "DPXImageReader: Invalid bitdepth (only 10-bit images are supported) while reading " + fileName() );
		}
		
		if ( m_header->m_imageInformation.image_element[0].descriptor != 50 )
		{
			throw IOException( boost::str( boost::format( "DPXImageReader: Cannot read image '%s' of type '%s' ( only RGB are supported)" ) % fileName() % descriptorStr( m_header->m_imageInformation.image_element[0].descriptor ) ) );
		}
		
		if ( m_header->m_imageInformation.image_element[0].packing != 1 )
		{
			throw IOException( "DPXImageReader: Found invalid image packing while reading " + fileName() );
		}
		
		if ( m_header->m_imageInformation.image_element[0].encoding != 0 )
		{
			throw IOException( "DPXImageReader: Found invalid image encoding while reading " + fileName() );
		}

		m_bufferWidth = m_header->m_imageInformation.pixels_per_line;
		m_bufferHeight = m_header->m_imageInformation.lines_per_image_ele;
		in.seekg( m_header->m_fileInformation.image_data_offset, ios_base::beg );
		if ( in.fail() )
		{
			throw IOException( "DPXImageReader: Error reading " + fileName() );
		}
		
		// Read the data into the buffer - remember that we're currently packing upto 3 channels into each 32-bit "cell"
		int bufferSize = ( std::max<unsigned int>( 1u, 3 / 3 ) ) * m_bufferWidth * m_bufferHeight;
		m_buffer.resize( bufferSize, 0 );

		in.read( reinterpret_cast<char*>(&m_buffer[0]), sizeof(unsigned int) * bufferSize );
		if ( in.fail() )
		{
			throw IOException( "DPXImageReader: Error reading " + fileName() );
		}
	}
	catch (...)
	{
		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	return true;
}

const char* DPXImageReader::descriptorStr( int descriptor ) const
{
	switch ( descriptor ) 
	{	
		case 0 :
		case 150:
		case 151:
		case 152:
		case 153:
		case 154:
		case 155:
		case 156: 
			  return "User-defined";
		case 1  : return "Red";
		case 2  : return "Green";
		case 3  :  return "Blue";
		case 4  : return "Alpha";
		case 6  : return "Luminance";
		case 7  : return "Chrominance";
		case 8  : return "Depth";
		case 9  : return "Composite video";
		case 50 : return "RGB";
		case 51 : return "RGBA";
		case 52 : return "ABGR";
		case 100: return "CbYCrY";
		case 101: return "CbYaCrYa";
		case 102: return "CbYCr";		
		case 103: return "CbYCra";
		
		default : return "Unknown";						
	}
	
	assert( false );
	return 0;
}
