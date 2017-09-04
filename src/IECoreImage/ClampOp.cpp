//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"

#include "IECoreImage/ClampOp.h"

using namespace std;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ClampOp );

ClampOp::ClampOp()
	:	ChannelOp( "Clamps channel data within a given range." )
{
	FloatParameterPtr minParameter = new FloatParameter(
		"min",
		"Values below this value are clamped.",
		0.0f
	);
	
	FloatParameterPtr maxParameter = new FloatParameter(
		"max",
		"Values above this value are clamped.",
		1.0f
	);
	
	BoolParameterPtr enableMinToParameter = new BoolParameter(
		"enableMinTo",
		"When this is on, the minTo parameter is used.",
		false
	);
	
	FloatParameterPtr minToParameter = new FloatParameter(
		"minTo",
		"When enableMinTo is on, values less than min will "
		"be given this value, rather than just being clamped "
		"at min.",
		0.0f
	);
	
	BoolParameterPtr enableMaxToParameter = new BoolParameter(
		"enableMaxTo",
		"When this is on, the maxTo parameter is used.",
		false
	);
	
	FloatParameterPtr maxToParameter = new FloatParameter(
		"maxTo",
		"When enableMaxTo is on, values greater than max will "
		"be given this value, rather than just being clamped "
		"at max.",
		1.0f
	);

	
	parameters()->addParameter( minParameter );
	parameters()->addParameter( maxParameter );
	parameters()->addParameter( enableMinToParameter );
	parameters()->addParameter( minToParameter );
	parameters()->addParameter( enableMaxToParameter );
	parameters()->addParameter( maxToParameter );
}

ClampOp::~ClampOp()
{
}

FloatParameter *ClampOp::minParameter()
{
	return parameters()->parameter<FloatParameter>( "min" );
}

const FloatParameter *ClampOp::minParameter() const
{
	return parameters()->parameter<FloatParameter>( "min" );
}

FloatParameter *ClampOp::maxParameter()
{
	return parameters()->parameter<FloatParameter>( "max" );
}

const FloatParameter *ClampOp::maxParameter() const
{
	return parameters()->parameter<FloatParameter>( "max" );
}

BoolParameter *ClampOp::enableMinToParameter()
{
	return parameters()->parameter<BoolParameter>( "enableMinTo" );
}

const BoolParameter *ClampOp::enableMinToParameter() const
{
	return parameters()->parameter<BoolParameter>( "enableMinTo" );
}
		
FloatParameter *ClampOp::minToParameter()
{
	return parameters()->parameter<FloatParameter>( "minTo" );
}

const FloatParameter *ClampOp::minToParameter() const
{
	return parameters()->parameter<FloatParameter>( "minTo" );
}

BoolParameter *ClampOp::enableMaxToParameter()
{
	return parameters()->parameter<BoolParameter>( "enableMaxTo" );
}

const BoolParameter *ClampOp::enableMaxToParameter() const
{
	return parameters()->parameter<BoolParameter>( "enableMaxTo" );
}

FloatParameter *ClampOp::maxToParameter()
{
	return parameters()->parameter<FloatParameter>( "maxTo" );
}

const FloatParameter *ClampOp::maxToParameter() const
{
	return parameters()->parameter<FloatParameter>( "maxTo" );
}

void ClampOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	float minValue = minParameter()->getNumericValue();
	float maxValue = maxParameter()->getNumericValue();
	
	float minTo = enableMinToParameter()->getTypedValue() ? minToParameter()->getNumericValue() : minValue;
	float maxTo = enableMaxToParameter()->getTypedValue() ? maxToParameter()->getNumericValue() : maxValue;
	
	for( unsigned i=0; i<channels.size(); i++ )
	{
		vector<float> &channel = channels[i]->writable();
		for( vector<float>::iterator it = channel.begin(), eIt = channel.end(); it != eIt; it++ )
		{
			*it = *it < minValue ? minTo : ( *it > maxValue ? maxTo : *it );
		}
	}
}

