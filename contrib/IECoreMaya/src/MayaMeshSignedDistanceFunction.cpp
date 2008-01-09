//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "maya/MPointArray.h"
#include "maya/MVectorArray.h"
#include "maya/MIntArray.h"

#include "OpenEXR/ImathVec.h"

#include "IECore/VectorOps.h"
#include "IECore/TriangleAlgo.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/MayaMeshSignedDistanceFunction.h"

using namespace IECore;
using namespace IECoreMaya;


MayaMeshSignedDistanceFunction::MayaMeshSignedDistanceFunction( const MObject &obj, MSpace::Space space ) : m_space( space )
{
	MStatus s;

	m_fnMesh = new MFnMesh( obj, &s );
	StatusException::throwIfError( s );

	assert( m_fnMesh );

	MObject o = obj;
	
	m_polyIt = new MItMeshPolygon( o, &s );
	
	for ( ; ! m_polyIt->isDone(); m_polyIt->next() )
	{
		if (m_polyIt->polygonVertexCount() != 3)
		{
			throw InvalidArgumentException( "Mesh is not triangulated" );
		}
	}

	m_polyIt->reset();
	
	m_edgeIt = new MItMeshEdge( o, &s );
	m_vertIt = new MItMeshVertex( o, &s );	

}

MayaMeshSignedDistanceFunction::~MayaMeshSignedDistanceFunction()
{
	assert( m_fnMesh );
	delete m_fnMesh;

	assert( m_vertIt );
	delete m_vertIt;

	assert( m_polyIt );
	delete m_polyIt;

	assert( m_edgeIt );
	delete m_edgeIt;
}

MayaMeshSignedDistanceFunction::Value MayaMeshSignedDistanceFunction::operator()( const MayaMeshSignedDistanceFunction::Point &p )
{
	MPoint testPoint(p.x, p.y, p.z);
	MPoint closestPoint;
	MVector closestNormal;
	assert( m_fnMesh );

	int closestPolygon;

	MStatus s = m_fnMesh->getClosestPointAndNormal(
	                testPoint,
	                closestPoint,
	                closestNormal,
	                m_space,
	                &closestPolygon
	            );
	StatusException::throwIfError( s );
	closestNormal.normalize();

	int prevIndex ;
	s = m_polyIt->setIndex( closestPolygon, prevIndex );
	assert( s );

	/// Compute signed distance from plane, which is defined by closestPoint and closestNormal
	double planeConstant = closestNormal * closestPoint ;
	double distance = (closestNormal * testPoint ) - planeConstant;
	
	if (distance > 0.0)
	{
		return distance;
	}

	MPointArray points;
	m_polyIt->getPoints( points );
	assert( points.length() == 3 );
	
	int feature = triangleClosestFeature( points[0], points[1], points[2], testPoint );

	if (feature == 0)
	{
		return distance;
	}
	
	/// As we're closest to an edge or a vertex we need to make sure we're inside all its connected faces, also.
	MIntArray connectedFaces;

	distance = fabs( distance );

	MIntArray vertexIds;
	m_polyIt->getVertices( vertexIds );
	assert( vertexIds.length() == 3 );

	MIntArray edges;
	m_polyIt->getEdges( edges );

	switch (feature)
	{
		case 1: //edge p1-p2
		{

			m_edgeIt->setIndex( edges[1], prevIndex );

#ifndef NDEBUG
			/// Make sure we got the edge we were expecting
			double edgeLength = 0.0;
			s = m_edgeIt->getLength( edgeLength );
			assert( s );
			assert( fabs( edgeLength - (points[1]-points[2]).length() ) < 0.01 );
#endif

			m_edgeIt->getConnectedFaces( connectedFaces );
		}
		break;
		case 2: // vert p2

			m_vertIt->setIndex( vertexIds[2], prevIndex );
			m_vertIt->getConnectedFaces( connectedFaces );

			break;
		case 3: //edge p0-p2
		{

			m_edgeIt->setIndex( edges[2], prevIndex );

#ifndef NDEBUG
			/// Make sure we got the edge we were expecting
			double edgeLength = 0.0;
			s = m_edgeIt->getLength( edgeLength );
			assert( s );
			assert( fabs( edgeLength - (points[0]-points[2]).length() ) < 0.01 );
#endif

			m_edgeIt->getConnectedFaces( connectedFaces );
		}
		break;
		case 4: //vert p0
			m_vertIt->setIndex( vertexIds[0], prevIndex );
			m_vertIt->getConnectedFaces( connectedFaces );
			break;
		case 5: //edge p0-p1
		{

			m_edgeIt->setIndex( edges[0], prevIndex );

#ifndef NDEBUG
			/// Make sure we got the edge we were expecting
			double edgeLength = 0.0;
			s = m_edgeIt->getLength( edgeLength );
			assert( s );
			assert( fabs( edgeLength - (points[0]-points[1]).length() ) < 0.01 );
#endif

			m_edgeIt->getConnectedFaces( connectedFaces );
		}
		break;
		case 6: // vert p1
			m_vertIt->setIndex( vertexIds[1], prevIndex );
			m_vertIt->getConnectedFaces( connectedFaces );
			break;
		default:
			assert( false );
	}

	for (unsigned f = 0; f < connectedFaces.length(); f++ )
	{
		s = m_polyIt->setIndex( connectedFaces[ f ], prevIndex );
		assert( s );

		assert( m_polyIt->hasValidTriangulation() );
		assert( m_polyIt->isPlanar() );

		int numTriangles = 0;
		s = m_polyIt->numTriangles( numTriangles );
		assert( s );
		assert( numTriangles == 1 );

		MPointArray triPoints;
		MIntArray triVerts;

		s = m_polyIt->getTriangle( 0, triPoints, triVerts, m_space );
		assert( s );

		MVector faceNormal = ((triPoints[1] - triPoints[0]) ^ (triPoints[2] - triPoints[0])).normal() ;

		/// Any point on the triangle if sufficient for computing the plane constant, as all the triangle points are coplanar
		double planeConstant = faceNormal * triPoints[0];
		double signedDistance = (faceNormal * testPoint) - planeConstant;

		if (signedDistance >= 0)
		{
			/// outside
			return distance;
		}
	}

	/// inside
	return -distance;
}

MayaMeshSignedDistanceFunction::Value MayaMeshSignedDistanceFunction::getValue( const Point &p )
{
	return this->operator()(p);
}
