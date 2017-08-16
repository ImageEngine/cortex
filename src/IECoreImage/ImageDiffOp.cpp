//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include <iostream>
#include <cassert>

#include "boost/format.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/Exception.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/Reader.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/MeanSquaredError.h"

#include "IECoreImage/ImageCropOp.h"
#include "IECoreImage/ImageDiffOp.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ImageDiffOp );

ImageDiffOp::ImageDiffOp()
		:	Op(
			"Evaluates the root-mean-squared error between two images and returns true if it "
			"exceeds a specified threshold. Unless the \"skip missing channels\" parameter is "
			"enabled, it will also return true if either image contains a channel which  "
			"the other doesn't.",
		        new BoolParameter(
		                "result",
		                "True if the image differ, false if they're considered the same",
		                true
		        )
		)
{

	m_imageAParameter = new ImagePrimitiveParameter(
	        "imageA",
	        "First image for comparison",
	        new ImagePrimitive()
	);

	m_imageBParameter = new ImagePrimitiveParameter(
	        "imageB",
	        "Second image for comparison",
	        new ImagePrimitive()
	);

	m_maxErrorParameter = new FloatParameter(
	        "maxError",
	        "Maximum permissible RMS error between the two images",
	        0.01f
	);

	m_skipMissingChannelsParameter = new BoolParameter(
	        "skipMissingChannels",
	        "If true then channels present in one image but missing in the other are ignored. If false, then missing channels mean the images are different.",
	        false
	);
	
	m_alignDisplayWindowsParameter = new BoolParameter(
	        "alignDisplayWindows",
	        "If true then display windows that are offset from the origin are moved to the origin before being compared.",
	        false
	);

	parameters()->addParameter( m_imageAParameter );
	parameters()->addParameter( m_imageBParameter );
	parameters()->addParameter( m_maxErrorParameter );
	parameters()->addParameter( m_skipMissingChannelsParameter );
	parameters()->addParameter( m_alignDisplayWindowsParameter );
}

ImageDiffOp::~ImageDiffOp()
{
}

ImagePrimitiveParameter * ImageDiffOp::imageAParameter()
{
	return m_imageAParameter.get();
}

const ImagePrimitiveParameter * ImageDiffOp::imageAParameter() const
{
	return m_imageAParameter.get();
}

ImagePrimitiveParameter * ImageDiffOp::imageBParameter()
{
	return m_imageBParameter.get();
}

const ImagePrimitiveParameter * ImageDiffOp::imageBParameter() const
{
	return m_imageBParameter.get();
}

FloatParameter * ImageDiffOp::maxErrorParameter()
{
	return m_maxErrorParameter.get();
}

const FloatParameter * ImageDiffOp::maxErrorParameter() const
{
	return m_maxErrorParameter.get();
}

BoolParameter * ImageDiffOp::skipMissingChannels()
{
	return m_skipMissingChannelsParameter.get();
}

const BoolParameter * ImageDiffOp::skipMissingChannels() const
{
	return m_skipMissingChannelsParameter.get();
}

BoolParameter * ImageDiffOp::alignDisplayWindows()
{
	return m_alignDisplayWindowsParameter.get();
}

const BoolParameter * ImageDiffOp::alignDisplayWindows() const
{
	return m_alignDisplayWindowsParameter.get();
}

/// A class to use a ScaledDataConversion to transform image data to floating point, to allow for simple measuring of
/// error between two potentially different data types (e.g. UShort and Half)
struct ImageDiffOp::FloatConverter
{
	typedef FloatVectorDataPtr ReturnType;

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		assert( data );

		return DataConvert < T, FloatVectorData, ScaledDataConversion< typename T::ValueType::value_type, float > >()
		       (
		               boost::static_pointer_cast<const T>( data )
		       );
	};
};

