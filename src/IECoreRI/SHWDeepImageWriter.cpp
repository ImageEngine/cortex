//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2014, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "IECoreRI/SHWDeepImageWriter.h"

using namespace IECore;
using namespace IECoreRI;

#include "IECore/ClassData.h"

static IECore::ClassData< SHWDeepImageWriter, Imath::M44f > g_ndcClassData;

Imath::M44f &SHWDeepImageWriter::m_NDCToCamera()
{
	return g_ndcClassData[ this ];
}


IE_CORE_DEFINERUNTIMETYPED( SHWDeepImageWriter );

const DeepImageWriter::DeepImageWriterDescription<SHWDeepImageWriter> SHWDeepImageWriter::g_writerDescription( "shw" );

SHWDeepImageWriter::SHWDeepImageWriter()
	:	DeepImageWriter( "Writes 3delight SHW deep shadow file format." ),
		m_outputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ), m_alphaOffset( 0 )
{
	std::vector<std::string> channels( 1, "A" );
	m_channelsParameter->setValue( new StringVectorData( channels ) );
	
	m_tileSizeParameter = new V2iParameter( "tileSize", "The tile size for the image cache. Must be equal or less than resolution.", new V2iData( Imath::V2i( 32, 32 ) ) );
	parameters()->addParameter( m_tileSizeParameter );

	g_ndcClassData.create( this, Imath::M44f() );

}

SHWDeepImageWriter::SHWDeepImageWriter( const std::string &fileName )
	:	DeepImageWriter( "Writes 3delight SHW deep shadow file format." ),
		m_outputFile( 0 ), m_dtexCache( 0 ), m_dtexImage( 0 ), m_dtexPixel( 0 ), m_alphaOffset( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
	
	std::vector<std::string> channels( 1, "A" );
	m_channelsParameter->setValue( new StringVectorData( channels ) );
	
	m_tileSizeParameter = new V2iParameter( "tileSize", "The tile size for the image cache. Must be equal or less than resolution.", new V2iData( Imath::V2i( 32, 32 ) ) );
	parameters()->addParameter( m_tileSizeParameter );

	g_ndcClassData.create( this, Imath::M44f() );
}

SHWDeepImageWriter::~SHWDeepImageWriter()
{
	clean();
	g_ndcClassData.erase( this );

}

bool SHWDeepImageWriter::canWrite( const std::string &fileName )
{
	DtexCache *dtexCache = DtexCreateCache( 1, NULL );
	DtexFile *dtexFile = 0;
	
	int status = DTEX_NOFILE;
	
	std::string ext = boost::filesystem::extension( boost::filesystem::path( fileName ) );
	if ( ext == ".shw" )
	{
		status = DtexCreateFile( fileName.c_str(), dtexCache, &dtexFile );
	}
	
	/// \todo: DtexClose seems to cause fatal errors, so I'm ignoring my belief that it's needed...
	
	DtexDestroyCache( dtexCache );
	
	return ( status == DTEX_NOERR );
}

void SHWDeepImageWriter::doWritePixel( int x, int y, const DeepPixel *pixel )
{
	open();
	
	// ignoring Pixel->numChannels() as only an Opacity triple is accepted by this format
	int numChannels = 3;
	DtexClearPixel( m_dtexPixel, numChannels );
	
	float adjustedData[numChannels];
	
	float previous = 0.0;
	unsigned numSamples = pixel->numSamples();

	const Imath::V2i &resolution = m_resolutionParameter->getTypedValue();
	float nearClip = m_NDCToCamera()[3][2] / m_NDCToCamera()[3][3];
	float correction = 1;
	if( m_NDCToCamera()[3][2] != 0 && m_NDCToCamera()[2][3] != 0 )
	{
		// Compute a correction factor that converts from perpendicular distance to spherical distance,
		// by comparing the closest distance to the near clip with the distance to the near clip at the current pixel position
		correction = ( Imath::V3f(((x+0.5f)/resolution.x * 2 - 1), -((y+0.5)/resolution.y * 2 - 1),0) * m_NDCToCamera() ).length() / nearClip;
	}

	for ( unsigned i=0; i < numSamples; ++i )
	{
		// SHW files require composited values, accumulated over depth, but we have uncomposited values
		float current = pixel->channelData( i )[m_alphaOffset];
		float value = current * ( 1 - previous ) + previous;
		previous = value;
		
		// SHW files represent occlusion, but we really want transparency,
		// so we invert the data upon reading it.
		/// \todo: consider a parameter to opt out of this behaviour
		value = 1.0 - value;
		
		for ( unsigned c=0; c < 3; ++c )
		{
			adjustedData[c] = value;
		}
	
		float depth = pixel->getDepth( i );	

		// Convert from Z ( distance from eye plane ) to "3delight distance" ( spherical distance from near clip )
		depth = ( depth - nearClip ) * correction;

		DtexAppendPixel( m_dtexPixel, depth, numChannels, adjustedData, 0 );
	}
	
	DtexFinishPixel( m_dtexPixel );
	DtexSetPixel( m_dtexImage, x, y, m_dtexPixel );
}

