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
#include <algorithm>
#include <cassert>

#include "IECore/PrimitiveVariable.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/ImagePrimitiveEvaluator.h"
#include "IECore/SimpleTypedData.h"

#include "IECore/MappedRandomPointDistributionOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

MappedRandomPointDistributionOp::MappedRandomPointDistributionOp()
		:	UniformRandomPointDistributionOp(
		        staticTypeName(),
		        "The MappedRandomPointDistributionOp distributes points over a mesh using a random distribution and a density map."
		)
{
	m_imageParameter = new ImagePrimitiveParameter(
	        "image",
	        "The image specifying the density map.",
	        new ImagePrimitive()
	);

	StringParameter::PresetsContainer channelNamePresets;
	channelNamePresets.push_back( StringParameter::Preset( "R", "R" ) );
	channelNamePresets.push_back( StringParameter::Preset( "G", "G" ) );
	channelNamePresets.push_back( StringParameter::Preset( "B", "B" ) );
	channelNamePresets.push_back( StringParameter::Preset( "A", "A" ) );
	channelNamePresets.push_back( StringParameter::Preset( "Y", "Y" ) );

	m_channelNameParameter = new StringParameter(
	        "channelName",
	        "The name of the floating point channel in the image to use as the density map.",
	        "Y",
	        channelNamePresets,
	        false
	);

	parameters()->addParameter( m_imageParameter );
	parameters()->addParameter( m_channelNameParameter );
}

MappedRandomPointDistributionOp::~MappedRandomPointDistributionOp()
{
}

ImagePrimitiveParameterPtr MappedRandomPointDistributionOp::imageParameter()
{
	return m_imageParameter;
}

ConstImagePrimitiveParameterPtr MappedRandomPointDistributionOp::imageParameter() const
{
	return m_imageParameter;
}

StringParameterPtr MappedRandomPointDistributionOp::channelNameParameter()
{
	return m_channelNameParameter;
}

ConstStringParameterPtr MappedRandomPointDistributionOp::channelNameParameter() const
{
	return m_channelNameParameter;
}

float MappedRandomPointDistributionOp::density( ConstMeshPrimitivePtr mesh, const Imath::V3f &point, const Imath::V2f &uv ) const
{
	assert( m_imageEvaluator );
	assert( m_imageEvaluator->primitive() );
	assert( runTimeCast<const ImagePrimitive>( m_imageEvaluator->primitive() ) );
	assert( m_channelIterator != runTimeCast<const ImagePrimitive>( m_imageEvaluator->primitive() )->variables.end() );
	
	/// \todo Texture repeat
	float repeatU = 1.0;
	float repeatV = 1.0;
	
	/// \todo Wrap modes	
	bool wrapU = true;
	bool wrapV = true;
	
	Imath::V2f placedUv(
		uv.x * repeatU,
		uv.y * repeatV
	);
	
	if ( wrapU )
	{
		placedUv.x = fmod( (double)placedUv.x, 1.0 );
	}
	
	if ( wrapV )
	{
		placedUv.y = fmod( (double)placedUv.y, 1.0 );
	}	
	
	bool found = m_imageEvaluator->pointAtUV( placedUv, m_result );
		
	if ( found )
	{
		return m_result->floatPrimVar( m_channelIterator->second );
	}
	else
	{
		return 0.0f;
	}
}

ObjectPtr MappedRandomPointDistributionOp::doOperation( ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr image = m_imageParameter->getTypedValue<ImagePrimitive>();
	assert( image );

	std::string channelName = m_channelNameParameter->getTypedValue();

	m_imageEvaluator = new ImagePrimitiveEvaluator( image );
	m_channelIterator = image->variables.find( channelName );

	if ( m_channelIterator == image->variables.end() )
	{
		throw InvalidArgumentException( "MappedRandomPointDistributionOp: Cannot find channel " + channelName + " in image" );
	}

	m_result = m_imageEvaluator->createResult();

	return this->UniformRandomPointDistributionOp::doOperation( operands );
}
