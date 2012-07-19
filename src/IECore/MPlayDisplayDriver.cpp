//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/MPlayDisplayDriver.h"
#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace Imath;

//////////////////////////////////////////////////////////////////////////
// Header definitions
//////////////////////////////////////////////////////////////////////////

struct MPlayDisplayDriver::ImageHeader
{
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
	int	x0, x1;
	int	y0, y1;
};

//////////////////////////////////////////////////////////////////////////
// MPlayDisplayDriver implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MPlayDisplayDriver );

const DisplayDriver::DisplayDriverDescription<MPlayDisplayDriver> MPlayDisplayDriver::g_description;

MPlayDisplayDriver::MPlayDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters )
	:	DisplayDriver( displayWindow, dataWindow, channelNames, parameters )
{
	if(
		channelNames.size() != 1 &&
		channelNames.size() != 3 &&
		channelNames.size() != 4
	)
	{
		throw Exception( "MPlayDisplayDriver only supports 1, 3, and 4 channel images" );
	}

	std::string commandLine = "imdisplay -f -p";
	
	
	V2i origin = dataWindow.min - displayWindow.min;
	commandLine += boost::str( boost::format( " -o %d %d" ) % origin.x % origin.y );
	
	V2i originalSize = displayWindow.size() + V2i( 1 );
	commandLine += boost::str( boost::format( " -Z %d %d" ) % originalSize.x % originalSize.y );
			
	const StringData *extraArguments = parameters->member<StringData>( "imdisplayExtraArguments" );
	if( extraArguments && extraArguments->readable().size() )
	{
		commandLine += " " + extraArguments->readable();
	}
		
	m_imDisplayStdIn = popen( commandLine.c_str(), "w" );

	ImageHeader header;
	header.magicNumber = (('h'<<24)+('M'<<16)+('P'<<8)+'0');
	header.xRes = dataWindow.size().x + 1;
	header.yRes = dataWindow.size().y + 1;
	header.dataType = 0; // floating point data
	header.numChannels = channelNames.size();
	header.multiPlaneCount = 0;
	header.reserved[0] = 0;
	header.reserved[1] = 0;
	
	fwrite( &header, sizeof( ImageHeader ), 1, m_imDisplayStdIn );
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

void MPlayDisplayDriver::imageData( const Imath::Box2i &box, const float *data, size_t dataSize )
{	
	TileHeader header;
	header.x0 = box.min.x;
	header.x1 = box.max.x;
	header.y0 = box.min.y;
	header.y1 = box.max.y;
	
	fwrite( &header, sizeof( header ), 1, m_imDisplayStdIn );
	fwrite( data, sizeof( float ), channelNames().size() * ( box.size().x + 1 ) * ( box.size().y + 1 ), m_imDisplayStdIn );
}

void MPlayDisplayDriver::imageClose()
{
	pclose( m_imDisplayStdIn );
	m_imDisplayStdIn = 0;
}

