//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/Math.h"
#include "IECore/Group.h"
#include "IECore/ImagePrimitive.h"

#include "IECoreImage/HdrMergeOp.h"

using namespace std;
using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( HdrMergeOp );

HdrMergeOp::HdrMergeOp()
	:	Op(
		"Merges all the given input images into a single HDR image.",
		new ObjectParameter(
			"result",
			"HDR image.",
			new ImagePrimitive(),
			ImagePrimitiveTypeId
		)
	)
{
	m_inputGroupParameter = new ObjectParameter(
		"inputGroup",
		"The Group of ImagePrimitive objects that will be merged. "
		"The order of the images in the group is important and should be from the less exposed to the most exposed image.",
		new Group(),
		GroupTypeId
	);
	m_exposureStepParameter = new FloatParameter(
		"exposureStep",
		"This number specifies the difference in exposure between one image and the next,"
		"measured in stops. You should set it to match the step used when shooting the bracketed "
		"images. Entering a higher number can produce an image which has more contrast and an "
		"artifically exaggerated dynamic range. Entering a lower number is undesirable as it " 
		"results in images without enough contrast.",
		1,
		0,
		4
	);
	m_exposureAdjustmentParameter = new FloatParameter(
		"exposureAdjustment",
		"By default the merged HDR output is balanced to match the overall brightness of the central "
		"exposure in the range. This exposure adjustment can be used to brighten or darken the output to "
		"the desired level. Note that this is just a simple multiplier on the output values, and has no "
		"effect on the actual merging process.",
		1,
		0,
		4
	);
	m_windowingParameter = new Box2fParameter(
		"windowing",
		"When calculating the contribution of an image to the result, very dark or light values are ignored. This "
		"prevents problems where noise from the dark areas appears in the output, or clipped highlights result in "
		"an inaccurate result. This parameter controls the intensity values which are ignored. All values below the "
		"leftmost value or above the rightmost value are completely ignored. Pixels with intensities within the "
		"range between the two central values are considered to have maximum importance. Pixels in the transition "
		"zones are weighted with a smooth curve.",
		new Box2fData( Box2f( V2f( 0.0, 0.05 ), V2f( 0.9, 1.0 ) ) )
	);
	parameters()->addParameter( m_inputGroupParameter );
	parameters()->addParameter( m_exposureStepParameter );
	parameters()->addParameter( m_exposureAdjustmentParameter );
	parameters()->addParameter( m_windowingParameter );
}

HdrMergeOp::~HdrMergeOp()
{
}

ObjectParameter * HdrMergeOp::inputGroupParameter()
{
	return m_inputGroupParameter.get();
}

const ObjectParameter * HdrMergeOp::inputGroupParameter() const
{
	return m_inputGroupParameter.get();
}

FloatParameter * HdrMergeOp::exposureStepParameter()
{
	return m_exposureStepParameter.get();
}

const FloatParameter * HdrMergeOp::exposureStepParameter() const
{
	return m_exposureStepParameter.get();
}

FloatParameter * HdrMergeOp::exposureAdjustmentParameter()
{
	return m_exposureAdjustmentParameter.get();
}

const FloatParameter * HdrMergeOp::exposureAdjustmentParameter() const
{
	return m_exposureAdjustmentParameter.get();
}

Box2fParameter * HdrMergeOp::windowingParameter()
{
	return m_windowingParameter.get();
}

const Box2fParameter * HdrMergeOp::windowingParameter() const
{
	return m_windowingParameter.get();
}

template< typename T >
inline void merge( bool firstImage, size_t &pixelCount,
					const ImagePrimitive * img, ImagePrimitive * outImg,
					const Imath::Box2f &windowing, float intensityMultiplier,
					FloatVectorData * outR, FloatVectorData * outG, FloatVectorData * outB, FloatVectorData * outA )
{

	const TypedData< std::vector< T > > *inR = img->getChannel< T >( "R" );
	const TypedData< std::vector< T > > *inG = img->getChannel< T >( "G" );
	const TypedData< std::vector< T > > *inB = img->getChannel< T >( "B" );

	if ( firstImage )
	{
		pixelCount = inR->readable().size();
		outR->writable().resize( pixelCount, 0 );
		outG->writable().resize( pixelCount, 0 );
		outB->writable().resize( pixelCount, 0 );
		outA->writable().resize( pixelCount, 0 );
		outImg->setDisplayWindow( img->getDisplayWindow() );
		outImg->setDataWindow( img->getDataWindow() );
	}
	else
	{
		if ( pixelCount != inR->readable().size() ||
			 pixelCount != inG->readable().size() ||
			 pixelCount != inB->readable().size() )
		{
			throw Exception( "Images are not of the same resolution!!" );
		}
	}

	const T *ptrInR = &(inR->readable()[0]);
	const T *ptrInG = &(inG->readable()[0]);
	const T *ptrInB = &(inB->readable()[0]);
	float *ptrOutR = &(outR->writable()[0]);
	float *ptrOutG = &(outG->writable()[0]);
	float *ptrOutB = &(outB->writable()[0]);
	float *ptrOutA = &(outA->writable()[0]);

	for ( size_t i = 0; i < pixelCount; i++ )
	{
		float intensity = (*ptrInR + *ptrInG + *ptrInB) / 3.0;
		float weight = smoothstep( windowing.min[0], windowing.min[1], intensity );
		if ( !firstImage )
		{
			weight *= 1.0f - smoothstep( windowing.max[0], windowing.max[1], intensity );
		}
		float m = weight * intensityMultiplier;
		*ptrOutR += *ptrInR * m;
		*ptrOutG += *ptrInG * m;
		*ptrOutB += *ptrInB * m;
		*ptrOutA += weight;

		ptrInR++; ptrInG++; ptrInB++;
		ptrOutR++; ptrOutG++; ptrOutB++; ptrOutA++;
	}
}

