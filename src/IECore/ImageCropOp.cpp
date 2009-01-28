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
#include "IECore/ClassData.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

/// \todo Move this into the main class for the next major version
struct ImageCropOp::ExtraMembers
{	
	IECore::BoolParameterPtr m_intersectParameter;
};

static IECore::ClassData<ImageCropOp, ImageCropOp::ExtraMembers> g_extraMembers;

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
	ExtraMembers &extraMembers = g_extraMembers.create( this );

	m_cropBox = new Box2iParameter(
		"cropBox",
		"Determines the crop coordinates to apply on the image.",
		Box2i()
	);

	parameters()->addParameter( m_cropBox );

	m_matchDataWindow = new BoolParameter(
		"matchDataWindow",
		"if On then the dataWindow will match displayWindow. Otherwise it will be intersected against the given crop box.",	
		
		/// \todo The original intent was that this default should be false, but when specifying "new BoolData(false)" as
		/// the default value it seems the compiler would prefer to convert the BoolData* to a bool over a BoolData::Ptr.
		/// Consider changing the default if required, but only on the next major version change.
		true
	);

	parameters()->addParameter( m_matchDataWindow );

	m_resetOrigin = new BoolParameter(
		"resetOrigin",
		"if On then the resulting image will have it's top-left corner at (0,0).",	
		true
	);

	parameters()->addParameter( m_resetOrigin );
	
	extraMembers.m_intersectParameter = new BoolParameter(
		"intersect",
		"If enabled then the display window of the resultant image will be cropped against the crop reigion too",
		true
	);

	parameters()->addParameter( extraMembers.m_intersectParameter );
	
	
	/// \todo Add "reformat" parameter, like Nuke
	/// Current behaviour is to reformat, so take care to set the default accordingly.
}

ImageCropOp::~ImageCropOp()
{
}

Box2iParameterPtr ImageCropOp::cropBoxParameter()
{
	return m_cropBox;
}

ConstBox2iParameterPtr ImageCropOp::cropBoxParameter() const
{
	return m_cropBox;
}

BoolParameterPtr ImageCropOp::matchDataWindowParameter()
{
	return m_matchDataWindow;
}

ConstBoolParameterPtr ImageCropOp::matchDataWindowParameter() const
{
	return m_matchDataWindow;
}

BoolParameterPtr ImageCropOp::resetOriginParameter()
{
	return m_resetOrigin;
}

ConstBoolParameterPtr ImageCropOp::resetOriginParameter() const
{
	return m_resetOrigin;
}

BoolParameterPtr ImageCropOp::intersectParameter()
{
	return g_extraMembers[this].m_intersectParameter;
}

ConstBoolParameterPtr ImageCropOp::intersectParameter() const
{
	return g_extraMembers[this].m_intersectParameter;
}

struct ImageCropOp::ImageCropFn
{
	typedef DataPtr ReturnType;
	
	const Imath::Box2i m_sourceDataWindow;
	const Imath::Box2i m_croppedDataWindow;
	const Imath::Box2i m_targetDataWindow;	
	
	ImageCropFn( const Imath::Box2i &sourceDataWindow, const Imath::Box2i &croppedDataWindow, const Imath::Box2i &targetDataWindow )
	: m_sourceDataWindow( sourceDataWindow ), m_croppedDataWindow( croppedDataWindow ), m_targetDataWindow( targetDataWindow )
	{
	}

