//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathRandom.h"
#include "OpenEXR/ImathQuat.h"

#include "IECore/TriangleAlgo.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/MeshPrimitiveImplicitSurfaceFunction.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveImplicitSurfaceFunction::MeshPrimitiveImplicitSurfaceFunction(  MeshPrimitivePtr mesh ) : PrimitiveImplicitSurfaceFunction( mesh )
{
	PrimitiveVariableMap::const_iterator primVarIt = mesh->variables.find("P");

	// PrimitiveEvaluator in base class assures this
	assert( primVarIt != mesh->variables.end() );

	m_P = runTimeCast< const V3fVectorData > ( primVarIt->second.data );

	// PrimitiveEvaluator in base class should be assuring this
	assert( m_P );

	ConstIntVectorDataPtr vertexIds = mesh->vertexIds();
	ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();

#ifndef NDEBUG
	for (IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin(); it != verticesPerFace->readable().end(); ++it )
	{
		// PrimitiveEvaluator in base class assures this
		assert( *it == 3 );
	}
#endif

	/// Build vertex connectivity. We want to be able to quickly find the list of triangles connected to each vertex.
	typedef std::map< VertexIndex, std::set<TriangleIndex> > VertexConnectivity;
	VertexConnectivity vertexConnectivity;

	IntVectorData::ValueType::const_iterator it = vertexIds->readable().begin();
	int triangleIndex = 0;
	while ( it != vertexIds->readable().end() )
	{
		VertexIndex v0 = *it ++;
		VertexIndex v1 = *it ++;
		VertexIndex v2 = *it ++;

		vertexConnectivity[ v0 ].insert( triangleIndex );
		vertexConnectivity[ v1 ].insert( triangleIndex );
		vertexConnectivity[ v2 ].insert( triangleIndex );

		triangleIndex++;
	}

	assert( triangleIndex == (int)(verticesPerFace->readable().size()) );

	/// Calculate "Angle-weighted pseudo-normal" for each vertex. A description of this, and proof of its validity for use in signed distance functions
	/// can be found here: www.ann.jussieu.fr/~frey/papiers/PsNormTVCG.pdf
	m_vertexAngleWeightedNormals = new V3fVectorData( );
	m_vertexAngleWeightedNormals->writable().reserve( m_P->readable().size() );

	int numVertices = m_P->readable().size();

	for ( VertexIndex vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		Imath::V3f n( 0.0, 0.0, 0.0 );

		const std::set<TriangleIndex> &connectedTriangles = vertexConnectivity[ vertexIndex ];

		double angleTotal = 0.0;
		for (std::set<TriangleIndex>::const_iterator faceIt = connectedTriangles.begin(); faceIt != connectedTriangles.end(); ++faceIt)
		{

			/// Find the vertices associated with this triangle
			VertexIndex v0 = vertexIds->readable()[ (*faceIt) * 3 + 0 ];
			assert( vertexConnectivity[ v0 ].find( *faceIt ) != vertexConnectivity[ v0 ].end() );

			VertexIndex v1 = vertexIds->readable()[ (*faceIt) * 3 + 1 ];
			assert( vertexConnectivity[ v1 ].find( *faceIt ) != vertexConnectivity[ v1 ].end() );

			VertexIndex v2 = vertexIds->readable()[ (*faceIt) * 3 + 2 ];
			assert( vertexConnectivity[ v2 ].find( *faceIt ) != vertexConnectivity[ v2 ].end() );

			/// Find the two edges that go from the current vertex (i) to the other	two triangle vertices
			Imath::V3f e0, e1;
			if ( v2 == vertexIndex )
			{
				e0 = (m_P->readable()[ v1 ] - m_P->readable()[ v2 ]).normalized();
				e1 = (m_P->readable()[ v0 ] - m_P->readable()[ v2 ]).normalized();
			}
			else if ( v1 == vertexIndex )
			{
				e0 = (m_P->readable()[ v2 ] - m_P->readable()[ v1 ]).normalized();
				e1 = (m_P->readable()[ v0 ] - m_P->readable()[ v1 ]).normalized();
			}
			else
			{
				assert( v0 == vertexIndex );

				e0 = (m_P->readable()[ v1 ] - m_P->readable()[ v0 ]).normalized();
				e1 = (m_P->readable()[ v2 ] - m_P->readable()[ v0 ]).normalized();
			}

			double cosAngle = e0.dot( e1 );
			double angle = acos( cosAngle );
			assert( angle >= -Imath::limits<double>::epsilon() );
			angleTotal += angle;
			
			const Imath::V3f &p0 = m_P->readable()[ v0 ];
			const Imath::V3f &p1 = m_P->readable()[ v1 ];
			const Imath::V3f &p2 = m_P->readable()[ v2 ];
			n += triangleNormal( p0, p1, p2 ) * angle;
		}

		n.normalize();

		assert(m_vertexAngleWeightedNormals);

		m_vertexAngleWeightedNormals->writable().push_back( n );
	}

	assert( m_vertexAngleWeightedNormals->readable().size() == m_P->readable().size()  );

	/// Calculate edge connectivity. For any given pair of (connected) vertices we want to be able to find the faces connected to that edge. 	
	typedef std::map<Edge, std::vector<int> > EdgeConnectivity;
	
	EdgeConnectivity edgeConnectivity;
	
	it = vertexIds->readable().begin();
	triangleIndex = 0;
	while ( it != vertexIds->readable().end() )
	{
		VertexIndex v0 = *it ++;
		VertexIndex v1 = *it ++;
		VertexIndex v2 = *it ++;
		
		edgeConnectivity[ Edge(v0, v1) ].push_back( triangleIndex );
		edgeConnectivity[ Edge(v0, v2) ].push_back( triangleIndex );
		edgeConnectivity[ Edge(v1, v2) ].push_back( triangleIndex );
		
		/// Store the same edges going in the opposite direction, too. This doubles our (small) storage overhead but allows for faster lookups.
		edgeConnectivity[ Edge(v1, v0) ].push_back( triangleIndex );
		edgeConnectivity[ Edge(v2, v0) ].push_back( triangleIndex );
		edgeConnectivity[ Edge(v2, v1) ].push_back( triangleIndex );

		triangleIndex ++;
	}

	/// Calculate the average edge normals
	for (EdgeConnectivity::const_iterator it = edgeConnectivity.begin(); it != edgeConnectivity.end(); ++it)
	{
		if (it->second.size() > 2)
		{
			/// If there are more than 2 faces connected to any given edge then the mesh is non-manifold, which results in an exception.
			throw Exception("Non-manifold mesh given to MeshPrimitiveImplicitSurfaceFunction");
		}
		else if (it->second.size() == 1)
		{
			/// If there are less than 2 faces connected to any given edge then the mesh is not closed, which results in an exception.
			throw Exception("Mesh given to MeshPrimitiveImplicitSurfaceFunction is not closed");
		}
		else
		{
			assert( it->second.size() == 2 );
		}
		
		TriangleIndex triangle0 = it->second[0];
		TriangleIndex triangle1 = it->second[1];

		VertexIndex v00 = vertexIds->readable()[ triangle0 * 3 + 0 ];
		VertexIndex v01 = vertexIds->readable()[ triangle0 * 3 + 1 ];
		VertexIndex v02 = vertexIds->readable()[ triangle0 * 3 + 2 ];

		VertexIndex v10 = vertexIds->readable()[ triangle1 * 3 + 0 ];
		VertexIndex v11 = vertexIds->readable()[ triangle1 * 3 + 1 ];
		VertexIndex v12 = vertexIds->readable()[ triangle1 * 3 + 2 ];
		
		const Imath::V3f &p00 = m_P->readable()[ v00 ];
		const Imath::V3f &p01 = m_P->readable()[ v01 ];
		const Imath::V3f &p02 = m_P->readable()[ v02 ];
		
		const Imath::V3f &p10 = m_P->readable()[ v10 ];
		const Imath::V3f &p11 = m_P->readable()[ v11 ];
		const Imath::V3f &p12 = m_P->readable()[ v12 ];
		
		m_edgeAverageNormals[ it->first ] = ( triangleNormal( p00, p01, p02 ) + triangleNormal( p10, p11, p12 ) ) / 2.0f;
	}
	
}

