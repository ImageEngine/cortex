//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014-2015, Image Engine Design Inc. All rights reserved.
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
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfPartType.h"
#include "OpenEXR/ImfDeepFrameBuffer.h"

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/convenience.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "IECore/EXRDeepImageWriter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( EXRDeepImageWriter );

inline size_t pixelTypeSize( Imf::PixelType p )
{
	return ( p == Imf::FLOAT ) ? sizeof( float ) : sizeof( half );
}

const DeepImageWriter::DeepImageWriterDescription<EXRDeepImageWriter> EXRDeepImageWriter::g_writerDescription( "dexr exr" );

void EXRDeepImageWriter::appendParameters()
{
	StringVectorDataPtr defaultChannels = new StringVectorData();
	std::vector<std::string> &channels = defaultChannels->writable();
	channels.push_back( "R" );
	channels.push_back( "G" );
	channels.push_back( "B" );
	channels.push_back( "A" );
	m_halfChannelsParameter = new StringVectorParameter( "halfPrecisionChannels", "The names of the channels which should be written using half precision.", defaultChannels );
	parameters()->addParameter( m_halfChannelsParameter );
	
	IECore::IntParameter::PresetsContainer presets;
	presets.push_back( IECore::IntParameter::Preset( "None", Imf::NO_COMPRESSION ) ); // No compression.
	presets.push_back( IECore::IntParameter::Preset( "RLE", Imf::RLE_COMPRESSION ) ); // Run length encoding.
	presets.push_back( IECore::IntParameter::Preset( "ZIPS", Imf::ZIPS_COMPRESSION ) ); // Zlib compression, one scan line at a time.
	m_compressionParameter = new IntParameter( "compression", "The type of EXR compression.", Imf::ZIPS_COMPRESSION, 0, presets.back().second, presets, true );
	parameters()->addParameter( m_compressionParameter );
}

EXRDeepImageWriter::EXRDeepImageWriter()
	:	DeepImageWriter( "Writes EXR 2.0 deep image file format." ),
		m_outputFile( 0 ),
		m_numberOfFloatChannels( 0 ),
		m_numberOfHalfChannels( 0 )
{
	appendParameters();
}

EXRDeepImageWriter::EXRDeepImageWriter( const std::string &fileName )
	:	DeepImageWriter( "Writes EXR 2.0 deep image file format." ),
		m_outputFile( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );

	appendParameters();
}

EXRDeepImageWriter::~EXRDeepImageWriter()
{
	open();
	
	// Write any remaining scanlines.
	while( m_currentSlice <= m_lastSlice )
	{
		writeScanline();
	}
	
	// Free our memory.	
	clearScanlineBuffer();
	delete m_outputFile;
}

bool EXRDeepImageWriter::canWrite( const std::string &fileName )
{
	std::string ext = boost::filesystem::extension( boost::filesystem::path( fileName ) );
	if ( ext != ".exr" && ext != ".dexr" )
	{
		return false;
	}
	
	Imf::DeepScanLineOutputFile *file( NULL );

	try
	{
		// Create the header.
		Imf::Header header( 1, 1 );
		header.setType( Imf::DEEPSCANLINE );
		header.compression() = Imf::NO_COMPRESSION;
		file = new Imf::DeepScanLineOutputFile( fileName.c_str(), header );
	}
	catch( ... )
	{
		delete file;
		return false;
	}

	delete file;
	return true;	
}

void EXRDeepImageWriter::clearScanlineBuffer()
{
	unsigned int numChannels = numberOfChannels();

	if( m_sampleCount.size() == ( unsigned int ) m_width )
	{
		memset( &m_sampleCount[0], 0, sizeof( unsigned int ) * m_width );
	}
	else
	{
		m_sampleCount.resize( m_width, 0 );
		m_samplePointers.resize( m_width * numChannels, NULL );
		m_floatSamples.resize( m_width * m_numberOfFloatChannels );
		m_halfSamples.resize( m_width * m_numberOfHalfChannels );
		m_depthSamples.resize( m_width );
		m_depthPointers.resize( m_width, NULL );
	}
}

void EXRDeepImageWriter::writeScanline()
{
	Imath::Box2i dataWindow( m_outputFile->header().dataWindow() );
	if ( m_currentSlice <= m_lastSlice )
	{
		Imf::DeepFrameBuffer frameBuffer;

		// Add the sample counts.
		frameBuffer.insertSampleCountSlice( 
				Imf::Slice( Imf::UINT, reinterpret_cast< char *>( &m_sampleCount[0] - dataWindow.min.x - m_currentSlice * m_width ),
					sizeof( unsigned int ) * 1,
					sizeof( unsigned int ) * m_width
					)
				);
		
		// Write the auxiliary channels.
		for( unsigned int c = 0; c < numberOfChannels(); ++c )
		{
			Imf::ChannelList::ConstIterator it = m_outputFile->header().channels().find( channelName( c ) );
			
			// Add the auxiliary channels. 
			Imf::DeepSlice slice(
					it.channel().type, reinterpret_cast< char * >( &m_samplePointers[ m_width * c ] - dataWindow.min.x - m_currentSlice * m_width ),
					sizeof( void * ), sizeof( void * ) * m_width, pixelTypeSize( it.channel().type )
					);
			frameBuffer.insert( it.name(), slice );
		}
	
		// Add the Z channel.	
		Imf::DeepSlice slice(
				Imf::FLOAT, reinterpret_cast< char * >( &m_depthPointers[0] - dataWindow.min.x - m_currentSlice * m_width ),
				sizeof( float * ), sizeof( float * ) * m_width, sizeof( float )
				);
		frameBuffer.insert( "Z", slice );

		m_outputFile->setFrameBuffer( frameBuffer );

		m_outputFile->writePixels(1);

		++m_currentSlice;
	}
	else
	{
		m_currentSlice = m_lastSlice + 1;
	}

	// Clear the scanline buffer.
	clearScanlineBuffer();
}

