//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/PointsPrimitiveEvaluator.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( PointsPrimitiveEvaluator );

PrimitiveEvaluator::Description<PointsPrimitiveEvaluator> PointsPrimitiveEvaluator::g_evaluatorDescription;

//////////////////////////////////////////////////////////////////////////
// Implementation of Result
//////////////////////////////////////////////////////////////////////////

PointsPrimitiveEvaluator::Result::Result( PointsPrimitiveEvaluator::ConstPtr evaluator )
	:	m_evaluator( evaluator )
{
}

template<class T>
const T &PointsPrimitiveEvaluator::Result::primVar( const PrimitiveVariable &primVar ) const
{
	switch( primVar.interpolation )
	{
		case PrimitiveVariable::Constant :
			{
				const TypedData<T> *d = static_cast<TypedData<T> *>( primVar.data.get() );
				return d->readable();
			}
			break;
		case PrimitiveVariable::Uniform :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return d[0];
			}
		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return d[m_pointIndex];
			}
		default :
			throw InvalidArgumentException( "PrimitiveVariable has invalid interpolation" );
	}
}

Imath::V3f PointsPrimitiveEvaluator::Result::point() const
{
	return primVar<V3f>( m_evaluator->m_p );
}

Imath::V3f PointsPrimitiveEvaluator::Result::normal() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V2f PointsPrimitiveEvaluator::Result::uv() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f PointsPrimitiveEvaluator::Result::uTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f PointsPrimitiveEvaluator::Result::vTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

size_t PointsPrimitiveEvaluator::Result::pointIndex() const
{
	return m_pointIndex;
}

Imath::V3f PointsPrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<V3f>( pv );
}

V2f PointsPrimitiveEvaluator::Result::vec2PrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<V2f>( pv );
}

float PointsPrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<float>( pv );
}

int PointsPrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<int>( pv );
}

const std::string &PointsPrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<std::string>( pv );
}

Imath::Color3f PointsPrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<Color3f>( pv );
}

half PointsPrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<half>( pv );
}

//////////////////////////////////////////////////////////////////////////
// Implementation of Evaluator
//////////////////////////////////////////////////////////////////////////

PointsPrimitiveEvaluator::PointsPrimitiveEvaluator( ConstPointsPrimitivePtr points )
	:	m_pointsPrimitive( points->copy() ), m_haveTree( false )
{
	PrimitiveVariableMap::iterator pIt = m_pointsPrimitive->variables.find( "P" );
	if( pIt==m_pointsPrimitive->variables.end() )
	{
		throw InvalidArgumentException( "No PrimitiveVariable named P on PointsPrimitive." );
	}
	m_p = pIt->second;
	if( !m_p.data || !m_p.data->isInstanceOf( V3fVectorData::staticTypeId() ) )
	{
		throw InvalidArgumentException( "PrimitiveVariable P is not of type V3fVectorData." );
	}
	m_pVector = &( boost::static_pointer_cast<const V3fVectorData>( m_p.data )->readable() );
}

PointsPrimitiveEvaluator::~PointsPrimitiveEvaluator()
{
}

PrimitiveEvaluatorPtr PointsPrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	ConstPointsPrimitivePtr points = runTimeCast<const PointsPrimitive>( primitive );
	assert( points );
	return new PointsPrimitiveEvaluator( points );
}

ConstPrimitivePtr PointsPrimitiveEvaluator::primitive() const
{
	return m_pointsPrimitive;
}

PrimitiveEvaluator::ResultPtr PointsPrimitiveEvaluator::createResult() const
{
	return new Result( this );
}

void PointsPrimitiveEvaluator::validateResult( PrimitiveEvaluator::Result *result ) const
{
	if( ! dynamic_cast<PointsPrimitiveEvaluator::Result *>( result ) )
	{
		throw InvalidArgumentException( "PointsPrimitiveEvaluator: Invalid result type" );
	}
}

float PointsPrimitiveEvaluator::surfaceArea() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

float PointsPrimitiveEvaluator::volume() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f PointsPrimitiveEvaluator::centerOfGravity() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool PointsPrimitiveEvaluator::closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const
{
	if( !m_pointsPrimitive->getNumPoints() )
	{
		return false;
	}

	// the cast isn't pretty but i think is the best of the alternatives. we want to delay building the tree until the first
	// closestPoint() query so people don't pay the overhead if they're just using other queries. the alternative to
	// the cast is to make the tree members mutable, but i'd rather keep them immutable so that the compiler tells us if
	// we do anything wrong during the queries.
	const_cast<PointsPrimitiveEvaluator *>( this )->buildTree();

	V3fTree::Iterator it = m_tree.nearestNeighbour( p );
	static_cast<Result *>( result )->m_pointIndex = it - m_pVector->begin();

	return true;
}

bool PointsPrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool PointsPrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
	PrimitiveEvaluator::Result *result, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

int PointsPrimitiveEvaluator::intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
	std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

void PointsPrimitiveEvaluator::buildTree()
{
	if( m_haveTree )
	{
		return;
	}

	std::lock_guard<TreeMutex> lock( m_treeMutex );
	if( m_haveTree )
	{
		// another thread may have built the tree while we waited for the mutex
		return;
	}

	m_tree.init( m_pVector->begin(), m_pVector->end() );
	m_haveTree = true;
}
