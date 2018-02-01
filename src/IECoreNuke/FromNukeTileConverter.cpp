//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/FromNukeTileConverter.h"

#include "IECoreImage/ImagePrimitive.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreImage;
using namespace IECoreNuke;

FromNukeTileConverter::FromNukeTileConverter( const DD::Image::Tile *tile )
	:	FromNukeConverter( "Converts nuke Tiles to IECoreImage ImagePrimitives." ), m_tile( tile )
{
}

FromNukeTileConverter::~FromNukeTileConverter()
{
}

IECore::ObjectPtr FromNukeTileConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	Box2i dataWindow( V2i( m_tile->x(), m_tile->y() ), V2i( m_tile->r() - 1, m_tile->t() - 1 ) );
	ImagePrimitivePtr result = new ImagePrimitive( dataWindow, dataWindow );

	foreach( z, m_tile->channels() )
	{
		std::string name;
		switch( z )
		{
			case DD::Image::Chan_Red :
				name = "R";
				break;
			case DD::Image::Chan_Green :
				name = "G";
				break;
			case DD::Image::Chan_Blue :
				name = "B";
				break;
			case DD::Image::Chan_Alpha :
				name = "A";
				break;
			case DD::Image::Chan_Z :
				name = "Z";
				break;
			default :
				name = getName( z );
		}

		FloatVectorDataPtr channelData = result->createChannel<float>( name );
		float *out = &*(channelData->writable().begin());

		for( int y=m_tile->t()-1; y>=m_tile->y(); y-- )
		{
			memcpy( out, (*m_tile)[z][y] + m_tile->x(), m_tile->w() * sizeof( float ) );
			out += m_tile->w();
		}
	}

	return result;
}
