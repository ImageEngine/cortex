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

#include <cassert>

#include "IECore/ImageCompositeOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/ImageCropOp.h"
#include "IECore/CompositeAlgo.h"
#include "IECore/ImagePremultiplyOp.h"
#include "IECore/ImageUnpremultiplyOp.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "boost/format.hpp"

using boost::str;
using boost::format;

using namespace IECore;
using namespace Imath;

ImageCompositeOp::ImageCompositeOp() : ImagePrimitiveOp( "ImageCompositeOp", "ImageCompositeOp" )
{
	IntParameter::PresetsMap operationPresets;
	operationPresets["Over"] = Over;
	operationPresets["Max"] = Max;
	operationPresets["Min"] = Min;
	operationPresets["Multiply"] = Multiply;
	m_operationParameter = new IntParameter(
		"operation",
		"operation description",
		Over,
		operationPresets
	);

	StringVectorDataPtr defaultChannels = new StringVectorData;
	defaultChannels->writable().push_back( "R" );
	defaultChannels->writable().push_back( "G" );
	defaultChannels->writable().push_back( "B" );
	
	m_channelNamesParameter = new StringVectorParameter(
		"channels",
		"The names of the channels to modify.",
		defaultChannels
	);
	
	m_alphaChannelNameParameter = new StringParameter(
		"alphaChannelName", 
		"The name of the channel which holds the alpha. This is used for both images.",
		"A"
	);

	m_imageAParameter = new ImagePrimitiveParameter(
		"imageA",
		"imageA is the second image operand of the composite. It is named such that operation names like 'A over B' make sense. "
		"Therefore parameter named 'input' represents imageB",
		new ImagePrimitive()
	);
	
	IntParameter::PresetsMap inputModePresets;
	inputModePresets["Premultiplied"] = Premultiplied;
	inputModePresets["Unpremultiplied"] = Unpremultiplied;
	m_inputModeParameter = new IntParameter(
		"inputMode",
		"States whether the input images are premultiplied by their alpha.",
		Premultiplied,
		inputModePresets
	);

	parameters()->addParameter( m_operationParameter );
	parameters()->addParameter( m_channelNamesParameter );
	parameters()->addParameter( m_alphaChannelNameParameter );
	parameters()->addParameter( m_imageAParameter );
	parameters()->addParameter( m_inputModeParameter );	
}

ImageCompositeOp::~ImageCompositeOp()
{
}

StringVectorParameterPtr ImageCompositeOp::channelNamesParameter()
{
	return m_channelNamesParameter;
}

ConstStringVectorParameterPtr ImageCompositeOp::channelNamesParameter() const
{
	return m_channelNamesParameter;
}

StringParameterPtr ImageCompositeOp::alphaChannelNameParameter()
{
	return m_alphaChannelNameParameter;
}

ConstStringParameterPtr ImageCompositeOp::alphaChannelNameParameter() const
{
	return m_alphaChannelNameParameter;
}

ImagePrimitiveParameterPtr ImageCompositeOp::imageAParameter()
{
	return m_imageAParameter;
}

ConstImagePrimitiveParameterPtr ImageCompositeOp::imageAParameter() const
{
	return m_imageAParameter;
}

IntParameterPtr ImageCompositeOp::operationParameter()
{
	return m_operationParameter;
}

ConstIntParameterPtr ImageCompositeOp::operationParameter() const
{
	return m_operationParameter;
}

IntParameterPtr ImageCompositeOp::inputModeParameter()
{
	return m_inputModeParameter;
}

ConstIntParameterPtr ImageCompositeOp::inputModeParameter() const
{
	return m_inputModeParameter;
}

FloatVectorDataPtr ImageCompositeOp::getChannelData( ImagePrimitivePtr image, const std::string &channelName, bool mustExist )
{
	assert( image );
	
	PrimitiveVariableMap::iterator it = image->variables.find( channelName );
	if( it==image->variables.end() )
	{
		if ( mustExist )
		{
			throw Exception( str( format( "ImageCompositeOp: Channel \"%s\" does not exist." ) % channelName ) );
		} 
		else
		{
			return 0;
		}
	}

	if( it->second.interpolation!=PrimitiveVariable::Vertex &&
		it->second.interpolation!=PrimitiveVariable::Varying &&
		it->second.interpolation!=PrimitiveVariable::FaceVarying )
	{
		throw Exception( str( format( "ImageCompositeOp: Primitive variable \"%s\" has inappropriate interpolation." ) % channelName ) );
	}

	if( !it->second.data )
	{
		throw Exception( str( format( "ImageCompositeOp: Primitive variable \"%s\" has no data." ) % channelName ) );
	}

	if( !it->second.data->isInstanceOf( FloatVectorData::staticTypeId() ) )
	{
		throw Exception( str( format( "ImageCompositeOp: Primitive variable \"%s\" has inappropriate type." ) % channelName ) );
	}
	
	return assertedStaticCast< FloatVectorData >( it->second.data );
}

