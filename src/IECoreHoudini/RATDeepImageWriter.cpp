//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/RATDeepImageWriter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( RATDeepImageWriter );

const DeepImageWriter::DeepImageWriterDescription<RATDeepImageWriter> RATDeepImageWriter::g_writerDescription( "rat" );

RATDeepImageWriter::RATDeepImageWriter()
	: DeepImageWriter( "Writes Houdini RAT deep texture file format." ), m_outputFile( 0 )
{
}

RATDeepImageWriter::RATDeepImageWriter( const std::string &fileName )
	: DeepImageWriter( "Writes Houdini RAT deep texture file format." ), m_outputFile( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
}

RATDeepImageWriter::~RATDeepImageWriter()
{
	if ( m_outputFile )
	{
		m_outputFile->close();
	}
	
	delete m_outputFile;
}

bool RATDeepImageWriter::canWrite( const std::string &fileName )
{
	return IMG_DeepShadow().open( fileName.c_str(), 2, 2 );
}

void RATDeepImageWriter::doWritePixel( int x, int y, const DeepPixel *pixel )
{
	open();
	
	const float *channelData = 0;
	
	float alpha = 1.0;
	float *adjustedData = new float[m_dataSize];
	for ( unsigned c=0; c < 3; ++c )
	{
		adjustedData[c] = alpha;
	}
	
	bool alphaExists = ( m_alphaOffset >= 0 );
	bool extraChannels = ( m_extraOffset >= 0 );
	
	y = m_resolutionParameter->getTypedValue().y - y - 1;
	
	m_outputFile->pixelStart( x, y );
	
	unsigned numChannels = pixel->numChannels();
	unsigned numSamples = pixel->numSamples();
	for ( unsigned i=0; i < numSamples; ++i )
	{
		channelData = pixel->channelData( i );
		
		if ( alphaExists )
		{
			alpha = channelData[m_alphaOffset];
			for ( unsigned c=0; c < 3; ++c )
			{
				adjustedData[c] = alpha;
			}
		}
		
		if ( extraChannels )
		{
			for ( unsigned c=0; c < numChannels; ++c )
			{
				adjustedData[c+3] = channelData[ m_extraOffset + c ];
			}
		}
		
		m_outputFile->pixelWriteOrdered( pixel->getDepth( i ), adjustedData, m_dataSize );
	}
	
	m_outputFile->pixelClose();
	delete [] adjustedData;
}

void RATDeepImageWriter::open()
{
	if ( m_outputFile && fileName() == m_outputFileName )
	{
		// we already opened the right file successfully
		return;
	}

	delete m_outputFile;
	m_outputFile = new IMG_DeepShadow;
	m_outputFileName = "";
	m_dataSize = 3;
	m_alphaOffset = -1;
	m_extraOffset = -1;
	
	const std::vector<std::string> &channelNames = m_channelsParameter->getTypedValue();
	
	// use A if it exists, otherwise assume opaque
	std::vector<std::string>::const_iterator aIt = std::find( channelNames.begin(), channelNames.end(), "A" );
	if ( aIt != channelNames.end() )
	{
		m_alphaOffset = aIt - channelNames.begin();
	}
	
	// add the extra channels, including color
	int numChannels = channelNames.size();
	for ( int i=0; i < numChannels; ++i )
	{
		std::string name = channelNames[i];
		
		// must have RGBA in order to have color
		if ( ( name == "R" ) && ( i < numChannels - 3 ) && ( channelNames[i+1] == "G" ) && ( channelNames[i+2] == "B" ) && ( m_alphaOffset == i + 3 ) )
		{
			// adding 3 to the actual offset since the opacity triple must come first
			m_outputFile->addExtraChannel( IMG_DeepShadowChannel( "C", i+3, 4 ) );
			m_dataSize += 4;
			
			if ( m_extraOffset < 0 )
			{
				m_extraOffset = i;
			}
		}
		else if ( name != "G" && name != "B" && name != "A" )
		{
			// adding 3 to the actual offset since the opacity triple must come first
			m_outputFile->addExtraChannel( IMG_DeepShadowChannel( name.c_str(), i+3, 1 ) );
			
			if ( m_extraOffset < 0 )
			{
				m_extraOffset = i;
			}

			m_dataSize++;
		}
	}
	
	m_outputFile->setOption( "compositing", "0" );
	m_outputFile->setOption( "depth_interp", "discrete" );
	
	const Imath::V2i &resolution = m_resolutionParameter->getTypedValue();
	
	if ( m_outputFile->open( fileName().c_str(), resolution.x, resolution.y ) )
	{
		m_outputFileName = fileName();
		
		// set the worldToCamera matrix
		UT_Options *options = m_outputFile->getTBFOptions();
		options->setOptionM4( "space:world", IECore::convert<UT_Matrix4>( worldToCameraParameter()->getTypedValue() ) );
		
		/// \todo: set the cameraToNDC parameters
	}
	else
	{
		delete m_outputFile;
		m_outputFile = 0;
		m_outputFileName = "";
		m_dataSize = 3;
		m_alphaOffset = -1;
		m_extraOffset = -1;
		
		throw IOException( std::string( "Failed to open file \"" ) + fileName() + "\" for writing." );
	}
}
