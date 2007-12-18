
#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/MayaMeshSignedDistanceFunction.h"

using namespace IECore;
using namespace IECoreMaya;


MayaMeshSignedDistanceFunction::MayaMeshSignedDistanceFunction( const MObject &obj, MSpace::Space space ) : m_space( space )
{
	MStatus s;
	m_fnMesh = new MFnMesh( obj, &s );

	if (!s)
	{
		throw StatusException(s);
	}

	assert( m_fnMesh );						
}

MayaMeshSignedDistanceFunction::~MayaMeshSignedDistanceFunction()		
{
	assert( m_fnMesh );

	delete m_fnMesh;
}

MayaMeshSignedDistanceFunction::Value MayaMeshSignedDistanceFunction::operator()( const MayaMeshSignedDistanceFunction::Point &p )
{
	MPoint testPoint(p.x, p.y, p.z);
	MPoint closestPoint;
	MVector closestNormal;
	assert( m_fnMesh );

	MStatus s = m_fnMesh->getClosestPointAndNormal(
		testPoint,
		closestPoint,
		closestNormal,
		m_space
	); 
	
	if (!s)
	{	
		throw StatusException( s );
	}

	return closestNormal.normal() * ( testPoint - closestPoint );
}

MayaMeshSignedDistanceFunction::Value MayaMeshSignedDistanceFunction::getValue( const Point &p )
{
	return this->operator()(p);
}
