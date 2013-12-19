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

#include "boost/format.hpp"

#include "UT/UT_Matrix.h"
#include "UT/UT_Options.h"
#include "UT/UT_Version.h"

#include "IECore/FileNameParameter.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/RATDeepImageReader.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( RATDeepImageReader );

const Reader::ReaderDescription<RATDeepImageReader> RATDeepImageReader::g_readerDescription( "rat" );

RATDeepImageReader::RATDeepImageReader()
	:	DeepImageReader( "Reads Houdini RAT deep texture file format." ),
		m_inputFile( 0 ), m_ratPixel( 0 ), m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
}

RATDeepImageReader::RATDeepImageReader( const std::string &fileName )
	:	DeepImageReader( "Reads Houdini RAT deep texture file format." ),
		m_inputFile( 0 ), m_ratPixel( 0 ), m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
	m_fileNameParameter->setTypedValue( fileName );
}

RATDeepImageReader::~RATDeepImageReader()
{
	if ( m_inputFile )
	{
		m_inputFile->close();
	}
	
	delete m_ratPixel;
	delete m_inputFile;
}

bool RATDeepImageReader::canRead( const std::string &fileName )
{
	IMG_DeepShadow file;

#if UT_MAJOR_VERSION_INT >= 13

	if ( file.open( fileName.c_str() ) && file.getDepthInterp() == IMG_DI_DISCRETE )

#else

	if ( file.open( fileName.c_str() ) && file.depthInterp() == IMG_COMPRESS_DISCRETE )

#endif
	{
		file.close();
		return true;
	}
	
	return false;
}

void RATDeepImageReader::channelNames( std::vector<std::string> &names )
{
	open( true );
	
	unsigned numChannels = m_channelNames.size();
	names.resize( numChannels );
	for ( unsigned i=0; i < numChannels; ++i )
	{
		names[i] = m_channelNames[i];
	}
}

bool RATDeepImageReader::isComplete()
{
	return open();
}

Imath::Box2i RATDeepImageReader::dataWindow()
{
	open( true );
	
	return m_dataWindow;
}

Imath::Box2i RATDeepImageReader::displayWindow()
{
	return dataWindow();
}

Imath::M44f RATDeepImageReader::worldToCameraMatrix()
{
	open( true );
	
	return m_worldToCamera;
}

Imath::M44f RATDeepImageReader::worldToNDCMatrix()
{
	open( true );
	
	return	m_worldToNDC;
}

DeepPixelPtr RATDeepImageReader::doReadPixel( int x, int y )
{
	if ( !open() )
	{
		return 0;
	}
	
	y = m_dataWindow.max.y - y;
	
	if ( !m_ratPixel->open( x, y ) )
	{
		return 0;
	}
	
	int numSamples = m_ratPixel->getDepth();
	if ( !numSamples )
	{
		return 0;
	}
	
	DeepPixelPtr pixel = new DeepPixel( m_channelNames, numSamples );
	
	m_ratPixel->uncomposite( *m_depthChannel, *m_opacityChannel );
	
	for ( int i=0; i < numSamples; ++i )
	{
		pixel->addSample(
			*m_ratPixel->getData( *m_depthChannel, i ),
			m_ratPixel->getData( *m_colorChannel, i )
		);
	}
	
	m_ratPixel->close();
	
	return pixel;
}

bool RATDeepImageReader::open( bool throwOnFailure )
{
	if ( m_inputFile && fileName() == m_inputFileName )
	{
		// we already opened the right file successfully
		return true;
	}
	
	delete m_ratPixel;
	delete m_inputFile;
	m_inputFile = new IMG_DeepShadow;
	m_inputFileName = "";
	m_channelNames = "";
	m_ratPixel = 0;
	m_depthChannel = 0;
	m_opacityChannel = 0;
	m_colorChannel = 0;
	m_dataWindow.max.x = 0;
	m_dataWindow.max.y = 0;
	m_worldToCamera = Imath::M44f();
	m_worldToNDC = Imath::M44f();
	
	bool success = true;
	
#if UT_MAJOR_VERSION_INT >= 13

	if ( m_inputFile->open( fileName().c_str() ) && m_inputFile->getDepthInterp() == IMG_DI_DISCRETE )

#else

	if ( m_inputFile->open( fileName().c_str() ) && m_inputFile->depthInterp() == IMG_COMPRESS_DISCRETE )

#endif
	{
		m_inputFileName = fileName();
		m_ratPixel = new IMG_DeepPixelReader( *m_inputFile );
		
		const IMG_DeepShadowChannel *channel = 0;
		unsigned numChannels = m_inputFile->getChannelCount();
		for ( unsigned c=0; c < numChannels; ++c )
		{
			channel = m_inputFile->getChannel( c );
			std::string name = channel->getName();

			if ( name == "Pz" )
			{
				m_depthChannel = channel;
			}
			else if ( name == "Of" )
			{
				m_opacityChannel = channel;
			}
			else if ( name == "C" )
			{
				m_colorChannel = channel;
			}
		}

		if ( m_colorChannel )
		{
			m_channelNames = "RGBA";
		}
		else
		{
			m_colorChannel = m_opacityChannel;
			m_channelNames = "A";
		}
		
		if ( !m_depthChannel || !m_opacityChannel || !m_colorChannel )
		{
			success = false;
		}
		
		m_inputFile->resolution( m_dataWindow.max.x, m_dataWindow.max.y );
		m_dataWindow.max.x -= 1;
		m_dataWindow.max.y -= 1;
		
		UT_Matrix4 wToC( 1 );
		m_inputFile->getWorldToCamera( wToC );
		m_worldToCamera = IECore::convert<Imath::M44f>( wToC );
		
		UT_Matrix4 wToNDC( 1 );
		m_inputFile->getWorldToNDC( wToNDC, true );
		
		// wToNDC has flipped values and a scaling issue related to the far clipping plane
		// applying a matrix that seems to fix the issues in several examples
		Imath::M44f fix;
		fix.makeIdentity();
		
#if UT_MAJOR_VERSION_INT >= 13

		UT_SharedPtr<UT_Options> options = m_inputFile->getTextureOptions();

#else

		UT_Options *options = m_inputFile->getTBFOptions();

#endif

		if ( options->hasOption( "camera:clip" ) )
		{
			const UT_Vector2D clip = options->getOptionV2( "camera:clip" );
			fix[2][2] = clip[1];
			fix[3][3] = -1;
		}
		
		m_worldToNDC = IECore::convert<Imath::M44f>( wToNDC ) * fix;
	}
	else
	{
		success = false;
	}
	
	if ( !success )
	{
		delete m_ratPixel;
		delete m_inputFile;
		m_inputFile = 0;
		m_inputFileName = "";
		m_channelNames = "";
		m_ratPixel = 0;
		m_depthChannel = 0;
		m_opacityChannel = 0;
		m_colorChannel = 0;
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