	template< typename T>
	ReturnType operator()( typename T::ConstPtr sourceData ) const
	{
		assert( sourceData );
		
		typename T::Ptr newChannel = new T();
		
		int sourceWidth = m_sourceDataWindow.max.x - m_sourceDataWindow.min.x + 1;

		int targetWidth = m_targetDataWindow.max.x - m_targetDataWindow.min.x + 1;
		int targetHeight = m_targetDataWindow.max.y - m_targetDataWindow.min.y + 1;
		int targetArea = targetWidth * targetHeight;

#ifndef NDEBUG
		int numPixelsCopied = 0;
#endif		

		typename T::ValueType &target = newChannel->writable();
		target.resize( targetArea, 0 );
					
		const typename T::ValueType &source = sourceData->readable();

		/// \todo Optimize
		Imath::V2i copyPixel;
		for ( copyPixel.y = m_croppedDataWindow.min.y; copyPixel.y <= m_croppedDataWindow.max.y; copyPixel.y ++ )
		{
			for ( copyPixel.x = m_croppedDataWindow.min.x; copyPixel.x <= m_croppedDataWindow.max.x; copyPixel.x ++ )
			{
				if ( m_sourceDataWindow.intersects( copyPixel ) && m_targetDataWindow.intersects( copyPixel ) )
				{
					assert( boxIntersection( m_sourceDataWindow, m_targetDataWindow ).intersects( copyPixel ) );
				
					int sourceOffset = sourceWidth * ( copyPixel.y - m_sourceDataWindow.min.y ) + ( copyPixel.x - m_sourceDataWindow.min.x );
					assert( sourceOffset >= 0 );
					assert( sourceOffset < (int)source.size() );
					int targetOffset = targetWidth * ( copyPixel.y - m_targetDataWindow.min.y ) + ( copyPixel.x - m_targetDataWindow.min.x );
					assert( targetOffset >= 0 );
					assert( targetOffset < (int)target.size() );

					target[ targetOffset++ ] = source[ sourceOffset++ ];
#ifndef NDEBUG
					numPixelsCopied ++;
#endif					
				}
				else
				{
					assert( !boxIntersection( m_sourceDataWindow, m_targetDataWindow ).intersects( copyPixel ) );
				}
			}
		}
		
		assert( (int)target.size() == targetArea );
		assert( numPixelsCopied <= (int)target.size() );
		
		assert( newChannel );			
		return newChannel;
	}

};

void ImageCropOp::modify( ObjectPtr toModify, ConstCompoundObjectPtr operands )
{	
	ImagePrimitivePtr image = assertedStaticCast< ImagePrimitive >( toModify );

	// Validate the input image
	if ( !image->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "ImageCropOp: Input image is not valid" );
	}

	const Imath::Box2i &cropBox = m_cropBox->getTypedValue();
	if ( cropBox.isEmpty() )
	{
		throw InvalidArgumentException( "ImageCropOp: Specified crop box is empty" );
	}
	
	const bool resetOrigin = m_resetOrigin->getTypedValue();
	const bool intersect = g_extraMembers[this].m_intersectParameter->getTypedValue();

	Imath::Box2i croppedDisplayWindow;
	if ( intersect )
	{
		croppedDisplayWindow = boxIntersection( cropBox, image->getDisplayWindow() );
	}
	else
	{
		croppedDisplayWindow = cropBox;
	}
	const Imath::Box2i &dataWindow = image->getDataWindow();
	
	Imath::Box2i croppedDataWindow = boxIntersection( cropBox, dataWindow );
	
	Imath::Box2i newDisplayWindow = croppedDisplayWindow;
	Imath::Box2i newDataWindow;
	
	const bool matchDataWindow = m_matchDataWindow->getTypedValue();
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

		switch ( channel.interpolation )
		{
			case PrimitiveVariable::Vertex:
			case PrimitiveVariable::Varying:
			case PrimitiveVariable::FaceVarying:
				{			
					DataPtr data = channel.data;
					assert( data );
					ImageCropFn fn( dataWindow, croppedDataWindow, newDataWindow );

					channel.data = despatchTypedData< ImageCropFn, TypeTraits::IsNumericVectorTypedData >( data, fn );
					assert( channel.data );
				}
				break;
			
			default:
				/// Nothing to do for these channel types
				assert( channel.interpolation == PrimitiveVariable::Constant || channel.interpolation == PrimitiveVariable::Uniform );
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
		
	if ( matchDataWindow )
	{
		assert( image->getDisplayWindow() == image->getDataWindow() );
	}
	
	assert( image->arePrimitiveVariablesValid() );
}

