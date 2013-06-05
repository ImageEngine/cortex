#include "IECoreMaya/ProceduralHolderComponentBoundIterator.h"

#include "maya/MFnSingleIndexedComponent.h"

using namespace IECoreMaya;

ProceduralHolderComponentBoundIterator::ProceduralHolderComponentBoundIterator( void *userGeometry, MObjectArray &components )
	: 	MPxGeometryIterator( userGeometry, components ),
		m_proceduralHolder( (ProceduralHolder*) userGeometry ),
		m_idx( 0 ),
		m_components( components )
{
	computeNumComponents();
}

ProceduralHolderComponentBoundIterator::ProceduralHolderComponentBoundIterator( void *userGeometry, MObject &components )
	: 	MPxGeometryIterator( userGeometry, components ),
		m_proceduralHolder( (ProceduralHolder*) userGeometry ),
		m_idx( 0 )
{
	m_components.append( components );
	computeNumComponents();
}

void ProceduralHolderComponentBoundIterator::computeNumComponents()
{
	MStatus s;
	m_numComponents = 0;
	for( unsigned j=0; j < m_components.length(); ++j )
	{
		MFnSingleIndexedComponent fnCmp( m_components[j], &s );
		if( s )
		{
			m_numComponents += fnCmp.elementCount();
		}
	}
}

ProceduralHolderComponentBoundIterator::~ProceduralHolderComponentBoundIterator()
{
}

void ProceduralHolderComponentBoundIterator::reset()
{
	m_idx = 0;
}

MPoint ProceduralHolderComponentBoundIterator::point() const
{
	MPoint ret;
	
	Imath::Box3f bbox = m_proceduralHolder->componentBound( index() );
	
	switch( m_idx % 8 )
	{
		case 0: ret = MPoint( bbox.min[0], bbox.min[1], bbox.min[2] ); break;
		case 1: ret = MPoint( bbox.max[0], bbox.min[1], bbox.min[2] ); break;
		case 2: ret = MPoint( bbox.min[0], bbox.max[1], bbox.min[2] ); break;
		case 3: ret = MPoint( bbox.max[0], bbox.max[1], bbox.min[2] ); break;
		case 4: ret = MPoint( bbox.min[0], bbox.min[1], bbox.max[2] ); break;
		case 5: ret = MPoint( bbox.max[0], bbox.min[1], bbox.max[2] ); break;
		case 6: ret = MPoint( bbox.min[0], bbox.max[1], bbox.max[2] ); break;
		case 7: ret = MPoint( bbox.max[0], bbox.max[1], bbox.max[2] ); break;
		default: break;
	}
	
	return ret;
}

void ProceduralHolderComponentBoundIterator::setPoint( const MPoint & ) const
{
}

int ProceduralHolderComponentBoundIterator::iteratorCount() const
{
	// one for each bounding box corner
	return 8;
}

bool ProceduralHolderComponentBoundIterator::hasPoints() const
{
	return true;
}


bool ProceduralHolderComponentBoundIterator::isDone() const
{
	return m_idx >= m_numComponents * 8;
}

void ProceduralHolderComponentBoundIterator::next()
{
	++m_idx;
}

void ProceduralHolderComponentBoundIterator::component( MObject &component )
{
	component = MObject::kNullObj;
}

int ProceduralHolderComponentBoundIterator::setPointGetNext(MPoint &p)
{
	return m_idx + 1;
}

int ProceduralHolderComponentBoundIterator::index() const
{
	MStatus s;
	unsigned componentNum = 0;
	unsigned targetComponentNum = m_idx / 8;
	for( unsigned j=0; j < m_components.length(); ++j )
	{
		MFnSingleIndexedComponent fnCmp( m_components[j], &s );
		if( s )
		{
			if( componentNum + fnCmp.elementCount() > targetComponentNum )
			{
				return fnCmp.element( targetComponentNum - componentNum );
			}
			componentNum += fnCmp.elementCount();
		}
	}
	return 0;
}

bool ProceduralHolderComponentBoundIterator::hasNormals() const
{
	return false;
}

int ProceduralHolderComponentBoundIterator::indexUnsimplified() const
{
	return m_idx;
}
