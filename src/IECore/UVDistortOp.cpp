//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/UVDistortOp.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Interpolator.h"

using namespace boost;
using namespace IECore;
using namespace Imath;

UVDistortOp::UVDistortOp()
	:	WarpOp( 
			"UVDistortOp", 
			"Distorts an ImagePrimitive by using a UV map as reference. The UV map must have the same pixel aspect then the image to be distorted. "
			"The resulting image will have the same data window as the reference UV map."
		), m_u(0), m_v(0)
{
	m_uvMapParameter = new ObjectParameter(
		"uvMap", 
		"Image with the red and green values being floating point normalized coordinates (x,y) from the undistorted image.",
		new NullObject,
		ImagePrimitiveTypeId
	);
	
	parameters()->addParameter( m_uvMapParameter );
}

UVDistortOp::~UVDistortOp()
{
}

ObjectParameterPtr UVDistortOp::uvMapParameter()
{
	return m_uvMapParameter;
}

ConstObjectParameterPtr UVDistortOp::uvMapParameter() const
{
	return m_uvMapParameter;
}

void UVDistortOp::begin( ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr uvImage = static_pointer_cast<ImagePrimitive>(m_uvMapParameter->getValue());
	PrimitiveVariableMap::iterator mit;
	mit = uvImage->variables.find( "R" );
	if ( mit == uvImage->variables.end() )
	{
		throw Exception("No channel R found in the given uv map object!");
	}
	if ( mit->second.data->typeId() != FloatVectorDataTypeId )
	{
		throw Exception("Channel R in the given uv map is not float type!");
	}
	m_u = static_pointer_cast<FloatVectorData>(mit->second.data);

	mit = uvImage->variables.find( "G" );
	if ( mit == uvImage->variables.end() )
	{
		throw Exception("No channel G found in the given uv map object!");
	}
	if ( mit->second.data->typeId() != FloatVectorDataTypeId )
	{
		throw Exception("Channel G in the given uv map is not float type!");
	}
	m_v = static_pointer_cast<FloatVectorData>(mit->second.data);

	ImagePrimitivePtr inputImage = static_pointer_cast<ImagePrimitive>( inputParameter()->getValue() );
	m_uvSize = uvImage->getDataWindow().size();
	m_uvOrigin = uvImage->getDataWindow().min;
	m_imageSize = inputImage->getDisplayWindow().size();
	m_imageOrigin = inputImage->getDisplayWindow().min;
}

Imath::Box2i UVDistortOp::warpedDataWindow( const Imath::Box2i &dataWindow ) const
{
	return Imath::Box2i( m_uvOrigin, m_uvOrigin + m_uvSize );
}

Imath::V2f UVDistortOp::warp( const Imath::V2f &p ) const
{
	Imath::V2f uvCoord = (p - m_uvOrigin);
	int x = int(uvCoord.x);
	int y = int(uvCoord.y);
	x = ( x < 0 ? 0 : x > m_uvSize.x ? m_uvSize.x : x );
	y = ( y < 0 ? 0 : y > m_uvSize.y ? m_uvSize.y : y );
	unsigned pos = x + y * (m_uvSize.x + 1);
	return Imath::V2f( m_u->readable()[ pos ], m_v->readable()[ pos ] ) * m_imageSize + m_imageOrigin;
}

void UVDistortOp::end()
{
	m_u = 0;
	m_v = 0;
}
