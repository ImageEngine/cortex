//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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
#include "boost/algorithm/string.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/format.hpp"

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImfArray.h"
#include "OpenEXR/ImfAttribute.h"
#include "OpenEXR/ImfMatrixAttribute.h"
#include "OpenEXR/half.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfTestFile.h"
#include "OpenEXR/ImfDeepFrameBuffer.h"
#include "OpenEXR/ImfPartType.h"

#include "IECore/EXRDeepImageReader.h"
#include "IECore/FileNameParameter.h"
#include "IECore/MessageHandler.h"

using namespace IECore;

inline size_t pixelTypeSize( Imf::PixelType p )
{
	return ( p == Imf::FLOAT ) ? sizeof( float ) : sizeof( half );
}

IE_CORE_DEFINERUNTIMETYPED( EXRDeepImageReader );

const Reader::ReaderDescription<EXRDeepImageReader> EXRDeepImageReader::g_readerDescription( "dexr exr" );

EXRDeepImageReader::EXRDeepImageReader()
	:	DeepImageReader( "Reads EXR 2.0 deep image file format." ),
		m_cache( 0 ), m_inputFile( 0 ), m_depthChannel( -1 ), m_channelNames()
{
}

EXRDeepImageReader::EXRDeepImageReader( const std::string &fileName )
	:	DeepImageReader( "Reads EXR 2.0 deep image file format." ),
		m_cache( 0 ), m_inputFile( 0 ), m_depthChannel( -1 ), m_channelNames()
{
	m_fileNameParameter->setTypedValue( fileName );
}

EXRDeepImageReader::~EXRDeepImageReader()
{
	delete m_cache;
	delete m_inputFile;
}

bool EXRDeepImageReader::canRead( const std::string &fileName )
{
	if( !Imf::isOpenExrFile( fileName.c_str() ) )
	{
		return false;
	}

	Imf::DeepScanLineInputFile *inputFile( NULL );
	try
	{
		inputFile = new Imf::DeepScanLineInputFile( fileName.c_str() );

		if ( inputFile->header().hasType() )
		{
			if ( inputFile->header().type() != Imf::DEEPSCANLINE )
			{
				throw IOException( "EXR is not a deep image." );
			}
		}
	}
	catch( ... )
	{
		delete inputFile;
		return false;
	}
	delete inputFile;

	return true;
}

void EXRDeepImageReader::channelNames( std::vector<std::string> &names )
{
	open( true );
	
	unsigned numChannels = m_channelNames.size();
	names.resize( numChannels );
	for ( unsigned i=0; i < numChannels; ++i )
	{
		names[i] = m_channelNames[i];
	}
}

bool EXRDeepImageReader::isComplete()
{
	if( !open() )
	{
		return false;
	}
	
	return m_inputFile->isComplete();
}

Imath::M44f EXRDeepImageReader::worldToNDCMatrix()
{
	open( true );
	
	const Imf::M44fAttribute *ndc = m_inputFile->header().findTypedAttribute< Imf::M44fAttribute >( std::string( "worldToNDC" ) );
	if( ndc )
	{
		return ndc->value();
	}

	return Imath::M44f();
}

Imath::M44f EXRDeepImageReader::worldToCameraMatrix()
{
	open( true );
	
	const Imf::M44fAttribute *worldToCamera = m_inputFile->header().findTypedAttribute< Imf::M44fAttribute >( std::string( "worldToCamera" ) );
	if( worldToCamera )
	{
		return worldToCamera->value();
	}

	return Imath::M44f();
}

Imath::Box2i EXRDeepImageReader::dataWindow()
{
	open( true );
	
	return m_inputFile->header().dataWindow();
}

Imath::Box2i EXRDeepImageReader::displayWindow()
{
	open( true );
	
	return m_inputFile->header().displayWindow();
}

DeepPixelPtr EXRDeepImageReader::doReadPixel( int x, int y )
{
	if ( !open() )
	{
		return 0;
	}
	
	Scanline::Ptr scanline = m_cache->get( y );
	if ( !scanline )
	{
		return 0;
	}
	
	size_t xOffset = x - dataWindow().min.x;
	
	int numSamples = scanline->sampleCount[xOffset];
	if ( !numSamples )
	{
		return 0;
	}
	
	DeepPixelPtr pixel = new DeepPixel( m_channelNames, numSamples );
	
	int numChannels = pixel->numChannels();
	std::vector<float> channelData(numChannels);
	
	unsigned cIndex = 0;
	float depth = 0;
	char *basePtr = reinterpret_cast< char * >( scanline->pointers[xOffset] );
	
	for ( int i=0; i < numSamples; ++i )
	{
		size_t offset = 0;
		for ( int c=0; c < numChannels + 1; ++c )
		{
			size_t channelSize = pixelTypeSize( m_channelTypes[c] );
			
			char *ptr = basePtr + offset + i * channelSize;
			if ( c == m_depthChannel )
			{
				depth = *reinterpret_cast< float * >( ptr );
			}
			else
			{
				cIndex = c > m_depthChannel ? c - 1 : c;

				if ( m_channelTypes[c] == Imf::FLOAT )
				{
					channelData[cIndex] = *reinterpret_cast< float * >( ptr );
				}
				else
				{
					channelData[cIndex] = static_cast< float >( *reinterpret_cast< half * >( ptr ) );
				}
			}
			offset += channelSize * numSamples;
		}
		
		pixel->addSample( depth, &channelData[0] );
	}
	
	return pixel;
}

