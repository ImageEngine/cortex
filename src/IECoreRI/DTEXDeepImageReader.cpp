//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/FileNameParameter.h"

#include "IECoreRI/DTEXDeepImageReader.h"

using namespace IECore;
using namespace IECoreRI;

IE_CORE_DEFINERUNTIMETYPED( DTEXDeepImageReader );

const Reader::ReaderDescription<DTEXDeepImageReader> DTEXDeepImageReader::g_readerDescription( "dtex" );

DTEXDeepImageReader::DTEXDeepImageReader()
	:	DeepImageReader( "Reads PRMan DTEX deep texture file format." ),
		m_inputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ),
		m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
}

DTEXDeepImageReader::DTEXDeepImageReader( const std::string &fileName )
	:	DeepImageReader( "Reads PRMan DTEX deep texture file format." ),
		m_inputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ),
		m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
	m_fileNameParameter->setTypedValue( fileName );
}

DTEXDeepImageReader::~DTEXDeepImageReader()
{
	cleanRixInterface();
}

bool DTEXDeepImageReader::canRead( const std::string &fileName )
{
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	RixDeepTexture::DeepCache *dtexCache = dtexInterface->CreateCache( 1 );
	RixDeepTexture::DeepFile *dtexFile = 0;
	
	int status = dtexInterface->OpenFile( fileName.c_str(), "rb", dtexCache, &dtexFile );
	
	if ( dtexFile )
	{
		dtexFile->Close();
		dtexInterface->DestroyFile( dtexFile );
	}
	
	dtexInterface->DestroyCache( dtexCache );
	
	return ( status == RixDeepTexture::k_ErrNOERR );
}

void DTEXDeepImageReader::channelNames( std::vector<std::string> &names )
{
	open( true );
	
	unsigned numChannels = m_channelNames.size();
	names.resize( numChannels );
	for ( unsigned i=0; i < numChannels; ++i )
	{
		names[i] = m_channelNames[i];
	}
}

bool DTEXDeepImageReader::isComplete()
{
	return open();
}

Imath::Box2i DTEXDeepImageReader::dataWindow()
{
	open( true );
	
	return m_dataWindow;
}

Imath::Box2i DTEXDeepImageReader::displayWindow()
{
	return dataWindow();
}

Imath::M44f DTEXDeepImageReader::worldToCameraMatrix()
{
	open( true );
	
	return m_worldToCamera;
}

Imath::M44f DTEXDeepImageReader::worldToNDCMatrix()
{
	open( true );
	
	return  m_worldToNDC;
}

DeepPixelPtr DTEXDeepImageReader::doReadPixel( int x, int y )
{
	if ( !open() )
	{
		return 0;
	}
	
	if ( m_dtexImage->GetPixel( x, y, m_dtexPixel ) != RixDeepTexture::k_ErrNOERR )
	{
		return 0;
	}
	
	int numSamples = m_dtexPixel->GetNumPoints();
	if ( !numSamples )
	{
		return 0;
	}
	
	DeepPixelPtr pixel = new DeepPixel( m_channelNames, numSamples );
	
	float depth = 0;
	float channelData[pixel->numChannels()];
	
	for ( int i=0; i < numSamples; ++i )
	{
		m_dtexPixel->GetPoint( i, &depth, channelData );
		pixel->addSample( depth, channelData );
	}
	
	return pixel;
}

bool DTEXDeepImageReader::open( bool throwOnFailure )
{
	if ( m_inputFile && fileName() == m_inputFileName )
	{
		// we already opened the right file successfully
		return true;
	}
	
	cleanRixInterface();
	m_inputFileName = "";
	m_channelNames = "";
	m_dataWindow.max.x = 0;
	m_dataWindow.max.y = 0;
	m_worldToCamera = Imath::M44f();
	m_worldToNDC = Imath::M44f();
	
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	/// \todo: what should numTiles be? we don't know the resolution yet because we haven't opened the file...
	m_dtexCache = dtexInterface->CreateCache( 10000 );
	
	if ( dtexInterface->OpenFile( fileName().c_str(), "rb", m_dtexCache, &m_inputFile ) == RixDeepTexture::k_ErrNOERR )
	{
		m_inputFileName = fileName();
		
		m_inputFile->GetImageByIndex( 0, &m_dtexImage );
		
		int numChannels = m_dtexImage->GetNumChan();
		std::string imageExtension = boost::filesystem::extension( boost::filesystem::path( m_dtexImage->GetName() ) );
		m_channelNames = boost::to_upper_copy( imageExtension.substr( 1, numChannels + 1 ) );
		m_dtexPixel = dtexInterface->CreatePixel( numChannels );
		
		m_dataWindow.max.x = m_dtexImage->GetWidth() - 1;
		m_dataWindow.max.y = m_dtexImage->GetHeight() - 1;
		
		m_dtexImage->GetNl( m_worldToCamera.getValue() );
		m_dtexImage->GetNP( m_worldToNDC.getValue() );
	}
	else
	{
		cleanRixInterface();
		m_inputFileName = "";
		m_channelNames = "";
		m_dataWindow.max.x = 0;
		m_dataWindow.max.y = 0;
		m_worldToCamera = Imath::M44f();
		m_worldToNDC = Imath::M44f();
		
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

void DTEXDeepImageReader::cleanRixInterface()
{
	RixDeepTexture *dtexInterface = (RixDeepTexture *)RixGetContext()->GetRixInterface( k_RixDeepTexture );
	
	if ( m_dtexPixel )
	{
		dtexInterface->DestroyPixel( m_dtexPixel );
	}

	if ( m_inputFile )
	{
		m_inputFile->Close();
		dtexInterface->DestroyFile( m_inputFile );
	}

	if ( m_dtexCache )
	{
		dtexInterface->DestroyCache( m_dtexCache );
	}

	m_inputFile = 0;
	m_dtexCache = 0;
	m_dtexImage = 0;
	m_dtexPixel = 0;
}