MeshPrimitiveImplicitSurfaceFunction::~MeshPrimitiveImplicitSurfaceFunction()
{
}

MeshPrimitiveImplicitSurfaceFunction::Value MeshPrimitiveImplicitSurfaceFunction::operator()( const PrimitiveImplicitSurfaceFunction::Point &p )
{
	MeshPrimitiveEvaluator::ResultPtr result = boost::static_pointer_cast< MeshPrimitiveEvaluator::Result>( m_evaluator->createResult() );

	bool found = m_evaluator->closestPoint( p, result );

	if (found)
	{
		const Imath::V3f &bary = result->barycentricCoordinates();

		/// Is nearest feature an edge, or the triangle itself?
		
		int region = triangleBarycentricFeature( bary );
		
		if ( region == 0  )
		{
			assert( region == 0 );
			const Point &n = result->normal();
			PrimitiveImplicitSurfaceFunction::Value planeConstant = n.dot( result->point() );			
			PrimitiveImplicitSurfaceFunction::Value sign = n.dot( p ) - planeConstant;									
			return (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
		}
		else  if ( region % 2 == 1 )
		{		
			// Closest feature is an edge, so we need to use the average normal of the adjoining triangles
			
			const V3i &triangleVertexIds = result->vertexIds();
			Edge edge;

			assert( runTimeCast< MeshPrimitive >( m_primitive ) );
			ConstIntVectorDataPtr vertexIds = (runTimeCast< MeshPrimitive >( m_primitive ))->vertexIds();

			if ( region == 1 )
			{
				edge = Edge( triangleVertexIds[1], triangleVertexIds[2] );
			}
			else if ( region == 3 )
			{
				edge = Edge( triangleVertexIds[0], triangleVertexIds[2] );
			}
			else 
			{
				assert( region == 5 );
				edge = Edge( triangleVertexIds[0], triangleVertexIds[1] );
			}

			EdgeAverageNormals::const_iterator it = m_edgeAverageNormals.find( edge );
			assert (it != m_edgeAverageNormals.end() );
			
			const Point &n = it->second;
			PrimitiveImplicitSurfaceFunction::Value planeConstant = n.dot( result->point() );			
			PrimitiveImplicitSurfaceFunction::Value sign = n.dot( p ) - planeConstant;									
			return (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
		}
		else
		{
			// Closest feature is a vertex, so we need to use the angle weighted normal of the adjoining triangles		
			assert( region % 2 == 0 );

			const V3i &triangleVertexIds = result->vertexIds();
			
			int closestVertex = 1;
			if ( region == 2 )
			{
				closestVertex = 2;
			}
			else if ( region == 4 )
			{
				closestVertex = 0;
			}
			else 
			{
				assert( region == 6 );
				assert( closestVertex = 1 );
			}			
			
			assert( triangleVertexIds[ closestVertex ] < (int)(m_vertexAngleWeightedNormals->readable().size()) );
			
			const Point &n = m_vertexAngleWeightedNormals->readable()[ triangleVertexIds[ closestVertex ] ];
			PrimitiveImplicitSurfaceFunction::Value planeConstant = n.dot( result->point() );
			PrimitiveImplicitSurfaceFunction::Value sign = n.dot( p ) - planeConstant;									
			return (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
		}
	}
	else
	{
		return Imath::limits<MeshPrimitiveImplicitSurfaceFunction::Value>::min();
	}
}

MeshPrimitiveImplicitSurfaceFunction::Value MeshPrimitiveImplicitSurfaceFunction::getValue( const Point &p )
{
	return this->operator()(p);
}
