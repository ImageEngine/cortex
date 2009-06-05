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

#include <fstream>
#include <stdint.h>

#include "boost/multi_array.hpp"
#include "boost/format.hpp"

#include "IECore/TGAImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/CompoundDataConversion.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/BoxOps.h"

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( TGAImageWriter )
const Writer::WriterDescription<TGAImageWriter> TGAImageWriter::m_writerDescription( "tga" );

TGAImageWriter::TGAImageWriter() :
		ImageWriter( "TGAImageWriter", "Serializes images to the Truevision Targa format" )
{
}

TGAImageWriter::TGAImageWriter( ObjectPtr image, const string &fileName ) :
		ImageWriter( "TGAImageWriter", "Serializes images to the Truevision Targa format" )
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

TGAImageWriter::~TGAImageWriter()
{
}

std::string TGAImageWriter::destinationColorSpace() const
{
	return "srgb";
}

struct TGAImageWriter::ChannelConverter
{
	typedef void ReturnType;

	std::string m_channelName;
	ConstImagePrimitivePtr m_image;
	Box2i m_dataWindow;
	int m_numChannels;
	int m_channelOffset;
	std::vector<uint8_t> &m_imageBuffer;

	ChannelConverter( const std::string &channelName, ConstImagePrimitivePtr image, const Box2i &dataWindow, int numChannels, int channelOffset, std::vector<uint8_t> &imageBuffer )
			: m_channelName( channelName ), m_image( image ), m_dataWindow( dataWindow ), m_numChannels( numChannels ), m_channelOffset( channelOffset ), m_imageBuffer( imageBuffer )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr dataContainer )
	{
		assert( dataContainer );

		const typename T::ValueType &data = dataContainer->readable();
		ScaledDataConversion<typename T::ValueType::value_type, uint8_t> converter;

		typedef boost::multi_array_ref< const typename T::ValueType::value_type, 2 > SourceArray2D;
		typedef boost::multi_array_ref< uint8_t, 3 > TargetArray3D;

		const SourceArray2D sourceData( &data[0], extents[ m_image->getDataWindow().size().y + 1 ][ m_image->getDataWindow().size().x + 1 ] );
		TargetArray3D targetData( &m_imageBuffer[0], extents[ m_image->getDisplayWindow().size().y + 1 ][ m_image->getDisplayWindow().size().x + 1 ][ m_numChannels ] );

		const Box2i copyRegion = boxIntersection( m_dataWindow, boxIntersection( m_image->getDisplayWindow(), m_image->getDataWindow() ) );

		for ( int y = copyRegion.min.y; y <= copyRegion.max.y ; y++ )
		{
			for ( int x = copyRegion.min.x; x <= copyRegion.max.x ; x++ )
			{
				targetData[ y - m_image->getDisplayWindow().min.y + copyRegion.min.y ][ x - m_image->getDisplayWindow().min.x + copyRegion.min.x ][ m_channelOffset ]
				= converter( sourceData[ y - m_image->getDataWindow().min.y ][ x - m_image->getDataWindow().min.x ] );
			}
		}
	};

	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			throw InvalidArgumentException(( boost::format( "TGAImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_channelName ).str() );
		}
	};
};

template<typename T>
static void writeLittleEndian( std::ostream &f, T n )
{
	const T nl = asLittleEndian<>( n );
	f.write(( const char* ) &nl, sizeof( T ) );
}

void TGAImageWriter::writeImage( const vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow ) const
{
	vector<string>::const_iterator rIt = std::find( names.begin(), names.end(), "R" );
	vector<string>::const_iterator gIt = std::find( names.begin(), names.end(), "G" );
	vector<string>::const_iterator bIt = std::find( names.begin(), names.end(), "B" );
	vector<string>::const_iterator aIt = std::find( names.begin(), names.end(), "A" );

	int numChannels = 0;
	if ( rIt != names.end() && gIt != names.end() && bIt != names.end() )
	{
		numChannels = 3;
	}
	if ( aIt != names.end() )
	{
		numChannels ++;
	}

	if ( numChannels != 3 && numChannels != 4 )
	{
		throw IOException( "TGAImageWriter: Unsupported channel names specified." );
	}

	std::ofstream out;
	out.open( fileName().c_str() );
	if ( !out.is_open() )
	{
		throw IOException( "TGAImageWriter: Could not open " + fileName() );
	}

	vector<string> desiredChannelOrder;
	desiredChannelOrder.push_back( "B" );
	desiredChannelOrder.push_back( "G" );
	desiredChannelOrder.push_back( "R" );
	desiredChannelOrder.push_back( "A" );

	vector<string> namesCopy = names;
	vector<string> filteredNames;

	for ( vector<string>::const_iterator it = desiredChannelOrder.begin(); it != desiredChannelOrder.end(); ++it )
	{
		vector<string>::iterator res = find( namesCopy.begin(), namesCopy.end(), *it );
		if ( res != namesCopy.end() )
		{
			namesCopy.erase( res );
			filteredNames.push_back( *it );
		}
	}

	for ( vector<string>::const_iterator it = namesCopy.begin(); it != namesCopy.end(); ++it )
	{
		filteredNames.push_back( *it );
	}

	assert( names.size() == filteredNames.size() );

	Box2i displayWindow = image->getDisplayWindow();

	int displayWidth  = 1 + displayWindow.size().x;
	int displayHeight = 1 + displayWindow.size().y;

	// Write the header

	/// ID Length
	writeLittleEndian<uint8_t>( out, 0 );

	/// Color Map Type
	writeLittleEndian<uint8_t>( out, 0 );

	/// Image Type
	writeLittleEndian<uint8_t>( out, 2 );

	/// Color Map Specification
	writeLittleEndian<uint16_t>( out, 0 );
	writeLittleEndian<uint16_t>( out, 0 );
	writeLittleEndian<uint8_t>( out, 0 );

	/// Image Specification
	writeLittleEndian<uint16_t>( out, 0 );
	writeLittleEndian<uint16_t>( out, 0 );
	writeLittleEndian<uint16_t>( out, displayWidth );
	writeLittleEndian<uint16_t>( out, displayHeight );
	writeLittleEndian<uint8_t>( out, numChannels * 8 );
	writeLittleEndian<uint8_t>( out, ( numChannels == 4 ? 8 : 0 ) + 32 );

	// Encode the image buffer
	std::vector<uint8_t> imageBuffer( displayWidth*displayHeight*numChannels, 0 );

	int offset = 0;
	for ( vector<string>::const_iterator it = filteredNames.begin(); it != filteredNames.end(); ++it )
	{
		const std::string &name = *it;
		if ( !( name == "R" || name == "G" || name == "B" || name == "A" ) )
		{
			msg( Msg::Warning, "TGAImageWriter::write", format( "Channel \"%s\" was not encoded." ) % name );
		}
		else
		{
			assert( image->variables.find( name ) != image->variables.end() );
			DataPtr dataContainer = image->variables.find( name )->second.data;
			assert( dataContainer );
			ChannelConverter converter( name, image, dataWindow, numChannels, offset, imageBuffer );
			despatchTypedData<
			ChannelConverter,
			TypeTraits::IsNumericVectorTypedData,
			ChannelConverter::ErrorHandler
			>( dataContainer, converter );

			++offset;
		}
	}

	/// Write the Buffer
	for ( int i = 0; i < displayWidth*displayHeight*numChannels; ++i )
	{
		assert( i < ( int )imageBuffer.size() );
		writeLittleEndian( out, imageBuffer[i] );
		if ( out.fail() )
		{
			throw IOException( "TGAImageWriter: Error writing to " + fileName() );
		}
	}
}
