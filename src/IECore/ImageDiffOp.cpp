//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ImageDiffOp.h"

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
#include "IECore/ImageCropOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( ImageDiffOp );

ImageDiffOp::ImageDiffOp()
		:	Op(
		        staticTypeName(),			
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

	parameters()->addParameter( m_imageAParameter );
	parameters()->addParameter( m_imageBParameter );
	parameters()->addParameter( m_maxErrorParameter );
	parameters()->addParameter( m_skipMissingChannelsParameter );
}

ImageDiffOp::~ImageDiffOp()
{
}

ImagePrimitiveParameterPtr ImageDiffOp::imageAParameter()
{
	return m_imageAParameter;
}

ConstImagePrimitiveParameterPtr ImageDiffOp::imageAParameter() const
{
	return m_imageAParameter;
}

ImagePrimitiveParameterPtr ImageDiffOp::imageBParameter()
{
	return m_imageBParameter;
}

ConstImagePrimitiveParameterPtr ImageDiffOp::imageBParameter() const
{
	return m_imageBParameter;
}

FloatParameterPtr ImageDiffOp::maxErrorParameter()
{
	return m_maxErrorParameter;
}

ConstFloatParameterPtr ImageDiffOp::maxErrorParameter() const
{
	return m_maxErrorParameter;
}

BoolParameterPtr ImageDiffOp::skipMissingChannels()
{
	return m_skipMissingChannelsParameter;
}

ConstBoolParameterPtr ImageDiffOp::skipMissingChannels() const
{
	return m_skipMissingChannelsParameter;
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

ObjectPtr ImageDiffOp::doOperation( ConstCompoundObjectPtr operands )
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
	
	if ( imageA->getDisplayWindow() != imageB->getDisplayWindow() )
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
			aFloatData = despatchTypedData< FloatConverter, TypeTraits::IsNumericVectorTypedData > ( aData );
			bFloatData = despatchTypedData< FloatConverter, TypeTraits::IsNumericVectorTypedData > ( bData );
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
