//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <boost/format.hpp>

#include "IECore/ImageCropOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/BoxOps.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/DespatchTypedData.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

ImageCropOp::ImageCropOp()
	:	ModifyOp(
		staticTypeName(),
		"Performs cropping over ImagePrimitive objects.\n"
		"The operation results on an ImagePrimitive with displayWindow equal to the intersection of the given crop box and the original image displayWindow.\n"
		"If matchDataWindow if On then the dataWindow will match the new displayWindow (new pixels will be filled with zero). Otherwise it will only be intersected against the given crop box.",
		new ImagePrimitiveParameter(
			"result",
			"Cropped image.",
			new ImagePrimitive()
		),
		new ImagePrimitiveParameter(
			"object",
			"The vector object that will be transformed by the matrix.",
			new ImagePrimitive()
		)
	)
{
	m_cropBox = new Box2iParameter(
		"cropBox",
		"Determines the crop coordinates to apply on the image.",
		new Box2iData()
	);

	parameters()->addParameter( m_cropBox );

	m_matchDataWindow = new BoolParameter(
		"matchDataWindow",
		"if On then the dataWindow will match displayWindow. Otherwise it will be intersected against the given crop box.",	
		new BoolData( false )
	);

	parameters()->addParameter( m_matchDataWindow );

	m_resetOrigin = new BoolParameter(
		"resetOrigin",
		"if On then the resulting image will have it's top-left corner at (0,0).",	
		new BoolData( true )
	);

	parameters()->addParameter( m_resetOrigin );

}

ImageCropOp::~ImageCropOp()
{
}


struct ImageCropOp::ImageCropFn
{
	typedef DataPtr ReturnType;
	
	const Imath::Box2i m_sourceDataWindow;
	const Imath::Box2i m_copyWindow;
	const Imath::Box2i m_targetDataWindow;	
	
	ImageCropFn( const Imath::Box2i &sourceDataWindow, const Imath::Box2i &copyWindow, const Imath::Box2i &targetDataWindow )
	: m_sourceDataWindow( sourceDataWindow ), m_copyWindow( copyWindow ), m_targetDataWindow( targetDataWindow )
	{
	}

	template< typename T>
	ReturnType operator()( typename T::Ptr source  ) const
	{
		assert( source );
		
		typename T::Ptr newChannel = new T;
		int sourceX, sourceY, targetX, targetY, sourceWidth, targetWidth, targetHeight, cropWidth, cropHeight;
		sourceWidth = m_sourceDataWindow.max.x - m_sourceDataWindow.min.x + 1;
		targetWidth = m_targetDataWindow.max.x - m_targetDataWindow.min.x + 1;
		targetHeight = m_targetDataWindow.max.y - m_targetDataWindow.min.y + 1;
		cropWidth = m_copyWindow.max.x - m_copyWindow.min.x + 1;
		cropHeight = m_copyWindow.max.y - m_copyWindow.min.y + 1;
		sourceX = m_copyWindow.min.x - m_sourceDataWindow.min.x;
		sourceY = m_copyWindow.min.y - m_sourceDataWindow.min.y;
		targetX = m_copyWindow.min.x - m_targetDataWindow.min.x;
		targetY = m_copyWindow.min.y - m_targetDataWindow.min.y;

#ifndef NDEBUG
		size_t numPixelsCopied = 0;
#endif		

		if ( targetWidth > 0 && targetHeight > 0 )
		{
			typename T::ValueType &target = newChannel->writable();
			target.resize( targetWidth * targetHeight );
			typename T::ValueType::iterator targetIt;

			if ( cropWidth < targetWidth || cropHeight < targetHeight )
			{
				for ( targetIt = target.begin(); targetIt != target.end(); targetIt++ )
				{
					*targetIt = 0;
				}
			}

			typename T::ValueType::const_iterator sourceIt = source->readable().begin()+sourceWidth*sourceY+sourceX;
			targetIt = target.begin()+targetWidth*targetY+targetX;

			for ( int y = 0; y < cropHeight; y++ )
			{
				std::copy( sourceIt, sourceIt + cropWidth, targetIt );
#ifndef NDEBUG
				numPixelsCopied += cropWidth;
#endif				
				sourceIt += sourceWidth;
				targetIt += targetWidth;
			}
		}		
		assert( numPixelsCopied <= ((size_t)( m_targetDataWindow.size().x + 1 ) * ( m_targetDataWindow.size().y + 1)) );		
		
		assert( newChannel );		
		return newChannel;
	}

};

void ImageCropOp::modify( ObjectPtr toModify, ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr image = static_pointer_cast< ImagePrimitive >( toModify );

	// first, make sure the input image is correct.
	if ( !image->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "ImageCropOp: Input image is not valid!" );
	}

	const Imath::Box2i &cropBox = m_cropBox->getTypedValue();
	bool matchDataWindow = m_matchDataWindow->getTypedValue();
	bool resetOrigin = m_resetOrigin->getTypedValue();

	Imath::Box2i croppedDisplayWindow = boxIntersection( cropBox, image->getDisplayWindow() );
	const Imath::Box2i &dataWindow = image->getDataWindow();
	Imath::Box2i croppedDataWindow = boxIntersection( cropBox, dataWindow );
	Imath::Box2i newDisplayWindow = croppedDisplayWindow;
	Imath::Box2i newDataWindow;

	if ( matchDataWindow )
	{
		newDataWindow = newDisplayWindow;
	}
	else
	{
		newDataWindow = croppedDataWindow;
	}

	for ( PrimitiveVariableMap::iterator varIt = image->variables.begin(); varIt != image->variables.end(); varIt++ )
	{
		PrimitiveVariable &channel = varIt->second;
		DataPtr data = channel.data;

		switch ( channel.interpolation )
		{
			case PrimitiveVariable::Vertex:
			case PrimitiveVariable::Varying:
			case PrimitiveVariable::FaceVarying:
				{			
					ImageCropFn fn( dataWindow, croppedDataWindow, newDataWindow );

					channel.data = despatchTypedData< ImageCropFn, TypeTraits::IsNumericVectorTypedData >( data, fn );			
				}
				break;
			
			default:
				// do nothing.
				break;
		}
	}

	if ( resetOrigin )
	{
#ifndef NDEBUG
		V2i newDisplayWindowSize = newDisplayWindow.size();
		V2i newDataWindowSize = newDataWindow.size();
#endif	
		newDisplayWindow.max = newDisplayWindow.max - newDisplayWindow.min;
		newDataWindow.min = newDataWindow.min - newDisplayWindow.min;
		newDataWindow.max = newDataWindow.max - newDisplayWindow.min;
		newDisplayWindow.min = Imath::V2i( 0, 0 );
		assert( newDisplayWindowSize == newDisplayWindow.size() );
		assert( newDataWindowSize == newDataWindow.size() );	
	}	
	image->setDataWindow( newDataWindow );
	image->setDisplayWindow( newDisplayWindow );
	
	assert( image->arePrimitiveVariablesValid() );
}

