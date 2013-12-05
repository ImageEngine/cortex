//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DeepImageReader.h"
#include "IECore/DeepPixel.h"

#include "IECoreNuke/DeepImageReader.h"

namespace IECoreNuke
{

DeepImageReader::DeepImageReader( DD::Image::DeepReaderOwner *op, const std::string &fileName ) :
	DD::Image::DeepReader( op )
{
	std::string errorMsg;
	if( !loadFileFromPath( fileName, errorMsg ) )
	{
		_op->error( errorMsg.c_str() );
		return;
	}
	
	// Set the output size, channels and context.
	int width = m_reader->displayWindow().size().x + 1;
	int height = m_reader->displayWindow().size().y + 1;
	setInfo( width, height, _owner->readerOutputContext(), m_channels );

	// Set the metadata.
	Imath::M44f cam = m_reader->worldToCameraMatrix();
	m_meta.setData( "cortex/worldToCamera", &cam[0][0], 16 );
	
	Imath::M44f ndc = m_reader->worldToNDCMatrix();
	m_meta.setData( "cortex/worldToNDC", &ndc[0][0], 16 );
}

bool DeepImageReader::doDeepEngine( DD::Image::Box box, const DD::Image::ChannelSet &channels, DD::Image::DeepOutputPlane &plane )
{
	plane = DD::Image::DeepOutputPlane( channels, box );

	if( !m_reader || m_channels.empty() )
	{
		for( int y = box.y(); y < box.t(); ++y )
		{
			for( int ox = box.x(); ox != box.r(); ++ox )
			{
				plane.addHole();
			}
		}

		return true;
	}

	DD::Image::Guard g( m_lock );
	Imath::Box2i displayWindow( m_reader->displayWindow() );	

	for( int y = box.y(); y < box.t(); ++y )
	{
		if( y < m_dataWindow.y() || y >= m_dataWindow.t() )
		{
			for( int ox = box.x(); ox != box.r(); ++ox )
			{
				plane.addHole();
			}
			continue;
		}

		int minX = std::min( m_dataWindow.x(), box.x() );
		int maxX = std::max( m_dataWindow.r(), box.r() );

		// The Y coordinate in the cortex deep image coordinate space.	
		int cy = displayWindow.size().y - ( y - displayWindow.min[1] );

		for( int x = minX; x < maxX; ++x )
		{
			if( x < m_dataWindow.x() || x >= m_dataWindow.r() )
			{
				plane.addHole();
				continue;
			}

			unsigned int nSamples( 0 );

			IECore::DeepPixelPtr pixel;
			try
			{
				pixel = m_reader->readPixel( x, cy );
				if( pixel )
				{
					nSamples = pixel->numSamples(); 
				}
			}
			catch( const IECore::Exception &e )
			{
			}

			if( nSamples == 0 )
			{
				if( x >= box.x() || x < box.r() )
				{
					plane.addHole();
				}
				continue;
			}

			if( x < box.x() || x >= box.r() )
			{	
				continue;
			}
		
			DD::Image::DeepOutPixel dop;
			float previousBack = pixel->getDepth( 0 );
			for( unsigned int i = 0; i < nSamples; ++i )
			{
				float *data( pixel->channelData( i ) );
				float depth = pixel->getDepth( i );
				DD::Image::ChannelSet allChans = m_channels + channels;
				foreach( z, allChans )
				{
					if( z == DD::Image::Chan_DeepFront )
					{
						dop.push_back( previousBack );
						previousBack = depth;
					}
					else if( z == DD::Image::Chan_DeepBack )
					{
						dop.push_back( depth );
					}
					else
					{
						if( m_channels.contains(z) )
						{
							dop.push_back( data[ m_channelMap[z] ] );
						}
						else
						{
							dop.push_back(0);
						}
					}
				}
			}
			plane.addPixel( dop );
		}
	}
	return true;
}

bool DeepImageReader::loadFileFromPath( const std::string &filePath, std::string &errorMsg )
{
	try
	{
		// Perform an early-out if we have already loaded the desired file.
		if( m_currentPath == filePath && m_reader && m_currentPath != "" )
		{
			return true;
		}

		IECore::ReaderPtr object( IECore::Reader::create( filePath ) );
		m_reader = IECore::runTimeCast<IECore::DeepImageReader>( object );
		
		if( m_reader )
		{
			m_currentPath = filePath;
			std::vector< std::string > channelNames;

			m_reader->channelNames( channelNames );

			m_channels = DD::Image::ChannelSet( DD::Image::Mask_DeepFront | DD::Image::Mask_DeepBack );
			m_channelMap.clear();
			for( std::vector< std::string >::const_iterator it( channelNames.begin() ); it != channelNames.end(); ++it )
			{
				int idx( it - channelNames.begin() ); 
				if( *it == "A" )
				{
					m_channels += DD::Image::Chan_Alpha;
					m_channelMap[DD::Image::Chan_Alpha] = idx; 
				}
				else if( *it == "R" )
				{
					m_channels += DD::Image::Chan_Red;
					m_channelMap[DD::Image::Chan_Red] = idx; 
				}
				else if( *it == "G" )
				{
					m_channels += DD::Image::Chan_Green;
					m_channelMap[DD::Image::Chan_Green] = idx; 
				}
				else if( *it == "B" )
				{
					m_channels += DD::Image::Chan_Blue;
					m_channelMap[DD::Image::Chan_Blue] = idx; 
				}
			}
			
			Imath::Box2i dataWindow( m_reader->dataWindow() ); 
			m_dataWindow = DD::Image::Box( dataWindow.min[0], dataWindow.min[1], dataWindow.max[0]+1, dataWindow.max[1]+1 );
			errorMsg = "";
			return true;
		}
		else
		{
			errorMsg = "Object is not an IECore::DeepImageReader.";
		}
	}
	catch( const IECore::Exception &e )
	{
		errorMsg = ( boost::format( "DeepImageReader : %s" ) % e.what() ).str();
	}
	return false;
}

DD::Image::DeepReader *DeepImageReader::build( DD::Image::DeepReaderOwner *op, const std::string &fn )
{
	return new DeepImageReader( op, fn );
}

DD::Image::DeepReaderFormat *DeepImageReader::buildformat( DD::Image::DeepReaderOwner *op )
{
	return new DeepImageReaderFormats();
}

const DD::Image::MetaData::Bundle &DeepImageReader::fetchMetaData( const char *key )
{
	  return m_meta;
}

const char *DeepImageReader::supportedExtensions()
{
	// We have to hard code the supported deep image types here
	// because we cannot call IECore::Reader::supportedExtensions
	// as we are inside a static initializer.
	// We have also omitted the IECoreDL dtex reader here as nuke
	// already supports it out of the box.
	return "shw\0dsm\0rat\0";
}

const DD::Image::DeepReader::Description DeepImageReader::g_description( supportedExtensions(), "cortex", build, buildformat );

} // namespace IECoreNuke
