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

#include <cassert>

#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/KDTree.h"
#include "IECore/Math.h"
#include "IECoreScene/PointDensitiesOp.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointDensitiesOp );

static IECore::TypeId pointTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };
static IECore::TypeId resultTypes[] = { FloatVectorDataTypeId, DoubleVectorDataTypeId, InvalidTypeId };

PointDensitiesOp::PointDensitiesOp()
	:	Op(
		"Calculates densities for a volume of points.",
		new ObjectParameter(
			"result",
			"Densities calculated for the points.",
			new FloatVectorData(),
			resultTypes
		)
	)
{
	m_pointParameter = new ObjectParameter(
		"points",
		"The points to calculate normals for.",
		new V3fVectorData(),
		pointTypes
	);
	m_numNeighboursParameter = new IntParameter(
		"numNeighbours",
		"The number of neighbours to use in estimating density.",
		10,
		2
	);
	m_multiplierParameter = new DoubleParameter(
		"multiplier",
		"A simple multiplier on the output densities.",
		1
	);
	parameters()->addParameter( m_pointParameter );
	parameters()->addParameter( m_numNeighboursParameter );
	parameters()->addParameter( m_multiplierParameter );
}

PointDensitiesOp::~PointDensitiesOp()
{
}

ObjectParameter * PointDensitiesOp::pointParameter()
{
	return m_pointParameter.get();
}

const ObjectParameter * PointDensitiesOp::pointParameter() const
{
	return m_pointParameter.get();
}

IntParameter * PointDensitiesOp::numNeighboursParameter()
{
	return m_numNeighboursParameter.get();
}

const IntParameter * PointDensitiesOp::numNeighboursParameter() const
{
	return m_numNeighboursParameter.get();
}

DoubleParameter * PointDensitiesOp::multiplierParameter()
{
	return m_multiplierParameter.get();
}

const DoubleParameter * PointDensitiesOp::multiplierParameter() const
{
	return m_multiplierParameter.get();
}

/// This works by finding the nearest n neighbours, and returning n divided by the volume of the sphere containing them.
template<typename T>
static void densities( const vector<Vec3<T> > &points, int numNeighbours, T multiplier, vector<T> &result )
{
	typedef KDTree<typename vector<Vec3<T> >::const_iterator > Tree;

	// factor constant parts of density calculation into the multiplier
	multiplier *= (T)numNeighbours / ((4.0/3.0) * M_PI);

	Tree tree( points.begin(), points.end() );
	vector<typename Tree::Neighbour> neighbours;

	result.resize( points.size() );
	for( unsigned int i=0; i<points.size(); i++ )
	{
		tree.nearestNNeighbours( points[i], numNeighbours, neighbours );
		T r = ((*(neighbours.rbegin()->point)) - points[i]).length();
		result[i] = multiplier / (r*r*r);
	}
}

/// \todo Support 2d point types?
/// \todo Threading?
ObjectPtr PointDensitiesOp::doOperation( const CompoundObject * operands )
{
	const int numNeighbours = m_numNeighboursParameter->getNumericValue();
	const double multiplier = m_multiplierParameter->getNumericValue();

	const Object * points = pointParameter()->getValue();
	ObjectPtr result = nullptr;
	switch( points->typeId() )
	{
		case V3fVectorDataTypeId :
			{
				FloatVectorDataPtr resultT = new FloatVectorData;
				densities<float>( static_cast<const V3fVectorData *>( points )->readable(), numNeighbours, multiplier, resultT->writable() );
				result = resultT;
			}
			break;
		case V3dVectorDataTypeId :
			{
				DoubleVectorDataPtr resultT = new DoubleVectorData;
				densities<double>( static_cast<const V3dVectorData *>( points )->readable(), numNeighbours, multiplier, resultT->writable() );
				result = resultT;
			}
			break;
		default :
			// should never get here
			assert( 0 );
	}

	return result;
}
