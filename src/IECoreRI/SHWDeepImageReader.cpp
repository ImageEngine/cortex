//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/SHWDeepImageReader.h"

using namespace IECore;
using namespace IECoreRI;

IE_CORE_DEFINERUNTIMETYPED( SHWDeepImageReader );

const Reader::ReaderDescription<SHWDeepImageReader> SHWDeepImageReader::g_readerDescription( "shw" );

SHWDeepImageReader::SHWDeepImageReader()
	:	DeepImageReader( "Reads 3delight SHW deep shadow file format." ),
		m_inputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ),
		m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
}

SHWDeepImageReader::SHWDeepImageReader( const std::string &fileName )
	:	DeepImageReader( "Reads 3delight SHW deep shadow file format." ),
		m_inputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ),
		m_dataWindow( Imath::V2i( 0 ), Imath::V2i( 0 ) )
{
	m_fileNameParameter->setTypedValue( fileName );
}

SHWDeepImageReader::~SHWDeepImageReader()
{
	clean();
}

bool SHWDeepImageReader::canRead( const std::string &fileName )
{
	DtexCache *dtexCache = DtexCreateCache( 1, NULL );
	DtexFile *dtexFile = 0;
	
	int status = DtexOpenFile( fileName.c_str(), "rb", dtexCache, &dtexFile );
	
	/// \todo: DtexClose seems to cause fatal errors, so I'm ignoring my belief that it's needed...
	
	DtexDestroyCache( dtexCache );
	
	return ( status == DTEX_NOERR );
}

void SHWDeepImageReader::channelNames( std::vector<std::string> &names )
{
	open( true );
	
	unsigned numChannels = m_channelNames.size();
	names.resize( numChannels );
	for ( unsigned i=0; i < numChannels; ++i )
	{
		names[i] = m_channelNames[i];
	}
}

bool SHWDeepImageReader::isComplete()
{
	return open();
}

Imath::Box2i SHWDeepImageReader::dataWindow()
{
	open( true );
	
	return m_dataWindow;
}

Imath::Box2i SHWDeepImageReader::displayWindow()
{
	return dataWindow();
}

Imath::M44f SHWDeepImageReader::worldToCameraMatrix()
{
	open( true );
	
	return m_worldToCamera;
}

Imath::M44f SHWDeepImageReader::worldToNDCMatrix()
{
	open( true );
	
	return  m_worldToNDC;
}

DeepPixelPtr SHWDeepImageReader::doReadPixel( int x, int y )
{
	if ( !open() )
	{
		return 0;
	}
	
	if ( DtexGetPixel( m_dtexImage, x, y, m_dtexPixel ) != DTEX_NOERR )
	{
		return 0;
	}
	
	int numSamples = DtexPixelGetNumPoints( m_dtexPixel );
	if ( !numSamples )
	{
		return 0;
	}
	
	DeepPixelPtr pixel = new DeepPixel( m_channelNames, numSamples );
	
	unsigned numRealChannels = DtexNumChan( m_dtexImage );
	
	float depth = 0;
#ifdef _MSC_VER
	float* channelData = new float[numRealChannels];
	float* previous = new float[numRealChannels];
#else
	float channelData[numRealChannels];
	float previous[numRealChannels];
#endif
	for ( unsigned j=0; j < numRealChannels; ++j )
	{
		previous[j] = 0.0;
	}

	float nearClip = m_NDCToCamera[3][2] / m_NDCToCamera[3][3];
	float correction = 1;
	if( m_NDCToCamera[3][2] != 0 && m_NDCToCamera[2][3] != 0 )
	{
		// Compute a correction factor that converts from spherical distance to perpendicular distance,
		// by comparing the closest distance to the near clip with the distance to the near clip at the current pixel position
		correction = nearClip / ( Imath::V3f(((x+0.5f)/(m_dataWindow.max.x+1) * 2 - 1), -((y+0.5)/(m_dataWindow.max.y+1) * 2 - 1),0) * m_NDCToCamera ).length();
	}

	for ( int i=0; i < numSamples; ++i )
	{
		DtexPixelGetPoint( m_dtexPixel, i, &depth, channelData );

		// Convert from "3delight distance" ( spherical distance from near clip ) to Z ( distance from eye plane )
		depth = depth * correction + nearClip;
		
		for ( unsigned j=0; j < numRealChannels; ++j )
		{
			// SHW files represent occlusion, but we really want transparency,
			// so we invert the data upon reading it.
			/// \todo: consider a parameter to opt out of this behaviour
			float current = 1.0 - channelData[j];
			
			// SHW files store composited values, accumulated over depth, but we want uncomposited values
			channelData[j] = ( previous[j] == 1.0 ) ? 1.0 : ( current - previous[j] ) / ( 1 - previous[j] );
			previous[j] = current;
		}
		
		pixel->addSample( depth, channelData );
	}
#ifdef _MSC_VER
	delete[] channelData;
	delete[] previous;
#endif
	
	return pixel;
}