void SHWDeepImageWriter::open()
{
	if ( m_outputFile && fileName() == m_outputFileName )
	{
		// we already opened the right file successfully
		return;
	}
	
	m_outputFileName = "";
	m_alphaOffset = 0;
	clean();
	
	const std::vector<std::string> &channelNames = m_channelsParameter->getTypedValue();
	
	// ignoring channelNames->size() as only an Opacity triple is accepted by this format
	unsigned numChannels = 3;
	
	// use A if it exists, otherwise use the first channel
	std::vector<std::string>::const_iterator aIt = std::find( channelNames.begin(), channelNames.end(), "A" );
	if ( aIt != channelNames.end() )
	{
		m_alphaOffset = aIt - channelNames.begin();
	}
	
	const Imath::V2i &resolution = m_resolutionParameter->getTypedValue();
	const Imath::V2i &tileSize = m_tileSizeParameter->getTypedValue();
	
	if ( tileSize.x > resolution.x || tileSize.y > resolution.y )
	{
		throw InvalidArgumentException( std::string( "Tile size must be equal to or less than resolution." ) );
	}
	
	if ( ( ( tileSize.x & ( tileSize.x - 1 ) ) != 0 ) || ( ( tileSize.y & ( tileSize.y - 1 ) ) != 0 ) )
	{
		throw InvalidArgumentException( std::string( "Tile width and height must be a power of two." ) );
	}
	
	m_dtexCache = DtexCreateCache( resolution.x / tileSize.x, NULL );
	
	if ( DtexCreateFile( fileName().c_str(), m_dtexCache, &m_outputFile ) != DTEX_NOERR )
	{
		m_outputFileName = "";
		m_alphaOffset = 0;
		clean();
		throw IOException( std::string( "Failed to open file \"" ) + fileName() + "\" for writing." );
	}
	
	m_outputFileName = fileName();
	
	float *NL = worldToCameraParameter()->getTypedValue().getValue();
	float *NP = worldToNDCParameter()->getTypedValue().getValue();
	// TODO - there should probably be a better way to perform this simple operation in double precision,
	// but right now I really just need this to start working, so I'm doing the long version
	// ( Plus we can hopefully get rid of this whole thing fairly soon, when 3delight fixes thier depth
	// mapping to start with, and/or we throw this out and move to deep exr )
	//m_NDCToCamera() = worldToNDCParameter()->getTypedValue().inverse() * worldToCameraParameter()->getTypedValue();
	Imath::M44d worldToCameraDouble;
	Imath::M44d worldToNDCDouble;
	for( int ix = 0; ix < 4; ix++ )
	{
		for( int iy = 0; iy < 4; iy++ )
		{
			worldToCameraDouble[ix][iy] = worldToCameraParameter()->getTypedValue()[ix][iy];
			worldToNDCDouble[ix][iy] = worldToNDCParameter()->getTypedValue()[ix][iy];
		}
	}
	Imath::M44d NDCToCameraDouble = worldToNDCDouble.inverse() * worldToCameraDouble;
	for( int ix = 0; ix < 4; ix++ )
	{
		for( int iy = 0; iy < 4; iy++ )
		{
			m_NDCToCamera()[ix][iy] = NDCToCameraDouble[ix][iy];
		}
	}

	
	/// \todo: does image name mean anything for this format?
	int status = DtexAddImage(
		m_outputFile, "", numChannels,
		resolution.x, resolution.y, tileSize.x, tileSize.y,
		NP, NL, DTEX_COMPRESSION_NONE, DTEX_TYPE_FLOAT, &m_dtexImage
	);

	
	if ( status != DTEX_NOERR )
	{
		m_outputFileName = "";
		m_alphaOffset = 0;
		clean();
		throw IOException( std::string( "Failed to create the main sub-image in \"" ) + fileName() + "\" for writing." );
	}

	m_dtexPixel = DtexMakePixel( numChannels );
}

void SHWDeepImageWriter::clean()
{
	if ( m_dtexPixel )
	{
		DtexDestroyPixel( m_dtexPixel );
	}
	
	// DtexClose seems to cause fatal errors, unless a valid image was created
	if ( m_outputFile && m_outputFileName != "" )
	{
		DtexClose( m_outputFile );
	}
	
	if ( m_dtexCache )
	{
		DtexDestroyCache( m_dtexCache );
	}
	
	m_outputFile = 0;
	m_dtexCache = 0;
	m_dtexImage = 0;
	m_dtexPixel = 0;
}
