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

#include "math.h"

#include "IECore/EXRImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/MessageHandler.h"

#include "boost/format.hpp"

#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/Iex.h"
#include "OpenEXR/ImfTestFile.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace Imf;
using namespace std;

using Imath::Box2i;

IE_CORE_DEFINERUNTIMETYPED( EXRImageReader );

const Reader::ReaderDescription<EXRImageReader> EXRImageReader::g_readerDescription("exr");

EXRImageReader::EXRImageReader() :
		ImageReader( "EXRImageReader", "Reads ILM OpenEXR file format." ),
		m_inputFile( 0 )
{
}

EXRImageReader::EXRImageReader(const string &fileName) :
		ImageReader( "EXRImageReader", "Reads ILM OpenEXR file format." ),
		m_inputFile( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
}

EXRImageReader::~EXRImageReader()
{
	delete m_inputFile;
}

bool EXRImageReader::canRead( const string &fileName )
{
	return isOpenExrFile( fileName.c_str() );
}

void EXRImageReader::channelNames( vector<string> &names )
{
	open( true );

	const ChannelList &channels = m_inputFile->header().channels();
	names.clear();
	for( ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i )
	{
		names.push_back( i.name() );
	}
}

bool EXRImageReader::isComplete()
{
	if( !open() )
	{
		return false;
	}
	return m_inputFile->isComplete();
}

Imath::Box2i EXRImageReader::dataWindow()
{
	open( true );
	return m_inputFile->header().dataWindow();
}

Imath::Box2i EXRImageReader::displayWindow()
{
	open( true );
	return m_inputFile->header().displayWindow();
}

std::string EXRImageReader::defaultColorSpace() const
{
	return "linear";
}

template<class T>
DataPtr EXRImageReader::readTypedChannel( const std::string &name, const Imath::Box2i &dataWindow, const Imf::Channel *channel )
{
	assert( channel );
	Imath::V2i pixelDimensions = dataWindow.size() + Imath::V2i( 1 );
	unsigned numPixels = pixelDimensions.x * pixelDimensions.y;

	typedef TypedData<vector<T> > DataType;
	typename DataType::Ptr data = new DataType;
	data->writable().resize( numPixels );
	
	Imath::Box2i fullDataWindow = this->dataWindow();
	if( fullDataWindow.min.x==dataWindow.min.x && fullDataWindow.max.x==dataWindow.max.x )
	{
		// the width we want to read matches the width in the file, so we can read straight
		// into the result buffer
		FrameBuffer frameBuffer;
		T *buffer00 = data->baseWritable() - dataWindow.min.y * pixelDimensions.x - fullDataWindow.min.x;
		Slice slice( channel->type, (char *)buffer00, sizeof(T), sizeof(T) * pixelDimensions.x );
		frameBuffer.insert( name.c_str(), slice );
		m_inputFile->setFrameBuffer( frameBuffer );
		// exr library will choose the best order to read scanlines automatically (increasing or decreasing)
		try
		{
			m_inputFile->readPixels( dataWindow.min.y, dataWindow.max.y );
		}
		catch( Iex::InputExc &e )
		{
			// so we can read incomplete files
			msg( Msg::Warning, "EXRImageReader::readChannel", e.what() );
			return data;
		}
	}
	else
	{
		// widths don't match, we need to read into a temporary buffer and then transfer just
		// the bits we need into the result buffer.
		int numTmpPixels = fullDataWindow.size().x + 1;
		vector<T> tmpBuffer( numTmpPixels );
		T *tmpBufferTransferStart = &(tmpBuffer[0]) + dataWindow.min.x - fullDataWindow.min.x;
		size_t tmpBufferTransferLength = pixelDimensions.x * sizeof( T );
		T *transferDestination = &(data->writable()[0]);
		
		// slice has yStride of 0 so each successive scanline just overwrites the previous one		
		Slice slice( channel->type, (char *)(&(tmpBuffer[0]) - fullDataWindow.min.x), sizeof(T), 0 );
		FrameBuffer frameBuffer;
		frameBuffer.insert( name.c_str(), slice );
		m_inputFile->setFrameBuffer( frameBuffer );
		
		int yStart = dataWindow.min.y;
		int yEnd = dataWindow.max.y;
		int yStep = 1;
		try
		{
			for( int y=yStart; y!=(yEnd+yStep); y+=yStep )
			{
				m_inputFile->readPixels( y );
				memcpy( (char *)transferDestination, (const char *)tmpBufferTransferStart, tmpBufferTransferLength );
				transferDestination += pixelDimensions.x;
			}
		}
		catch( Iex::InputExc &e )
		{
			// so we can read incomplete files
			msg( Msg::Warning, "EXRImageReader::readChannel", e.what() );
			return data;
		}	
	}
	
#ifndef NDEBUG
	for ( typename DataType::ValueType::const_iterator it = data->readable().begin(); it != data->readable().end(); ++it )
	{
		assert( *it == *it ); // Will fail iff NaN
	}
#endif	
	
	return data;
}

DataPtr EXRImageReader::readChannel( const string &name, const Imath::Box2i &dataWindow )
{
	open( true );
	
	try
	{
		
		const Channel *channel = m_inputFile->header().channels().findChannel( name.c_str() );
		assert( channel );
		assert( channel->xSampling==1 ); /// \todo Support subsampling when we have a need for it
		assert( channel->ySampling==1 );
			
		switch( channel->type )
		{
	
			case UINT :
				BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
				return readTypedChannel<unsigned int>( name, dataWindow, channel );	

			case HALF :
				return readTypedChannel<half>( name, dataWindow, channel );	

			case FLOAT :
				BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
				return readTypedChannel<float>( name, dataWindow, channel );	

			default:
				throw IOException( ( boost::format( "EXRImageReader : Unsupported data type for channel \"%s\"" ) % name ).str() );
		
		}
		
	} 
	catch ( Exception &e )
	{
		throw;
	}
	catch( Iex::BaseExc &e )
	{
		std::string s = ( boost::format( "EXRImageReader : %s" ) % e.what() ).str();
		e.assign( s.c_str() );
		throw e;
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format( "EXRImageReader : %s" ) % e.what() ).str() );
	}
	catch ( ... )
	{
		throw IOException( "EXRImageReader : Unexpected error" );
	}
}

bool EXRImageReader::open( bool throwOnFailure )
{
	if( m_inputFile && fileName()==m_inputFile->fileName() )
	{
		// we already opened the right file successfully
		return true;
	}

	delete m_inputFile;
	m_inputFile = 0;
		
	try
	{
		m_inputFile = new Imf::InputFile( fileName().c_str() );
	}
	catch( ... )
	{
		delete m_inputFile;
		m_inputFile = 0;
		if( !throwOnFailure )
		{
			return false;
		}
		else
		{
			throw IOException( string( "Failed to open file \"" ) + fileName() + "\"" );
		}
	}	

	return true;
}
