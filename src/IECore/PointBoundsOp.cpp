//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PointBoundsOp.h"
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

IE_CORE_DEFINERUNTIMETYPED( PointBoundsOp );

static TypeId pointAndVelocityTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };
static TypeId radiusTypes[] = { FloatVectorDataTypeId, DoubleVectorDataTypeId, InvalidTypeId };

PointBoundsOp::PointBoundsOp()
	:	Op(
		staticTypeName(),
		"Calculates the bounding box for a volume of points.",
		new Box3fParameter(
			"result",
			"The bounding box for the points.",
			Box3f()
		)
	)
{
	m_pointParameter = new ObjectParameter(
		"points",
		"The points to calculate normals for.",
		new V3fVectorData(),
		pointAndVelocityTypes
	);
	m_velocityParameter = new ObjectParameter(
		"velocities",
		"The velocities for the points.",
		new V3fVectorData,
		pointAndVelocityTypes
	);
	m_velocityMultiplierParameter = new FloatParameter(
		"velocityMultiplier",
		"A multiplier for the velocity values.",
		1.0f
	);
	m_radiusParameter = new ObjectParameter(
		"radii",
		"The radii for the points.",
		new FloatVectorData,
		radiusTypes
	);
	m_radiusMultiplierParameter = new FloatParameter(
		"radiusMultiplier",
		"A multiplier for the radius values.",
		1.0f
	);
	parameters()->addParameter( m_pointParameter );
	parameters()->addParameter( m_velocityParameter );
	parameters()->addParameter( m_velocityMultiplierParameter );
	parameters()->addParameter( m_radiusParameter );
	parameters()->addParameter( m_radiusMultiplierParameter );
}

PointBoundsOp::~PointBoundsOp()
{
}

ObjectParameterPtr PointBoundsOp::pointParameter()
{
	return m_pointParameter;
}

ConstObjectParameterPtr PointBoundsOp::pointParameter() const
{
	return m_pointParameter;
}

ObjectParameterPtr PointBoundsOp::radiusParameter()
{
	return m_radiusParameter;
}

ConstObjectParameterPtr PointBoundsOp::radiusParameter() const
{
	return m_radiusParameter;
}

ObjectParameterPtr PointBoundsOp::velocityParameter()
{
	return m_velocityParameter;
}

ConstObjectParameterPtr PointBoundsOp::velocityParameter() const
{
	return m_velocityParameter;
}

template<typename P, typename R, typename V>
static Box3fDataPtr bound3( typename P::ConstPtr pData, typename R::ConstPtr rData, float rMult, typename V::ConstPtr vData, float vMult )
{
	typedef typename P::ValueType::value_type Point;
	typedef typename V::ValueType::value_type Vector;
	typedef typename R::ValueType::value_type Radius;

	Box3f result;
	const typename P::ValueType &pVector = pData->readable();

	size_t vLength = 0;
	const Vector *v = 0;
	if( vData && vData->readable().size() )
	{
		vLength = vData->readable().size();
		v = &(vData->readable()[0]);
	}

	size_t rLength = 0;
	const Radius *r = 0;
	if( rData && rData->readable().size() )
	{
		rLength = rData->readable().size();
		r = &(rData->readable()[0]);
	}
	
	size_t i = 0;
	for( typename P::ValueType::const_iterator pIt = pVector.begin(); pIt!=pVector.end(); pIt++ )
	{
		Box3f b;
		b.extendBy( *pIt );
		if( v && i<vLength )
		{
			b.extendBy( *pIt + v[i] * vMult );
		}
		if( r && i<rLength )
		{
			Point rr( r[i] * rMult );
			b.min -= rr;
			b.max += rr;
			
		}
		result.extendBy( b );
		i++;
	}
	return new Box3fData( result );
}

template<typename P, typename R>
static Box3fDataPtr bound2( typename P::ConstPtr pData, typename R::ConstPtr rData, float rMult, ConstObjectPtr vData, float vMult )
{
	switch( vData->typeId() )
	{
		case V3fVectorDataTypeId :
			return bound3<P, R, V3fVectorData>( pData, rData, rMult, static_pointer_cast<const V3fVectorData>( vData ), vMult );
		case V3dVectorDataTypeId :
			return bound3<P, R, V3dVectorData>( pData, rData, rMult, static_pointer_cast<const V3dVectorData>( vData ), vMult );
		default :
			assert( 0 ); // parameter validation should prevent us getting here
	}
	return 0;
}

template<typename P>
static Box3fDataPtr bound1( typename P::ConstPtr pData, ConstObjectPtr rData, float rMult, ConstObjectPtr vData, float vMult )
{
	switch( rData->typeId() )
	{
		case FloatVectorDataTypeId :
			return bound2<P, FloatVectorData>( pData, static_pointer_cast<const FloatVectorData>( rData ), rMult, vData, vMult );
		case DoubleVectorDataTypeId :
			return bound2<P, DoubleVectorData>( pData, static_pointer_cast<const DoubleVectorData>( rData ), rMult, vData, vMult );
		default :
			assert( 0 ); // parameter validation should prevent us getting here
	}
	return 0;
}

ObjectPtr PointBoundsOp::doOperation( ConstCompoundObjectPtr operands )
{
	ConstObjectPtr p = m_pointParameter->getValue();
	ConstObjectPtr v = m_velocityParameter->getValue();
	ConstObjectPtr r = m_radiusParameter->getValue();
	float vm = m_velocityMultiplierParameter->getNumericValue();
	float rm = m_radiusMultiplierParameter->getNumericValue();
	switch( p->typeId() )
	{
		case V3fVectorDataTypeId :
			return bound1<V3fVectorData>( static_pointer_cast<const V3fVectorData>( p ), r, rm, v, vm );
		case V3dVectorDataTypeId :
			return bound1<V3dVectorData>( static_pointer_cast<const V3dVectorData>( p ), r, rm, v, vm );
		default :
			assert( 0 ); // parameter validation should prevent us getting here
	}
	return 0;
}