ObjectPtr HdrMergeOp::doOperation( const CompoundObject * operands )
{
	Group *imageGroup = static_cast<Group *>( m_inputGroupParameter->getValue() );

	// first of all, check if the group contains ImagePrimitive objects with float or half vector data types and "R","G","B" channels.
	const Group::ChildContainer &images = imageGroup->children();
	for ( Group::ChildContainer::const_iterator it = images.begin(); it != images.end(); it++ )
	{
		if ( (*it)->typeId() != ImagePrimitiveTypeId )
		{
			throw Exception( "Input group should contain images only!" );
		}
		ImagePrimitivePtr img = boost::static_pointer_cast< ImagePrimitive >(*it);
		if ( !((img->getChannel< float >( "R" ) &&
				img->getChannel< float >( "G" ) &&
				img->getChannel< float >( "B" )) ||
			   (img->getChannel< half >( "R" ) &&
			    img->getChannel< half >( "G" ) &&
			    img->getChannel< half >( "B" ))
			  )
			)
		{
			throw Exception( "Input images must have RGB channels of either half or float data types." );
		}
	}
	if ( images.size() == 0 )
	{
		throw Exception( "Input group has no images to merge!" );
	}

	float exposureStep = operands->member< FloatData >( "exposureStep" )->readable();
	float exposureAdjustment = operands->member< FloatData >("exposureAdjustment")->readable();
	Imath::Box2f windowing = operands->member< Box2fData >("windowing" )->readable();

	FloatVectorDataPtr outR = new FloatVectorData();
	FloatVectorDataPtr outG = new FloatVectorData();
	FloatVectorDataPtr outB = new FloatVectorData();
	FloatVectorDataPtr outA = new FloatVectorData();

	ImagePrimitivePtr outImg = new ImagePrimitive();
	outImg->variables["R"] = PrimitiveVariable( PrimitiveVariable::Vertex, outR );
	outImg->variables["G"] = PrimitiveVariable( PrimitiveVariable::Vertex, outG );
	outImg->variables["B"] = PrimitiveVariable( PrimitiveVariable::Vertex, outB );
	outImg->variables["A"] = PrimitiveVariable( PrimitiveVariable::Vertex, outA );

	// loop over inputs accumulating them into the buffer
	int numInputs = images.size();

	float exposure = exposureStep * (numInputs-1)/2.0;
	size_t pixelCount = 0;
	bool firstImage = true;
	for ( Group::ChildContainer::const_iterator it = images.begin(); it != images.end(); it++, firstImage = false )
	{
		const ImagePrimitive *img = static_cast<const ImagePrimitive *>( it->get() );
		float intensityMultiplier = pow( 2.0f, exposure );
		if ( img->getChannel< float >( "R" ) )
		{
			merge< float >( firstImage, pixelCount, img, outImg.get(), windowing, intensityMultiplier, outR.get(), outG.get(), outB.get(), outA.get() );
		}
		else
		{
			merge< half >( firstImage, pixelCount, img, outImg.get(), windowing, intensityMultiplier, outR.get(), outG.get(), outB.get(), outA.get() );
		}
		exposure -= exposureStep;
	}
	
	// normalize the outputs
	float adjustment = pow( 2.0f, -exposureAdjustment );
	float *ptrOutR = &(outR->writable()[0]);
	float *ptrOutG = &(outG->writable()[0]);
	float *ptrOutB = &(outB->writable()[0]);
	float *ptrOutA = &(outA->writable()[0]);

	for ( size_t i = 0; i < pixelCount; i++ )
	{
		float w = adjustment * ptrOutA[i];
		if ( w > 0 )
		{
			ptrOutR[i] /= w;
			ptrOutG[i] /= w;
			ptrOutB[i] /= w;
		}
	}

	return outImg;
}
