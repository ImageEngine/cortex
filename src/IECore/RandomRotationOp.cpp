//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/RandomRotationOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/ObjectParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/VectorTypedData.h"

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( RandomRotationOp );

static TypeId seedTypes[] = { FloatVectorDataTypeId, DoubleVectorDataTypeId, IntVectorDataTypeId, UIntVectorDataTypeId, InvalidTypeId };

RandomRotationOp::RandomRotationOp()
	:	Op(
		"Calculates a set of random vectors which rotate coherently over time.",
		new ObjectParameter(
			"result",
			"The random vectors.",
			new V3fVectorData(),
			V3fVectorData::staticTypeId()
		)
	)
{
	m_seedParameter = new ObjectParameter(
		"seeds",
		"The seeds to use for each vector.",
		new IntVectorData(),
		seedTypes
	);
	m_timeParameter = new FloatParameter(
		"time",
		"The time at which to calculate rotations.",
		0.0f
	);
	m_speedMinParameter = new FloatParameter(
		"speedMin",
		"The minimum rotation speed.",
		1.0f
	);
	m_speedMaxParameter = new FloatParameter(
		"speedMax",
		"The maximum rotation speed.",
		2.0f
	);

	parameters()->addParameter( m_seedParameter );
	parameters()->addParameter( m_timeParameter );
	parameters()->addParameter( m_speedMinParameter );
	parameters()->addParameter( m_speedMaxParameter );
}

RandomRotationOp::~RandomRotationOp()
{
}

ObjectParameter * RandomRotationOp::seedParameter()
{
	return m_seedParameter.get();
}

const ObjectParameter * RandomRotationOp::seedParameter() const
{
	return m_seedParameter.get();
}

FloatParameter * RandomRotationOp::timeParameter()
{
	return m_timeParameter.get();
}

const FloatParameter * RandomRotationOp::timeParameter() const
{
	return m_timeParameter.get();
}

FloatParameter * RandomRotationOp::speedMinParameter()
{
	return m_speedMinParameter.get();
}

const FloatParameter * RandomRotationOp::speedMinParameter() const
{
	return m_speedMinParameter.get();
}

FloatParameter * RandomRotationOp::speedMaxParameter()
{
	return m_speedMaxParameter.get();
}

const FloatParameter * RandomRotationOp::speedMaxParameter() const
{
	return m_speedMaxParameter.get();
}

template<typename T>
V3fVectorDataPtr doOp( const T * seed, float time, float minSpeed, float maxSpeed )
{
	V3fVectorDataPtr result = new V3fVectorData;
	vector<V3f> &v = result->writable();
	const typename T::ValueType &seedV = seed->readable();
	v.resize( seedV.size() );
	RandomRotationOp::generate( seedV.begin(), seedV.end(), time, minSpeed, maxSpeed, v.begin() );
	return result;
}

ObjectPtr RandomRotationOp::doOperation( const CompoundObject * operands )
{
	float time = m_timeParameter->getNumericValue();
	float minSpeed = m_speedMinParameter->getNumericValue();
	float maxSpeed = m_speedMaxParameter->getNumericValue();

	const Object * seed = m_seedParameter->getValue();
	switch( seed->typeId() )
	{
		case FloatVectorDataTypeId :
			return doOp<FloatVectorData>( static_cast<const FloatVectorData *>( seed ), time, minSpeed, maxSpeed );
		case DoubleVectorDataTypeId :
			return doOp<DoubleVectorData>( static_cast<const DoubleVectorData *>( seed ), time, minSpeed, maxSpeed );
		case IntVectorDataTypeId :
			return doOp<IntVectorData>( static_cast<const IntVectorData *>( seed ), time, minSpeed, maxSpeed );
		case UIntVectorDataTypeId :
			return doOp<UIntVectorData>( static_cast<const UIntVectorData *>( seed ), time, minSpeed, maxSpeed );
		default :
			return nullptr; // shouldn't get here
	}
	return nullptr;
}
