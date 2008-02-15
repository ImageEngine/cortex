//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
#include "IECore/BoxOperators.h"
#include "IECore/ImagePrimitive.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

ImageCropOp::ImageCropOp()
	:	ModifyOp(
		staticTypeName(),
		"Performs cropping over ImagePrimitive objects.\n"
		"The operation results on an ImagePrimitive with displayWindow equal to the given crop box.\n"
		"If matchDataWindow if On then the dataWindow will match displayWindow. Otherwise it will be intersected against the given crop box.",
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

template< typename T>
DataPtr cropImageVariable( boost::intrusive_ptr<T> source, const Imath::Box2i &sourceDataWindow, const Imath::Box2i &copyWindow, const Imath::Box2i &targetDataWindow )
{
	boost::intrusive_ptr<T> newChannel = new T;
	int sourceX, sourceY, targetX, targetY, sourceWidth, targetWidth, targetHeight, cropWidth, cropHeight;
	sourceWidth = sourceDataWindow.max.x - sourceDataWindow.min.x + 1;
	targetWidth = targetDataWindow.max.x - targetDataWindow.min.x + 1;
	targetHeight = targetDataWindow.max.y - targetDataWindow.min.y + 1;
	cropWidth = copyWindow.max.x - copyWindow.min.x + 1;
	cropHeight = copyWindow.max.y - copyWindow.min.y + 1;
	sourceX = copyWindow.min.x - sourceDataWindow.min.x;
	sourceY = copyWindow.min.y - sourceDataWindow.min.y;
	targetX = copyWindow.min.x - targetDataWindow.min.x;
	targetY = copyWindow.min.y - targetDataWindow.min.y;

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
			sourceIt += sourceWidth;
			targetIt += targetWidth;
		}
	}
	return newChannel;
}

void ImageCropOp::modify( ObjectPtr toModify, ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr image = static_pointer_cast< ImagePrimitive >( toModify );

	// first, make sure the input image is correct.
	if ( !image->arePrimitiveVariablesValid() )
	{
		throw Exception( "Input image is not valid!" );
	}

	const Imath::Box2i &cropBox = m_cropBox->getTypedValue();
	bool matchDataWindow = m_matchDataWindow->getTypedValue();
	bool resetOrigin = m_resetOrigin->getTypedValue();

	Imath::Box2i dataWindow = image->getDataWindow();
	Imath::Box2i croppedDataWindow = intersection( cropBox, dataWindow );
	Imath::Box2i newDisplayWindow = cropBox;
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

			if ( data->typeId() == FloatVectorDataTypeId )
			{
				channel.data = cropImageVariable( boost::static_pointer_cast< FloatVectorData >(data),
										dataWindow, croppedDataWindow, newDataWindow );
			}
			else if (data->typeId() == HalfVectorDataTypeId )
			{
				channel.data = cropImageVariable( boost::static_pointer_cast< HalfVectorData >(data),
										dataWindow, croppedDataWindow, newDataWindow );
			}
			else if (data->typeId() == UIntVectorDataTypeId )
			{
				channel.data = cropImageVariable( boost::static_pointer_cast< UIntVectorData >(data),
										dataWindow, croppedDataWindow, newDataWindow );
			}
			else
			{
				throw Exception( "Invalid channel type in input image!" );
			}
			break;
		default:
			// do nothing.
			break;
		}
	}

	if ( resetOrigin )
	{
		newDisplayWindow.max = newDisplayWindow.max - newDisplayWindow.min;
		newDataWindow.min = newDataWindow.min - newDisplayWindow.min;
		newDataWindow.max = newDataWindow.max - newDisplayWindow.min;
		newDisplayWindow.min = Imath::V2i( 0, 0 );
	}
	image->setDataWindow( newDisplayWindow );
	image->setDisplayWindow( newDataWindow );
}