bool SHWDeepImageReader::open( bool throwOnFailure )
{
	if ( m_inputFile && fileName() == m_inputFileName )
	{
		// we already opened the right file successfully
		return true;
	}
	
	m_inputFileName = "";
	m_channelNames = "";
	m_dataWindow.max.x = 0;
	m_dataWindow.max.y = 0;
	m_worldToCamera = Imath::M44f();
	m_worldToNDC = Imath::M44f();
	m_NDCToCamera = Imath::M44f();
	
	clean();
	
	m_dtexCache = DtexCreateCache( 10000, NULL );
	
	if ( DtexOpenFile( fileName().c_str(), "rb", m_dtexCache, &m_inputFile ) == DTEX_NOERR )
	{
		m_inputFileName = fileName();
		
		DtexGetImageByIndex( m_inputFile, 0, &m_dtexImage );

		unsigned numChannels = DtexNumChan( m_dtexImage );
		
		// Since these are monochrome deep shadows, we know that regardless
		// of numChannels, this is really just an Alpha value.
		m_channelNames = "A";
		
		m_dtexPixel = DtexMakePixel( numChannels );
		
		m_dataWindow.max.x = DtexWidth( m_dtexImage ) - 1;
		m_dataWindow.max.y = DtexHeight( m_dtexImage ) - 1;
		
		DtexNl( m_dtexImage, m_worldToCamera.getValue() );
		DtexNP( m_dtexImage, m_worldToNDC.getValue() );

		// TODO - there should probably be a better way to perform this simple operation in double precision,
		// but right now I really just need this to start working, so I'm doing the long version
		// ( Plus we can hopefully get rid of this whole thing fairly soon, when 3delight fixes thier depth
		// mapping to start with, and/or we throw this out and move to deep exr )
		//m_NDCToCamera = m_worldToNDC.inverse() * m_worldToCamera;
		
		Imath::M44d worldToCameraDouble;
		Imath::M44d worldToNDCDouble;
		for( int ix = 0; ix < 4; ix++ )
		{
			for( int iy = 0; iy < 4; iy++ )
			{
				worldToCameraDouble[ix][iy] = m_worldToCamera[ix][iy];
				worldToNDCDouble[ix][iy] = m_worldToNDC[ix][iy];
			}
		}
		Imath::M44d NDCToCameraDouble = worldToNDCDouble.inverse() * worldToCameraDouble;
		for( int ix = 0; ix < 4; ix++ )
		{
			for( int iy = 0; iy < 4; iy++ )
			{
				m_NDCToCamera[ix][iy] = NDCToCameraDouble[ix][iy];
			}
		}
	}
	else
	{
		m_inputFileName = "";
		m_channelNames = "";
		m_dataWindow.max.x = 0;
		m_dataWindow.max.y = 0;
		m_worldToCamera = Imath::M44f();
		m_worldToNDC = Imath::M44f();
		clean();
		
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

void SHWDeepImageReader::clean()
{
	if ( m_dtexPixel )
	{
		DtexDestroyPixel( m_dtexPixel );
	}
	
	// DtexClose seems to cause fatal errors, unless a file image was opened
	if ( m_inputFile && m_inputFileName != "" )
	{
		DtexClose( m_inputFile );
	}
		
	if ( m_dtexCache )
	{
		DtexDestroyCache( m_dtexCache );
	}
	
	m_inputFile = 0;
	m_dtexCache = 0;
	m_dtexImage = 0;
	m_dtexPixel = 0;
}
