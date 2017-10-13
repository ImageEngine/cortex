//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/MessageHandler.h"

#include "IECoreImage/SplineToImage.h"

#include "IECoreGL/SplineToGLTextureConverter.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/LuminanceTexture.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( SplineToGLTextureConverter );

SplineToGLTextureConverter::ConverterDescription<SplineToGLTextureConverter> SplineToGLTextureConverter::g_descriptionff( IECore::SplineffData::staticTypeId(), IECoreGL::Texture::staticTypeId() );
SplineToGLTextureConverter::ConverterDescription<SplineToGLTextureConverter> SplineToGLTextureConverter::g_descriptionfColor3f( IECore::SplinefColor3fData::staticTypeId(), IECoreGL::Texture::staticTypeId() );
SplineToGLTextureConverter::ConverterDescription<SplineToGLTextureConverter> SplineToGLTextureConverter::g_descriptionfColor4f( IECore::SplinefColor4fData::staticTypeId(), IECoreGL::Texture::staticTypeId() );

SplineToGLTextureConverter::SplineToGLTextureConverter( IECore::ConstObjectPtr toConvert )
	:	ToGLConverter( "Converts IECore::SplineData objects to IECoreGL::Texture objects.", IECore::ObjectTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::Object>( toConvert ) );

	m_resolutionParameter = new IECore::V2iParameter(
		"resolution",
		"The resolution of the created ImagePrimitive",
		Imath::V2i( 8, 512 )
	);

}

SplineToGLTextureConverter::~SplineToGLTextureConverter()
{
}

IECore::RunTimeTypedPtr SplineToGLTextureConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{

	TexturePtr t = nullptr;
	IECoreImage::SplineToImagePtr op = new IECoreImage::SplineToImage();
	op->splineParameter()->setValue( boost::const_pointer_cast< IECore::Object >(src) );
	op->resolutionParameter()->setValue( m_resolutionParameter->getValue() );
	IECoreImage::ImagePrimitivePtr image = boost::static_pointer_cast<IECoreImage::ImagePrimitive>( op->operate() );

	bool r = image->channelValid( "R" );
	bool g = image->channelValid( "G" );
	bool b = image->channelValid( "B" );
	bool y = image->channelValid( "Y" );

	if ( !y && r && g && b )
	{
			t = new ColorTexture( image.get() );
	}
	else if ( y && !r && !g && !b )
	{
			t = new LuminanceTexture( image.get() );
	}
	else
	{
		throw IECore::Exception( "Invald image format, SplineToGLTextureConverter supports RGB[A] and Y[A]." );
	}

	if ( ! t )
	{
		throw IECore::Exception( "Failed to create IECoreGL Texture." );
	}

	return t;
}

