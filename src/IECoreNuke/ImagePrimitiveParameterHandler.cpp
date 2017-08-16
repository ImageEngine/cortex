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

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/ImagePrimitiveParameter.h"

#include "IECoreNuke/ImagePrimitiveParameterHandler.h"
#include "IECoreNuke/FromNukeTileConverter.h"

using namespace IECoreNuke;

ParameterHandler::Description<ImagePrimitiveParameterHandler> ImagePrimitiveParameterHandler::g_description( (IECore::TypeId)IECoreImage::ImagePrimitiveParameter::staticTypeId() );

ImagePrimitiveParameterHandler::ImagePrimitiveParameterHandler()
{
}
		
int ImagePrimitiveParameterHandler::minimumInputs( const IECore::Parameter *parameter )
{
	return 1;
}

int ImagePrimitiveParameterHandler::maximumInputs( const IECore::Parameter *parameter )
{
	return 1;
}

bool ImagePrimitiveParameterHandler::testInput( const IECore::Parameter *parameter, int input, const DD::Image::Op *op )
{
	if( dynamic_cast<const DD::Image::Iop *>( op ) )
	{
		return 1;
	}
	return 0;
}

void ImagePrimitiveParameterHandler::setParameterValue( IECore::Parameter *parameter, InputIterator first, InputIterator last )
{
	DD::Image::Iop *iOp = static_cast<DD::Image::Iop *>( *first );
	if( iOp )
	{
		DD::Image::Tile tile( *iOp, iOp->requested_channels(), true );
		
		FromNukeTileConverterPtr converter = new FromNukeTileConverter( &tile );
		IECoreImage::ImagePrimitivePtr image = boost::static_pointer_cast<IECoreImage::ImagePrimitive>( converter->convert() );
		
		/// \todo Sort out data window vs display window and suchlike.
		
		parameter->setValue( image );
		return;
	}
	
	// no input - set parameter to default value.
	parameter->setValue( parameter->defaultValue()->copy() );
}