ObjectPtr ImageDiffOp::doOperation( const CompoundObject * operands )
{
	ImagePrimitivePtr imageA = m_imageAParameter->getTypedValue< ImagePrimitive >();
	ImagePrimitivePtr imageB = m_imageBParameter->getTypedValue< ImagePrimitive >();

	if ( imageA == imageB )
	{
		msg( Msg::Warning, "ImageDiffOp", "Exact same image specified as both input parameters.");
		return new BoolData( false );
	}

	if ( !imageA || !imageB )
	{
		throw InvalidArgumentException( "ImageDiffOp: NULL image specified as input parameter" );
	}

	assert( imageA );
	assert( imageB );

	if ( !imageA->arePrimitiveVariablesValid() || !imageB->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "ImageDiffOp: Image with invalid primitive variables specified as input parameter" );
	}

	const bool alignDisplayWindows = m_alignDisplayWindowsParameter->getTypedValue();
	
	if( alignDisplayWindows )
	{
		/// Fail if the display windows are of a different width or height.
		if ( ( imageA->getDisplayWindow().size().x != imageB->getDisplayWindow().size().x ) || ( imageA->getDisplayWindow().size().y != imageB->getDisplayWindow().size().y ) )
		{
			return new BoolData( true );
		}

		// If the display windows are different to each other then we move them back to the origin.
		if ( imageA->getDisplayWindow().min != imageB->getDisplayWindow().min || imageA->getDisplayWindow().min != Imath::V2i( 0 ) )
		{
			Imath::V2i offsetA = imageA->getDisplayWindow().min;
			Imath::Box2i displayWindowA( imageA->getDisplayWindow().min-offsetA, imageA->getDisplayWindow().max-offsetA );
			Imath::Box2i dataWindowA( imageA->getDataWindow().min-offsetA, imageA->getDataWindow().max-offsetA );
			imageA->setDisplayWindow( displayWindowA );
			imageA->setDataWindow( dataWindowA );

			Imath::V2i offsetB = imageB->getDisplayWindow().min;
			Imath::Box2i displayWindowB( imageB->getDisplayWindow().min-offsetB, imageB->getDisplayWindow().max-offsetB );
			Imath::Box2i dataWindowB( imageB->getDataWindow().min-offsetB, imageB->getDataWindow().max-offsetB );
			imageB->setDisplayWindow( displayWindowB );
			imageB->setDataWindow( dataWindowB );
		}
	}
	else if ( imageA->getDisplayWindow() != imageB->getDisplayWindow() )
	{
		return new BoolData( true );
	}

	/// Use the CropOp to expand the dataWindows of both images to fill the display window
	ImageCropOpPtr cropOp = new ImageCropOp();
	cropOp->matchDataWindowParameter()->setTypedValue( true );
	cropOp->cropBoxParameter()->setTypedValue( imageA->getDisplayWindow() );

	cropOp->inputParameter()->setValue( imageA );
	imageA = runTimeCast< ImagePrimitive >( cropOp->operate() );

	cropOp->inputParameter()->setValue( imageB );
	imageB = runTimeCast< ImagePrimitive >( cropOp->operate() );

	const float maxError = m_maxErrorParameter->getNumericValue();

	const bool skipMissingChannels = m_skipMissingChannelsParameter->getTypedValue();

	std::vector< std::string > channelsA;
	imageA->channelNames( channelsA );

	if ( !skipMissingChannels )
	{
		std::vector< std::string > channelsB, channelsIntersection ;

		imageA->channelNames( channelsA );
		imageB->channelNames( channelsB );

		std::set_intersection(
			channelsA.begin(), channelsA.end(),
			channelsB.begin(), channelsB.end(),
			std::inserter( channelsIntersection, channelsIntersection.begin() )
		);

		if ( channelsIntersection != channelsA || channelsIntersection != channelsB )
		{
			return new BoolData( true );
		}
	}

	for ( std::vector< std::string >::const_iterator it = channelsA.begin(); it != channelsA.end(); ++it )
	{
		PrimitiveVariableMap::const_iterator aPrimVarIt = imageA->variables.find( *it );
		assert( aPrimVarIt != imageA->variables.end() );
		assert( aPrimVarIt->second.interpolation == PrimitiveVariable::Vertex );

		PrimitiveVariableMap::const_iterator bPrimVarIt = imageB->variables.find( *it );
		if ( bPrimVarIt == imageB->variables.end() )
		{
			assert( skipMissingChannels );
			continue;
		}

		assert( bPrimVarIt->second.interpolation == PrimitiveVariable::Vertex );

		DataPtr aData = aPrimVarIt->second.data;

		DataPtr bData = 0;
		if ( bPrimVarIt != imageB->variables.end() )
		{
			bData = bPrimVarIt->second.data;
		}

		if ( aData == bData )
		{
			msg( Msg::Warning, "ImageDiffOp", "Exact same data found in two different input images.");
			continue;
		}

		if ( !aData || !bData )
		{
			msg( Msg::Warning, "ImageDiffOp", "Null data present in input image.");
			return new BoolData( true );
		}

		assert( aData );
		assert( bData );

		FloatVectorDataPtr aFloatData = 0;
		FloatVectorDataPtr bFloatData = 0;

		try
		{
			aFloatData = despatchTypedData< FloatConverter, TypeTraits::IsNumericVectorTypedData > ( aData.get() );
			bFloatData = despatchTypedData< FloatConverter, TypeTraits::IsNumericVectorTypedData > ( bData.get() );
		}
		catch ( Exception &e )
		{
			msg( Msg::Warning, "ImageDiffOp", boost::format( "Could not convert data for image channel '%s' to floating point" ) % *it );
			return new BoolData( true );
		}

		assert( aFloatData );
		assert( bFloatData );
		assert( aFloatData->readable().size() == bFloatData->readable().size() );

		float rms = sqrt( MeanSquaredError<FloatVectorData>()( aFloatData, bFloatData ) );
		if ( rms > maxError )
		{
			return new BoolData( true );
		}
	}

	return new BoolData( false );
}
