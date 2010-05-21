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

#include "IECore/PointVelocityDisplaceOp.h"

#include "IECore/PointsPrimitive.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( PointVelocityDisplaceOp );

//static TypeId pointAndVelocityTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };

PointVelocityDisplaceOp::PointVelocityDisplaceOp()
	:	ModifyOp(
		"Displaces points using their velocity (v) attribute.",
		new PointsPrimitiveParameter(
			"result",
			"The updated positions for points.",
			new PointsPrimitive()
			),
		new PointsPrimitiveParameter(
			"input",
			"The input points to displace.",
			new PointsPrimitive()
			)
		)
{

	m_sampleLengthParameter = new FloatParameter(
		"samplelength",
		"The sample time across which to displace P.",
		1.0 );

	parameters()->addParameter( m_sampleLengthParameter );
}

PointVelocityDisplaceOp::~PointVelocityDisplaceOp()
{
}

FloatParameter * PointVelocityDisplaceOp::sampleLengthParameter()
{
	return m_sampleLengthParameter;
}

const FloatParameter * PointVelocityDisplaceOp::sampleLengthParameter() const
{
	return m_sampleLengthParameter;
}

void PointVelocityDisplaceOp::modify( Object *input, const CompoundObject *operands )
{
	PointsPrimitive *pt = runTimeCast<PointsPrimitive>( input );
	float sample_length = m_sampleLengthParameter->getNumericValue();
	if ( !pt )
		return;

	// check for P and V
	if ( pt->variables.count("P")==0 || pt->variables.count("v")==0 )
		return;

	// access our P & V information
	V3fVectorDataPtr p = runTimeCast<V3fVectorData>(pt->variables["P"].data);
	V3fVectorDataPtr v = runTimeCast<V3fVectorData>(pt->variables["v"].data);
	if ( !p || !v )
		return;

	// check P and V are the same length
	if ( p->readable().size()!=v->readable().size() )
		return;
	
	// modify P by ( V * samplelen )
	std::vector<V3f> &p_data = p->writable();
	std::vector<V3f>::iterator p_it = p_data.begin();
	const std::vector<V3f> &v_data = v->readable();
	std::vector<V3f>::const_iterator v_it = v_data.begin();

	for ( p_it=p_data.begin(), v_it=v_data.begin();
		  p_it!=p_data.end(); ++p_it, ++v_it )
	{
		(*p_it) += (*v_it) * sample_length;
	}
}