EXRDeepImageReader::Scanline::Scanline( size_t width, size_t numChannels )
	: sampleCount( width ), pointers( width * numChannels ), data()
{
}

struct EXRDeepImageReader::Getter
{
	Getter( Imf::DeepScanLineInputFile *inputFile, size_t numChannels )
	: m_inputFile( inputFile ), m_numChannels( numChannels ), m_sampleSizeInBytes( 0 )
	{
		const Imf::ChannelList &channels = m_inputFile->header().channels();
		m_channelTypes.reserve( numChannels );
		for ( Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it )
		{
			Imf::PixelType type = it.channel().type;
			m_channelTypes.push_back( type );
			m_sampleSizeInBytes += pixelTypeSize( type );
		}
	}
	
	Imf::DeepScanLineInputFile *m_inputFile;
	size_t m_numChannels;
	size_t m_sampleSizeInBytes;
	std::vector<int> m_channelTypes;
	
	Scanline::Ptr operator()( const int &y, size_t &cost )
	{
		cost = 1;
		
		Imath::Box2i dataWindow = m_inputFile->header().dataWindow();
		size_t width = dataWindow.max.x - dataWindow.min.x + 1;
		
		const Imf::ChannelList &channels = m_inputFile->header().channels();
		
		Scanline::Ptr result = new Scanline( width, m_numChannels );
		
		Imf::DeepFrameBuffer frameBuffer;
		frameBuffer.insertSampleCountSlice(
			Imf::Slice(
				Imf::UINT, reinterpret_cast< char * >( &result->sampleCount[0] - dataWindow.min.x - y * width ),
				sizeof( unsigned ), sizeof( unsigned ) * width
			)
		);
	
		unsigned c = 0;
		for ( Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it, ++c )
		{
			Imf::DeepSlice slice(
					it.channel().type, reinterpret_cast< char * >( &result->pointers[ width * c ] - dataWindow.min.x - y * width ),
					sizeof( char * ), sizeof( char * ) * width, pixelTypeSize( it.channel().type )
					);
			frameBuffer.insert( it.name(), slice );
		}

		m_inputFile->setFrameBuffer( frameBuffer );

		m_inputFile->readPixelSampleCounts( y );

		size_t totalSamples = 0;
		for ( size_t i=0; i < width; ++i )
		{
			totalSamples += result->sampleCount[i];
		}
		
		result->data.resize( totalSamples * m_sampleSizeInBytes );
		
		if ( !totalSamples )
		{
			return result;
		}
		
		size_t current = 0;
		for ( size_t i=0; i < width; ++i )
		{
			size_t numSamples = result->sampleCount[i];
			
			for ( size_t c=0; c < m_numChannels; ++c )
			{
				result->pointers[ width * c + i ] = &result->data[ current ];
				current += numSamples * pixelTypeSize( Imf::PixelType( m_channelTypes[c] ) );
			}
		}
		
		m_inputFile->readPixels( y );
		return result;
	}

};

bool EXRDeepImageReader::open( bool throwOnFailure )
{
	if ( m_inputFile && fileName() == m_inputFile->fileName() )
	{
		// We already opened the right file successfully.
		return true;
	}
	
	delete m_cache;
	delete m_inputFile;
	m_inputFile = 0;
	m_cache = 0;
	m_channelNames.clear();
	m_channelTypes.clear();
	m_depthChannel = -1;
		
	try
	{
		m_inputFile = new Imf::DeepScanLineInputFile( fileName().c_str() );
	
		int i = 0;
		const Imf::ChannelList &channels = m_inputFile->header().channels();
		for ( Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it, ++i )
		{
			if ( it.channel().type != Imf::FLOAT && it.channel().type != Imf::HALF )
			{
				throw IOException( "Channel" + std::string(it.name()) + " is not a float or half channel." );
			}
			
			if ( strcmp( it.name(), "Z" ) )
			{
				m_channelNames.push_back( it.name() );
			}
			else
			{
				m_depthChannel = i;
			}

			m_channelTypes.push_back( it.channel().type );
		}
		
		if ( m_depthChannel == -1 )
		{
			throw IOException( "No depth channel" );
		}
		
		// We're assuming each scanline has a cost of 1, so 64 allows us to access
		// a decent sized tile without re-reading any data.
		m_cache = new Cache( Getter( m_inputFile, m_channelNames.size() + 1 ), 64 );
	}
	catch( ... )
	{
		delete m_cache;
		delete m_inputFile;
		m_inputFile = 0;
		m_cache = 0;
		m_channelNames.clear();
		m_channelTypes.clear();
		m_depthChannel = -1;
		
		if ( !throwOnFailure )
		{
			return false;
		}
		else
		{
			throw IOException( std::string( "Failed to open file \"" ) + fileName() + "\"" );
		}
	}

	return true;
}