void EXRDeepImageWriter::doWritePixel( int x, int y, const DeepPixel *pixel )
{
	open();
	
	if ( y < m_currentSlice )
	{
		throw Exception( "Deep slices have to be written sequentially and the pixel to be written belongs to a slice that has already been written." );
	}
	
	if ( y > m_currentSlice )
	{
		do
		{
			writeScanline();
		} while( m_currentSlice < std::min( y, m_lastSlice ) );
	}
		
	if ( y > m_lastSlice )
	{
		throw Exception( "Cannot write past the bounds of the deep image." );
	}

	// Write the number of samples.
	const unsigned int numSamples = pixel->numSamples();
	size_t xOffset = x - m_outputFile->header().dataWindow().min.x;
	m_sampleCount[ xOffset ] = numSamples;

	// Write the Z channel.
	m_depthSamples[ xOffset ].clear();
	m_depthSamples[ xOffset ].reserve( numSamples );
	m_depthPointers[ xOffset ] = &m_depthSamples[ xOffset ][0];
	for ( unsigned int i = 0; i < numSamples; ++i )
	{
		m_depthSamples[ xOffset ].push_back( pixel->getDepth( i ) );
	}

	// Write the auxiliary channels.
	for( unsigned int i = 0, floatCount = 0, halfCount = 0; i < numberOfChannels(); ++i )
	{
		Imf::ChannelList::ConstIterator it = m_outputFile->header().channels().find( channelName( i ) );

		int pointerIndex = m_width * i + xOffset;
		if ( m_channelTypes[i] == Imf::FLOAT )
		{
			int index = m_width * floatCount + xOffset;
			m_floatSamples[ index ].clear();
			m_floatSamples[ index ].reserve( numSamples );
			m_samplePointers[ pointerIndex ] = &m_floatSamples[ index ][0];
			
			for ( unsigned i = 0; i < numSamples; ++i )
			{
				 m_floatSamples[ index ].push_back( pixel->channelData( i )[ pixel->channelIndex( it.name() ) ] );
			}

			++floatCount;
		}
		else
		{
			int index = m_width * halfCount + xOffset;
			m_halfSamples[ index ].clear();
			m_halfSamples[ index ].reserve( numSamples );
			m_samplePointers[ pointerIndex ] = &m_halfSamples[ index ][0];
			
			for ( unsigned i = 0; i < numSamples; ++i )
			{
				 m_halfSamples[ index ].push_back( pixel->channelData( i )[ pixel->channelIndex( it.name() ) ] );
			}

			++halfCount;
		}
	}
}

Imf::Compression EXRDeepImageWriter::compression() const
{
	return static_cast< Imf::Compression >( parameters()->parameter<IECore::IntParameter>("compression")->getNumericValue() );
}

unsigned int EXRDeepImageWriter::numberOfChannels() const
{
	return m_channelsParameter->getTypedValue().size();
}

const std::string &EXRDeepImageWriter::channelName( unsigned int index ) const
{
	return m_channelsParameter->getTypedValue()[index];
}

void EXRDeepImageWriter::open()
{
	if ( m_outputFile && fileName() == m_outputFile->fileName() )
	{
		// we already opened the right file successfully
		return;
	}

	delete m_outputFile;
	m_channelTypes.clear();
	m_outputFile = 0;
	m_numberOfFloatChannels = 0;
	m_numberOfHalfChannels = 0;

	/// \todo: Once data windows and display windows are supported, add them here.
	const Imath::V2i &resolution = m_resolutionParameter->getTypedValue();
	Imath::Box2i displayWindow( Imath::V2i( 0, 0 ), Imath::V2i( resolution.x - 1, resolution.y - 1 ) );
	Imath::Box2i dataWindow( Imath::V2i( 0, 0 ), Imath::V2i( resolution.x - 1, resolution.y - 1 ) );
	m_height = dataWindow.max.y - dataWindow.min.y + 1;
	m_width = dataWindow.max.x - dataWindow.min.x + 1;

	m_currentSlice = dataWindow.min.y;
	m_lastSlice = dataWindow.max.y;
	
	// Create the header.	
	Imf::Header header( displayWindow, dataWindow );	
	header.setType( Imf::DEEPSCANLINE );
	header.compression() = compression();
	
	// Add the channels.
	const std::vector<std::string> &halfChannels = m_halfChannelsParameter->getTypedValue();
		
	for ( unsigned int i = 0; i < numberOfChannels(); ++i )
	{
		if( std::find( halfChannels.begin(), halfChannels.end(), channelName(i) ) == halfChannels.end() )
		{
			header.channels().insert( channelName(i), Imf::Channel( Imf::FLOAT ) );
			m_channelTypes.push_back( Imf::FLOAT );
			++m_numberOfFloatChannels;
		}
		else
		{
			header.channels().insert( channelName(i), Imf::Channel( Imf::HALF ) );
			m_channelTypes.push_back( Imf::HALF );
			++m_numberOfHalfChannels;
		}
	}

	header.channels().insert( "Z", Imf::Channel( Imf::FLOAT ) );

	m_outputFile = new Imf::DeepScanLineOutputFile( fileName().c_str(), header );

	// Allocate a new scanline buffer.
	clearScanlineBuffer();
}

