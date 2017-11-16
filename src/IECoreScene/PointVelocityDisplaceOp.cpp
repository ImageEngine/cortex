//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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


#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECoreScene/PointVelocityDisplaceOp.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/TypedPrimitiveParameter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( PointVelocityDisplaceOp );

PointVelocityDisplaceOp::PointVelocityDisplaceOp()
	:	ModifyOp(
		"Displaces points using a velocity attribute.",
		new PrimitiveParameter(
			"result",
			"The updated Primitive with displace points.",
			new PointsPrimitive()
			),
		new PrimitiveParameter(
			"input",
			"The input Primitive with points to displace.",
			new PointsPrimitive()
			)
		)
{
	m_positionVarParameter = new StringParameter(
		"positionVar",
		"The variable name to use as per-point position.",
		"P" );

	m_velocityVarParameter = new StringParameter(
		"velocityVar",
		"The variable name to use as per-point velocity.",
		"v" );

	m_sampleLengthParameter = new FloatParameter(
		"sampleLength",
		"The sample time across which to displace P.",
		1.0 );

	m_sampleLengthVarParameter = new StringParameter(
		"sampleLengthVar",
		"The variable name to use as per-point sample length.",
		"" );

	parameters()->addParameter( m_positionVarParameter );
	parameters()->addParameter( m_velocityVarParameter );
	parameters()->addParameter( m_sampleLengthParameter );
	parameters()->addParameter( m_sampleLengthVarParameter );
}

PointVelocityDisplaceOp::~PointVelocityDisplaceOp()
{
}

StringParameter * PointVelocityDisplaceOp::positionVarParameter()
{
	return m_positionVarParameter.get();
}

const StringParameter * PointVelocityDisplaceOp::positionVarParameter() const
{
	return m_positionVarParameter.get();
}

StringParameter * PointVelocityDisplaceOp::velocityVarParameter()
{
	return m_velocityVarParameter.get();
}

const StringParameter * PointVelocityDisplaceOp::velocityVarParameter() const
{
	return m_velocityVarParameter.get();
}

FloatParameter * PointVelocityDisplaceOp::sampleLengthParameter()
{
	return m_sampleLengthParameter.get();
}

const FloatParameter * PointVelocityDisplaceOp::sampleLengthParameter() const
{
	return m_sampleLengthParameter.get();
}

StringParameter * PointVelocityDisplaceOp::sampleLengthVarParameter()
{
	return m_sampleLengthVarParameter.get();
}

const StringParameter * PointVelocityDisplaceOp::sampleLengthVarParameter() const
{
	return m_sampleLengthVarParameter.get();
}

void PointVelocityDisplaceOp::modify( Object *input, const CompoundObject *operands )
{
	// get input and parameters
	Primitive *pt = static_cast<Primitive *>( input );
	std::string position_var = operands->member<StringData>( "positionVar" )->readable();
	std::string velocity_var = operands->member<StringData>( "velocityVar" )->readable();
	float sample_length = operands->member<FloatData>( "sampleLength" )->readable();
	std::string sample_length_var = operands->member<StringData>( "sampleLengthVar" )->readable();

	// should we look for pp sample length?
	bool use_pp_sample_length = ( sample_length_var!="" );

	// check for variables
	if ( pt->variables.count(position_var)==0 )
	{
		throw Exception( "Could not find position variable on primitive!" );
	}
	if ( pt->variables.count(velocity_var)==0 )
	{
		throw Exception( "Could not find velocity variable on primitive!" );
	}

	// access our P & V information
	V3fVectorData *p = pt->variableData<V3fVectorData>(position_var);
	V3fVectorData *v = pt->variableData<V3fVectorData>(velocity_var);
	if ( !p || !v )
	{
		throw Exception("Could not get position and velocity data from primitive!");
	}

	// check P and V are the same length
	std::vector<V3f> &p_data = p->writable();
	const std::vector<V3f> &v_data = v->readable();
	if ( p_data.size()!=v_data.size() )
	{
		throw Exception("Position and velocity variables must be the same length!");
	}

	// update p using v
	if ( !use_pp_sample_length )
	{
		std::vector<V3f>::iterator p_it = p_data.begin();
		std::vector<V3f>::const_iterator v_it = v_data.begin();
		for ( p_it=p_data.begin(), v_it=v_data.begin();
				p_it!=p_data.end(); ++p_it, ++v_it )
		{
			(*p_it) += (*v_it) * sample_length;
		}
	}
	else
	{
		// check for pp sample length variable
		if( pt->variables.count(sample_length_var)==0 )
		{
			throw Exception( "Could not find sample length variable on primitive!" );
		}

		// get data from parameter
		FloatVectorData *s = pt->variableData<FloatVectorData>(sample_length_var);
		if ( !s )
		{
			throw Exception("Could not get scale length data from primitive!");
		}

		// check size against p_data
		const std::vector<float> &s_data = s->readable();
		if ( s_data.size()!=p_data.size() )
		{
			throw Exception("Position and scale length variables must be the same length!");
		}

		std::vector<V3f>::iterator p_it = p_data.begin();
		std::vector<V3f>::const_iterator v_it = v_data.begin();
		std::vector<float>::const_iterator s_it = s_data.begin();
		for ( p_it=p_data.begin(), v_it=v_data.begin(), s_it=s_data.begin();
				p_it!=p_data.end(); ++p_it, ++v_it, ++s_it )
		{
			(*p_it) += (*v_it) * (*s_it) * sample_length;
		}
	}
}
