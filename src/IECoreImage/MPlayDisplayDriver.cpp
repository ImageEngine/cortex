//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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
#include <stdio.h>
#include "boost/format.hpp"

#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreImage/MPlayDisplayDriver.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

//////////////////////////////////////////////////////////////////////////
// Header definitions required by imdisplay
//////////////////////////////////////////////////////////////////////////

struct MPlayDisplayDriver::ImageHeader
{
	ImageHeader( const V2i &resolution, size_t numPlanes )
	{
		magicNumber = (('h'<<24)+('M'<<16)+('P'<<8)+'0');
		xRes = resolution.x;
		yRes = resolution.y;
		dataType = 0; // floating point data
		numChannels = 0; // multiplane
		multiPlaneCount = numPlanes;
		reserved[0] = 0;
		reserved[1] = 0;
	}
	
	int	magicNumber;
	int	xRes;
	int	yRes;
	int	dataType;
	int	numChannels;
	int	multiPlaneCount;
	int	reserved[2];
};

struct MPlayDisplayDriver::TileHeader
{
	// this weird invalid tile header is used to signify that
	// we're about to send a plane.
	TileHeader( size_t planeIndex )
	{
		x0 = -1;
		x1 = planeIndex;
		y0 = 0;
		y1 = 0;
	}
	
	TileHeader( const Box2i &box )
	{
		x0 = box.min.x;
		x1 = box.max.x;
		y0 = box.min.y;
		y1 = box.max.y;
	}
	
	int	x0, x1;
	int	y0, y1;
};

struct MPlayDisplayDriver::PlaneHeader
{
	PlaneHeader( const Plane &plane, size_t index )
	{
		planeIndex = index;
		nameLength = plane.name.size();
		dataType = 0; // floating point data
		numChannels = plane.channelIndices.size();
		reserved[0] = 0;
		reserved[1] = 0;
		reserved[2] = 0;
		reserved[3] = 0;
	}
	
	int	planeIndex;
	int	nameLength;
	int	dataType;
	int	numChannels;
	int	reserved[4];
};


//////////////////////////////////////////////////////////////////////////
// MPlayDisplayDriver implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MPlayDisplayDriver );

const DisplayDriver::DisplayDriverDescription<MPlayDisplayDriver> MPlayDisplayDriver::g_description;

MPlayDisplayDriver::MPlayDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters )
	:	DisplayDriver( displayWindow, dataWindow, channelNames, parameters )
{
	// Sort out our flat list of channels into planes, based on common prefixes
	Plane *currentPlane = 0;
	for( vector<string>::const_iterator cIt = channelNames.begin(); cIt != channelNames.end(); cIt++ )
	{
		size_t separatorIndex = cIt->find( '.' );
		string planeName;
		string channelName = *cIt;
		if( separatorIndex != string::npos )
		{
			planeName = string( *cIt, 0, separatorIndex );
			channelName = string( *cIt, separatorIndex );
		}
		else
		{
			planeName = "C";
		}
		
		if( !m_planes.size() || m_planes.rbegin()->name != planeName )
		{
			m_planes.push_back( Plane( planeName ) );
			currentPlane = &(*m_planes.rbegin());
		}
		currentPlane->channelNames.push_back( channelName );
		currentPlane->channelIndices.push_back( cIt - channelNames.begin() );
	}
	
	// Validate that our planes match mplay requirements.
	for( vector<Plane>::iterator pIt = m_planes.begin(); pIt != m_planes.end(); pIt++ )
	{
		if(
			pIt->channelNames.size() != 1 &&
			pIt->channelNames.size() != 3 &&
			pIt->channelNames.size() != 4
		)
		{
			throw Exception( "MPlayDisplayDriver only supports 1, 3, and 4 channel images" );
		}
		
		// Make sure that the "C" plane is first, as otherwise MPlay
		// gets a bit upset. It's ok to swap with the first plane because
		// we already checked that one.
		if( pIt->name == "C" && pIt != m_planes.begin() )
		{
			std::swap( *pIt, m_planes[0] );
		}
	}
	
	// Construct a command line calling imdisplay, and open it as a pipe
	std::string commandLine = "imdisplay -f -p";
	
	commandLine += boost::str( boost::format( " -o %d %d" ) % displayWindow.min.x % displayWindow.min.y );
	
	V2i originalSize = displayWindow.size() + V2i( 1 );
	commandLine += boost::str( boost::format( " -Z %d %d" ) % originalSize.x % originalSize.y );
			
	const StringData *extraArguments = parameters->member<StringData>( "imdisplayExtraArguments" );
	if( extraArguments && extraArguments->readable().size() )
	{
		commandLine += " " + extraArguments->readable();
	}
		
	m_imDisplayStdIn = popen( commandLine.c_str(), "w" );

	// Pipe out our image header
	ImageHeader header( dataWindow.size() + V2i( 1 ), m_planes.size() );
	fwrite( &header, sizeof( ImageHeader ), 1, m_imDisplayStdIn );
	
	// Pipe out a header for each of our planes
	for( vector<Plane>::const_iterator pIt = m_planes.begin(); pIt != m_planes.end(); pIt++ )
	{
		PlaneHeader planeHeader( *pIt, pIt - m_planes.begin() );	
		fwrite( &planeHeader, sizeof( PlaneHeader ), 1, m_imDisplayStdIn );
		fwrite( &(pIt->name[0]), pIt->name.size(), 1, m_imDisplayStdIn );
	}
	
}

MPlayDisplayDriver::~MPlayDisplayDriver()
{
	// in case imageClose() wasn't called for any reason.
	if( m_imDisplayStdIn )
	{
		pclose( m_imDisplayStdIn );
	}
}

bool MPlayDisplayDriver::scanLineOrderOnly() const
{
	return false;
}

bool MPlayDisplayDriver::acceptsRepeatedData() const
{
	return false;
}

void MPlayDisplayDriver::imageData( const Imath::Box2i &box, const float *data, size_t dataSize )
{	
	for( size_t planeIndex = 0; planeIndex < m_planes.size(); planeIndex++ )
	{
		const Plane &plane = m_planes[planeIndex];
	
		TileHeader planeHeader( planeIndex );
		fwrite( &planeHeader, sizeof( planeHeader ), 1, m_imDisplayStdIn );
	
		TileHeader tileHeader( box );
		fwrite( &tileHeader, sizeof( tileHeader ), 1, m_imDisplayStdIn );
		
		const size_t numPixels = ( box.size().x + 1 ) * ( box.size().y + 1 );
		
		// need to create interleaved data for the channels in the current plane
		std::vector<float> planeData( plane.channelIndices.size() * numPixels );
		const float *in = data;
		float *out = &planeData[0];
		const size_t numInChannels = channelNames().size();
		const size_t numOutChannels = plane.channelIndices.size();
								
		for( size_t i=0; i<numPixels; ++i )
		{
			for( size_t c=0; c<numOutChannels; ++c )
			{
				*out++ = in[plane.channelIndices[c]];
				
			}
			in += numInChannels;
		}
		
		fwrite( &planeData[0], sizeof( float ), planeData.size(), m_imDisplayStdIn );
	}
}

void MPlayDisplayDriver::imageClose()
{
	pclose( m_imDisplayStdIn );
	m_imDisplayStdIn = 0;
}