float ImageCompositeOp::readChannelData( ConstImagePrimitivePtr image, ConstFloatVectorDataPtr data, const V2i &pixel )
{
	assert( image );
	assert( data );
		
	V2i offset = pixel - image->getDataWindow().min;
	if ( offset.x < 0 || offset.y < 0 )
	{
		return 0.0f;
	}
	
	const int width = image->getDataWindow().size().x + 1;
	const int height = image->getDataWindow().size().y + 1;
	
	if ( offset.x >= width || offset.y >= height )
	{
		return 0.0f;
	}
	
	const int idx = offset.y * width + offset.x;
	
	assert( idx >= 0 );
	assert( idx < (int)data->readable().size() );
	return data->readable()[ idx ];
}

void ImageCompositeOp::composite( CompositeFn fn, DataWindowResult dwr, ImagePrimitivePtr imageB, ConstCompoundObjectPtr operands )
{
	assert( fn );
	assert( imageB );
	assert( operands );
	
	const StringVectorParameter::ValueType &channelNames = channelNamesParameter()->getTypedValue();
	if ( !channelNames.size() )
	{
		throw InvalidArgumentException( "ImageCompositeOp: No channels specified" );
	}			
	
	ImagePrimitivePtr imageA = runTimeCast< ImagePrimitive > ( imageAParameter()->getValue() );
	assert( imageA );
	
	const std::string &alphaChannel = alphaChannelNameParameter()->getTypedValue();
	if ( !imageA->arePrimitiveVariablesValid() )	
	{
		throw InvalidArgumentException( "ImageCompositeOp: Input image has invalid channels" );
	}
	
	const int inputMode = m_inputModeParameter->getNumericValue();
	if ( inputMode == Unpremultiplied )
	{
		ImagePremultiplyOpPtr premultOp = new ImagePremultiplyOp();
		premultOp->alphaChannelNameParameter()->setTypedValue( alphaChannel );
		premultOp->channelNamesParameter()->setTypedValue( channelNames );
		
		if ( imageA->variables.find( alphaChannel ) != imageA->variables.end() )
		{
			/// Make a new imageA, premultiplied
			premultOp->copyParameter()->setTypedValue( true );
			premultOp->inputParameter()->setValue( imageA );
		
			imageA = assertedStaticCast< ImagePrimitive >( premultOp->operate() );
			assert( imageA->arePrimitiveVariablesValid() );
		}
		
		if ( imageB->variables.find( alphaChannel ) != imageB->variables.end() )
		{		
			/// Premultiply imageB in-place
			premultOp->copyParameter()->setTypedValue( false );
			premultOp->inputParameter()->setValue( imageB );
			premultOp->operate();
		}
	}
	else
	{
		assert( inputMode == Premultiplied );
	}
	

	const Imath::Box2i displayWindow = imageB->getDisplayWindow();
	
	Imath::Box2i newDataWindow = imageB->getDataWindow();
	
	if ( dwr == Union )
	{				
		newDataWindow.extendBy( imageA->getDataWindow() );		
	}
	else
	{
		assert( dwr == Intersection );
		newDataWindow = boxIntersection( newDataWindow, imageA->getDataWindow() );
	}
	
	newDataWindow = boxIntersection( newDataWindow, displayWindow );
	
	ImageCropOpPtr cropOp = new ImageCropOp();

	/// Need to make sure that we don't create a new image here - we want to modify the current one in-place. So,
	/// we turn off the "copy" parameter of ModifyOp.
	cropOp->copyParameter()->setTypedValue( false );	
	cropOp->inputParameter()->setValue( imageB );	
	cropOp->cropBoxParameter()->setTypedValue( newDataWindow );
	cropOp->matchDataWindowParameter()->setTypedValue( true );
	cropOp->resetOriginParameter()->setTypedValue( false );
		
	cropOp->operate();
	
	assert( imageB->arePrimitiveVariablesValid() );	
	assert( imageB->getDataWindow() == newDataWindow );	
	
	/// \todo Use the "reformat" parameter of the ImageCropOp to do this, when it's implemented
	imageB->setDisplayWindow( displayWindow );	
		
	FloatVectorDataPtr aAlphaData = getChannelData( imageA, alphaChannel, false );
	FloatVectorDataPtr bAlphaData = getChannelData( imageB, alphaChannel, false );
		
	const int newWidth = newDataWindow.size().x + 1;
	const int newHeight = newDataWindow.size().y + 1;	
	const int newArea = newWidth * newHeight;
	
	assert( newArea == (int)imageB->variableSize( PrimitiveVariable::Vertex ) );
			
	for( unsigned i=0; i<channelNames.size(); i++ )
	{
		const StringVectorParameter::ValueType::value_type &channelName = channelNames[i];

		FloatVectorDataPtr aData = getChannelData( imageA, channelName );
		assert( aData->readable().size() == imageA->variableSize( PrimitiveVariable::Vertex ) );
		FloatVectorDataPtr bData = getChannelData( imageB, channelName );
		assert( bData->readable().size() == imageB->variableSize( PrimitiveVariable::Vertex ) );		
		FloatVectorDataPtr newBData = new FloatVectorData();
		
		newBData->writable().resize( newArea );
		imageB->variables[ channelName ].data = newBData;		
		
		for ( int y = newDataWindow.min.y; y <= newDataWindow.max.y; y++ )
		{
			int offset = (y - newDataWindow.min.y ) * newWidth;
			for ( int x = newDataWindow.min.x; x <= newDataWindow.max.x; x++, offset++ )
			{
				float aVal = readChannelData( imageA, aData, V2i( x, y ) );
				float bVal = readChannelData( imageB, bData, V2i( x, y ) );
				
				float aAlpha = aAlphaData ? readChannelData( imageA, aAlphaData, V2i( x, y ) ) : 1.0f;			
				float bAlpha = bAlphaData ? readChannelData( imageB, bAlphaData, V2i( x, y ) ) : 1.0f;
				
				assert( offset >= 0 );
				assert( offset < (int)newBData->readable().size() );
				newBData->writable()[ offset  ] = fn( aVal, aAlpha, bVal, bAlpha );				
			}
		}				
	}
	
	/// displayWindow should be unchanged
	assert( imageB->getDisplayWindow() == displayWindow );
	assert( imageB->arePrimitiveVariablesValid() );
	
	/// If input images were unpremultiplied, then ensure that the output is also
	if ( inputMode == Unpremultiplied && imageB->variables.find( alphaChannel ) != imageB->variables.end() )
	{
		ImageUnpremultiplyOpPtr unpremultOp = new ImageUnpremultiplyOp();
		/// Unpremultiply imageB in-place		
		unpremultOp->copyParameter()->setTypedValue( false );
		unpremultOp->channelNamesParameter()->setTypedValue( channelNames );
		unpremultOp->alphaChannelNameParameter()->setTypedValue( alphaChannel );
		unpremultOp->inputParameter()->setValue( imageB );
		unpremultOp->operate();	
		assert( imageB->arePrimitiveVariablesValid() );	
	}
	else
	{
		assert( inputMode == Premultiplied );
	}
}

void ImageCompositeOp::modifyTypedPrimitive( ImagePrimitivePtr imageB, ConstCompoundObjectPtr operands )
{
	if ( !imageB->arePrimitiveVariablesValid() )	
	{
		throw InvalidArgumentException( "ImageCompositeOp: Input image has invalid channels" );
	}
	
	const Operation operation = (Operation)operationParameter()->getNumericValue();
	
	switch (operation) 
	{
		case Over :
			composite( compositeOver<float>, Union, imageB, operands );
			break;
		case Max :
			composite( compositeMax<float>, Union, imageB, operands );
			break;	
		case Min :
			composite( compositeMin<float>, Intersection, imageB, operands );
			break;
		case Multiply :
			composite( compositeMultiply<float>, Intersection, imageB, operands );
			break;				
						
		default :		
			assert( false );	
	}
}

