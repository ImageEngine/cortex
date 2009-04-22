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

#include "IECore/PointNormalsOp.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/KDTree.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointNormalsOp );

static TypeId pointTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };

PointNormalsOp::PointNormalsOp()
	:	Op(
		staticTypeName(),
		"Calculates normals for a volume of points.",
		new ObjectParameter(
			"result",
			"Normals calculated for the points.",
			new V3fVectorData(),
			pointTypes
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
		"The number of neighbours to use in calculating points.",
		10,
		2
	);
	parameters()->addParameter( m_pointParameter );
	parameters()->addParameter( m_numNeighboursParameter );
}

PointNormalsOp::~PointNormalsOp()
{
}

ObjectParameterPtr PointNormalsOp::pointParameter()
{
	return m_pointParameter;
}

ConstObjectParameterPtr PointNormalsOp::pointParameter() const
{
	return m_pointParameter;
}

IntParameterPtr PointNormalsOp::numNeighboursParameter()
{
	return m_numNeighboursParameter;
}

ConstIntParameterPtr PointNormalsOp::numNeighboursParameter() const
{
	return m_numNeighboursParameter;
}

/// Calculates density at a point by finding the volume of a sphere holding numNeighbours. Doesn't bother
/// with any constant factors for the density (PI, 4/3, numNeighbours) as these are factored out in the use below anyway.
template<typename T>
static inline typename T::Point::BaseType density( const T tree, const typename T::Point &p, int numNeighbours, vector<typename T::Iterator> &neighbours )
{
	tree.nearestNNeighbours( p, numNeighbours, neighbours );
	typename T::Point::BaseType r = ((**neighbours.begin()) - p).length();
	return 1.0/(r*r*r);
}

/// This works by finding the gradient of a density function defined by the particles.
template<typename T>
static void normals( const vector<T> &points, int numNeighbours, vector<T> &result )
{
	typedef KDTree<typename vector<T>::const_iterator > Tree;
	typedef typename T::BaseType Real;
	
	Tree tree( points.begin(), points.end() );
	vector<typename Tree::Iterator> neighbours;
		
	result.resize( points.size() );

	for( unsigned int i=0; i<points.size(); i++ )
	{
		Real d = density( tree, points[i], numNeighbours, neighbours );
		float o = Real( 0.1 ) ; // should we scale offset for gradient by the radius of the neighbours sphere?
		Real dx = d - density( tree, points[i] + T( o, 0, 0 ), numNeighbours, neighbours );
		Real dy = d - density( tree, points[i] + T( 0, o, 0 ), numNeighbours, neighbours );
		Real dz = d - density( tree, points[i] + T( 0, 0, o ), numNeighbours, neighbours );
		result[i] = T( dx, dy, dz ).normalized();
	}
}

ObjectPtr PointNormalsOp::doOperation( ConstCompoundObjectPtr operands )
{
	const int numNeighbours = m_numNeighboursParameter->getNumericValue();
	
	ConstObjectPtr points = pointParameter()->getValue();
	ObjectPtr result = 0;
	switch( points->typeId() )
	{
		case V3fVectorDataTypeId :
			{
				V3fVectorDataPtr resultT = new V3fVectorData;
				normals<V3f>( boost::static_pointer_cast<const V3fVectorData>( points )->readable(), numNeighbours, resultT->writable() );
				result = resultT;
			}
			break;
		case V3dVectorDataTypeId :
			{
				V3dVectorDataPtr resultT = new V3dVectorData;
				normals<V3d>( boost::static_pointer_cast<const V3dVectorData>( points )->readable(), numNeighbours, resultT->writable() );
				result = resultT;
			}
			break;	
		default :
			// should never get here
			assert( 0 );
	}

	return result;
}
