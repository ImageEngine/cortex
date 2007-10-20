#include "IECore/PointDensitiesOp.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/KDTree.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;

static TypeId pointTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };
static TypeId resultTypes[] = { FloatVectorDataTypeId, DoubleVectorDataTypeId, InvalidTypeId };

PointDensitiesOp::PointDensitiesOp()
	:	Op(
		staticTypeName(),
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

ObjectParameterPtr PointDensitiesOp::pointParameter()
{
	return m_pointParameter;
}

ConstObjectParameterPtr PointDensitiesOp::pointParameter() const
{
	return m_pointParameter;
}

IntParameterPtr PointDensitiesOp::numNeighboursParameter()
{
	return m_numNeighboursParameter;
}

ConstIntParameterPtr PointDensitiesOp::numNeighboursParameter() const
{
	return m_numNeighboursParameter;
}

DoubleParameterPtr PointDensitiesOp::multiplierParameter()
{
	return m_multiplierParameter;
}

ConstDoubleParameterPtr PointDensitiesOp::multiplierParameter() const
{
	return m_multiplierParameter;
}

/// This works by finding the nearest n neighbours, and returning n divided by the volume of the sphere containing them.
template<typename T>
static void densities( const vector<Vec3<T> > &points, int numNeighbours, T multiplier, vector<T> &result )
{
	typedef KDTree<typename vector<Vec3<T> >::const_iterator > Tree;

	// factor constant parts of density calculation into the multiplier
	multiplier *= (T)numNeighbours / ((4.0/3.0) * M_PI);

	Tree tree( points.begin(), points.end() );
	vector<typename Tree::Iterator> neighbours;
		
	result.resize( points.size() );
	for( unsigned int i=0; i<points.size(); i++ )
	{
		tree.nearestNNeighbours( points[i], numNeighbours, neighbours );
		T r = ((**neighbours.begin()) - points[i]).length();
		result[i] = multiplier / (r*r*r);
	}
}

/// \todo Support 2d point types?
/// \todo Threading?
ObjectPtr PointDensitiesOp::doOperation( ConstCompoundObjectPtr operands )
{
	const int numNeighbours = m_numNeighboursParameter->getNumericValue();
	const double multiplier = m_multiplierParameter->getNumericValue();

	ConstObjectPtr points = pointParameter()->getValue();
	ObjectPtr result = 0;
	switch( points->typeId() )
	{
		case V3fVectorDataTypeId :
			{
				FloatVectorDataPtr resultT = new FloatVectorData;
				densities<float>( boost::static_pointer_cast<const V3fVectorData>( points )->readable(), numNeighbours, multiplier, resultT->writable() );
				result = resultT;
			}
			break;
		case V3dVectorDataTypeId :
			{
				DoubleVectorDataPtr resultT = new DoubleVectorData;
				densities<double>( boost::static_pointer_cast<const V3dVectorData>( points )->readable(), numNeighbours, multiplier, resultT->writable() );
				result = resultT;
			}
			break;	
		default :
			// should never get here
			assert( 0 );
	}

	return result;
}
