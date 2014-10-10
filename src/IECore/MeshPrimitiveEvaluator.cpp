//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathLineAlgo.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/BoxOps.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/Exception.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitiveEvaluator );

static PrimitiveEvaluator::Description< MeshPrimitiveEvaluator > g_registraar = PrimitiveEvaluator::Description< MeshPrimitiveEvaluator >();

MeshPrimitiveEvaluator::Result::Result()
{
}

V3f MeshPrimitiveEvaluator::Result::point() const
{
	return m_p;
}

V3f MeshPrimitiveEvaluator::Result::normal() const
{
	return m_n;
}

V2f MeshPrimitiveEvaluator::Result::uv() const
{
	return m_uv;
}

V3f MeshPrimitiveEvaluator::Result::uTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

V3f MeshPrimitiveEvaluator::Result::vTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

V3f MeshPrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< V3f >( pv );
}

float MeshPrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< float >( pv );
}

int MeshPrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< int >( pv );
}

const std::string &MeshPrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	const StringData *data = runTimeCast<StringData>( pv.data.get() );

	if (data)
	{
		return data->readable();
	}
	else
	{
		const StringVectorData *data = runTimeCast< StringVectorData >( pv.data.get() );

		if (data)
		{
			return data->readable()[0];
		}
	}

	throw InvalidArgumentException( "Could not retrieve primvar data for MeshPrimitiveEvaluator" );
}

Color3f MeshPrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< Color3f >( pv );
}

half MeshPrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< half >( pv );
}

template<typename T>
T MeshPrimitiveEvaluator::Result::getPrimVar( const PrimitiveVariable &pv ) const
{
	if ( pv.interpolation == PrimitiveVariable::Constant )
	{
		typedef TypedData<T> Data;

		const Data *data = runTimeCast< Data >( pv.data.get() );

		if (data)
		{
			return data->readable();
		}
	}

	typedef TypedData< std::vector<T> > VectorData;
	const VectorData *data = runTimeCast< VectorData >( pv.data.get() );

	if (!data)
	{
		throw InvalidArgumentException( "Could not retrieve primvar data for MeshPrimitiveEvaluator" );
	}

	switch ( pv.interpolation )
	{
		case PrimitiveVariable::Constant :
			assert( data->readable().size() == 1 );

			return data->readable()[0];

		case PrimitiveVariable::Uniform :
			assert( m_triangleIdx < data->readable().size() );

			return data->readable()[ m_triangleIdx ];

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
			assert( m_vertexIds[0] < (int)data->readable().size() );
			assert( m_vertexIds[1] < (int)data->readable().size() );
			assert( m_vertexIds[2] < (int)data->readable().size() );

			return static_cast<T>( data->readable()[ m_vertexIds[0] ] * m_bary[0] + data->readable()[ m_vertexIds[1] ] * m_bary[1] + data->readable()[ m_vertexIds[2] ] * m_bary[2] );

		case PrimitiveVariable::FaceVarying:

			assert( (m_triangleIdx * 3) + 0 < data->readable().size() );
			assert( (m_triangleIdx * 3) + 1 < data->readable().size() );
			assert( (m_triangleIdx * 3) + 2 < data->readable().size() );

			return static_cast<T>(
				  data->readable()[ (m_triangleIdx * 3) + 0 ] * m_bary[0]
				+ data->readable()[ (m_triangleIdx * 3) + 1 ] * m_bary[1]
				+ data->readable()[ (m_triangleIdx * 3) + 2 ] * m_bary[2]
			);

		default :
			/// Unimplemented primvar interpolation
			assert( false );
			return T();

	}
}

unsigned int MeshPrimitiveEvaluator::Result::triangleIndex() const
{
	return m_triangleIdx;
}

const V3f &MeshPrimitiveEvaluator::Result::barycentricCoordinates() const
{
	return m_bary;
}

const V3i &MeshPrimitiveEvaluator::Result::vertexIds() const
{
	return m_vertexIds;
}

MeshPrimitiveEvaluator::MeshPrimitiveEvaluator( ConstMeshPrimitivePtr mesh ) : m_uvTree(0), m_haveMassProperties( false ), m_haveSurfaceArea( false ), m_haveAverageNormals( false )
{
	if (! mesh )
	{
		throw InvalidArgumentException( "No mesh given to MeshPrimitiveEvaluator");
	}

	if (! mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to MeshPrimitiveEvaluator");
	}

	m_mesh = mesh->copy();


	PrimitiveVariableMap::const_iterator primVarIt = m_mesh->variables.find("P");
	if ( primVarIt == m_mesh->variables.end() )
	{
		throw InvalidArgumentException( "Mesh given to MeshPrimitiveEvaluator has no \"P\"");
	}

	m_verts = runTimeCast< const V3fVectorData > ( primVarIt->second.data );

	if (! m_verts )
	{
		throw InvalidArgumentException( "Mesh given to MeshPrimitiveEvaluator has no \"P\" primvar of type V3fVectorData");
	}

	m_meshVertexIds = &(m_mesh->vertexIds()->readable());

	m_u.interpolation = PrimitiveVariable::Invalid;
	primVarIt = m_mesh->variables.find("s");
	if ( primVarIt != m_mesh->variables.end() )
	{
		m_u = primVarIt->second;
	}

	m_v.interpolation = PrimitiveVariable::Invalid;
	primVarIt = m_mesh->variables.find("t");
	if ( primVarIt != m_mesh->variables.end() )
	{
		m_v = primVarIt->second;
	}

	const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
	m_triangles.reserve( verticesPerFace.size() );
	m_uvTriangles.reserve( verticesPerFace.size() );
	unsigned int triangleIdx = 0;
	IntVectorData::ValueType::const_iterator vertexIdIt = m_meshVertexIds->begin();
	for ( IntVectorData::ValueType::const_iterator it = verticesPerFace.begin();
		it != verticesPerFace.end(); ++it, ++triangleIdx)
	{

		if (*it != 3 )
		{
			throw InvalidArgumentException( "Non-triangular mesh given to MeshPrimitiveEvaluator");
		}

		Imath::V3i triangleVertexIds;

		triangleVertexIds[0] = *vertexIdIt++;
		assert( triangleVertexIds[0] < (int)( m_verts->readable().size() ) );
		triangleVertexIds[1] = *vertexIdIt++;
		assert( triangleVertexIds[1] < (int)( m_verts->readable().size() ) );
		triangleVertexIds[2] = *vertexIdIt++;
		assert( triangleVertexIds[2] < (int)( m_verts->readable().size() ) );

		const V3f &p0 = m_verts->readable()[ triangleVertexIds[0] ];
		const V3f &p1 = m_verts->readable()[ triangleVertexIds[1] ];
		const V3f &p2 = m_verts->readable()[ triangleVertexIds[2] ];

		Box3f bound( p0 );
		bound.extendBy( p1 );
		bound.extendBy( p2 );

		m_triangles.push_back( bound );

		if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
		{
			Imath::V2f uv[3];
			triangleUVs( triangleIdx, triangleVertexIds, uv );

			Box2f uvBound( uv[0] );
			uvBound.extendBy( uv[1] );
			uvBound.extendBy( uv[2] );

			m_uvTriangles.push_back( uvBound );
		}
	}
	
	m_tree = new TriangleBoundTree( m_triangles.begin(), m_triangles.end() );

	if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
	{
		m_uvTree = new UVBoundTree( m_uvTriangles.begin(), m_uvTriangles.end() );
	}
	else
	{
		m_uvTree = 0;
	}
}

PrimitiveEvaluatorPtr MeshPrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	ConstMeshPrimitivePtr mesh = runTimeCast< const MeshPrimitive >( primitive );
	assert ( mesh );

	return new MeshPrimitiveEvaluator( mesh );
}

MeshPrimitiveEvaluator::~MeshPrimitiveEvaluator()
{
	assert( m_tree );

	delete m_tree;
	m_tree = 0;

	delete m_uvTree;
	m_uvTree = 0;
}

ConstPrimitivePtr MeshPrimitiveEvaluator::primitive() const
{
	return m_mesh;
}

MeshPrimitive::ConstPtr MeshPrimitiveEvaluator::mesh() const
{
	return m_mesh;
}

float MeshPrimitiveEvaluator::volume() const
{
	if ( !m_haveMassProperties )
	{
		calculateMassProperties();
	}

	return m_volume;
}

Imath::V3f MeshPrimitiveEvaluator::centerOfGravity() const
{
	if ( !m_haveMassProperties )
	{
		calculateMassProperties();
	}

	return m_centerOfGravity;
}

float MeshPrimitiveEvaluator::surfaceArea() const
{
	if ( !m_haveSurfaceArea )
	{
		m_surfaceArea = 0.0f;
		IntVectorData::ValueType::const_iterator vertexIdIt = m_meshVertexIds->begin();

		for ( IntVectorData::ValueType::const_iterator it = m_mesh->verticesPerFace()->readable().begin();
			it != m_mesh->verticesPerFace()->readable().end(); ++it )
		{
			assert ( *it == 3 );

			const V3f &p0 = m_verts->readable()[ *vertexIdIt++ ];
			const V3f &p1 = m_verts->readable()[ *vertexIdIt++ ];
			const V3f &p2 = m_verts->readable()[ *vertexIdIt++ ];

			m_surfaceArea += triangleArea( p0, p1, p2 );
		}

		m_haveSurfaceArea = true;
	}

	return m_surfaceArea;
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license
void MeshPrimitiveEvaluator::calculateMassProperties() const
{
	assert( !m_haveMassProperties );

	double integral[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	IntVectorData::ValueType::const_iterator vertexIdIt = m_meshVertexIds->begin();

	for ( IntVectorData::ValueType::const_iterator it = m_mesh->verticesPerFace()->readable().begin();
		it != m_mesh->verticesPerFace()->readable().end(); ++it )
	{
		assert ( *it == 3 );

		V3i triangleVertexIds;

		triangleVertexIds[0] = *vertexIdIt++;
		assert( triangleVertexIds[0] < (int)( m_verts->readable().size() ) );
		triangleVertexIds[1] = *vertexIdIt++;
		assert( triangleVertexIds[1] < (int)( m_verts->readable().size() ) );
		triangleVertexIds[2] = *vertexIdIt++;
		assert( triangleVertexIds[2] < (int)( m_verts->readable().size() ) );

		const Imath::V3f &p0 = m_verts->readable()[ triangleVertexIds[0] ];
		const Imath::V3f &p1 = m_verts->readable()[ triangleVertexIds[1] ];
		const Imath::V3f &p2 = m_verts->readable()[ triangleVertexIds[2] ];

		/// Winding order has to be correct here
		V3f n = ( p1 - p0 ).cross( p2 - p0 );

		V3f f1, f2, f3, g0, g1, g2;
		for (int dim = 0; dim < 3; dim++)
		{
			double tmp0, tmp1, tmp2;

			tmp0 = p0[dim] + p1[dim];
			f1[dim] = tmp0 + p2[dim];
			tmp1 = p0[dim] * p0[dim];
			tmp2 = tmp1 + p1[dim] * tmp0;
			f2[dim] = tmp2 + p2[dim] * f1[dim];
			f3[dim] = p0[dim]*tmp1 + p1[dim]*tmp2 + p2[dim]*f2[dim];
			g0[dim] = f2[dim] + p0[dim] * (f1[dim] + p0[dim]);
			g1[dim] = f2[dim] + p1[dim] * (f1[dim] + p1[dim]);
			g2[dim] = f2[dim] + p2[dim] * (f1[dim] + p2[dim]);
		}

		integral[0] += n.x*f1.x;
		integral[1] += n.x*f2.x;
		integral[2] += n.y*f2.y;
		integral[3] += n.z*f2.z;
		integral[4] += n.x*f3.x;
		integral[5] += n.y*f3.y;
		integral[6] += n.z*f3.z;
		integral[7] += n.x*(p0.y*g0.x + p1.y*g1.x + p2.y*g2.x);
		integral[8] += n.y*(p0.z*g0.y + p1.z*g1.y + p2.z*g2.y);
		integral[9] += n.z*(p0.x*g0.z + p1.x*g1.z + p2.x*g2.z);
	}

	integral[0] /= 6.0;
	integral[1] /= 24.0;
	integral[2] /= 24.0;
	integral[3] /= 24.0;
	integral[4] /= 60.0;
	integral[5] /= 60.0;
	integral[6] /= 60.0;
	integral[7] /= 120.0;
	integral[8] /= 120.0;
	integral[9] /= 120.0;

	m_volume = integral[0];
	m_centerOfGravity = V3f( integral[1], integral[2], integral[3] ) / integral[0];
	m_inertia[0][0] = integral[5] + integral[6];
	m_inertia[0][1] = -integral[7];
	m_inertia[0][2] = -integral[9];
	m_inertia[1][0] = m_inertia[0][1];
	m_inertia[1][1] = integral[4] + integral[6];
	m_inertia[1][2] = -integral[8];
	m_inertia[2][0] = m_inertia[0][2];
	m_inertia[2][1] = m_inertia[1][2];
	m_inertia[2][2] = integral[4] + integral[5];

	m_haveMassProperties = true;
}

void MeshPrimitiveEvaluator::calculateAverageNormals() const
{
	assert( m_mesh );
	
	if( m_haveAverageNormals )
	{
		return;
	}
	
	NormalsMutex::scoped_lock lock( m_normalsMutex );
	if( m_haveAverageNormals )
	{
		// another thread may have calculated the normals while we waited for the mutex
		return;
	}
	
	ConstIntVectorDataPtr verticesPerFace = m_mesh->verticesPerFace();

#ifndef NDEBUG
	for (IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin(); it != verticesPerFace->readable().end(); ++it )
	{
		assert( *it == 3 );
	}
#endif

	/// Build vertex connectivity. We want to be able to quickly find the list of triangles connected to each vertex.
	typedef std::map< VertexIndex, std::set<TriangleIndex> > VertexConnectivity;
	VertexConnectivity vertexConnectivity;

	IntVectorData::ValueType::const_iterator it = m_meshVertexIds->begin();
	int triangleIndex = 0;
	while ( it != m_meshVertexIds->end() )
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
	m_vertexAngleWeightedNormals->writable().reserve( m_verts->readable().size() );

	int numVertices = m_verts->readable().size();

	for ( VertexIndex vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
	{
		Imath::V3f n( 0.0, 0.0, 0.0 );

		const std::set<TriangleIndex> &connectedTriangles = vertexConnectivity[ vertexIndex ];

		double angleTotal = 0.0;
		for (std::set<TriangleIndex>::const_iterator faceIt = connectedTriangles.begin(); faceIt != connectedTriangles.end(); ++faceIt)
		{

			/// Find the vertices associated with this triangle
			VertexIndex v0 = (*m_meshVertexIds)[ (*faceIt) * 3 + 0 ];
			assert( vertexConnectivity[ v0 ].find( *faceIt ) != vertexConnectivity[ v0 ].end() );

			VertexIndex v1 = (*m_meshVertexIds)[ (*faceIt) * 3 + 1 ];
			assert( vertexConnectivity[ v1 ].find( *faceIt ) != vertexConnectivity[ v1 ].end() );

			VertexIndex v2 = (*m_meshVertexIds)[ (*faceIt) * 3 + 2 ];
			assert( vertexConnectivity[ v2 ].find( *faceIt ) != vertexConnectivity[ v2 ].end() );

			/// Find the two edges that go from the current vertex (i) to the other	two triangle vertices
			Imath::V3f e0, e1;
			if ( v2 == vertexIndex )
			{
				e0 = (m_verts->readable()[ v1 ] - m_verts->readable()[ v2 ]).normalized();
				e1 = (m_verts->readable()[ v0 ] - m_verts->readable()[ v2 ]).normalized();
			}
			else if ( v1 == vertexIndex )
			{
				e0 = (m_verts->readable()[ v2 ] - m_verts->readable()[ v1 ]).normalized();
				e1 = (m_verts->readable()[ v0 ] - m_verts->readable()[ v1 ]).normalized();
			}
			else
			{
				assert( v0 == vertexIndex );

				e0 = (m_verts->readable()[ v1 ] - m_verts->readable()[ v0 ]).normalized();
				e1 = (m_verts->readable()[ v2 ] - m_verts->readable()[ v0 ]).normalized();
			}

			double cosAngle = e0.dot( e1 );
			double angle = acos( cosAngle );
			assert( angle >= -Imath::limits<double>::epsilon() );
			angleTotal += angle;

			const Imath::V3f &p0 = m_verts->readable()[ v0 ];
			const Imath::V3f &p1 = m_verts->readable()[ v1 ];
			const Imath::V3f &p2 = m_verts->readable()[ v2 ];
			n += triangleNormal( p0, p1, p2 ) * angle;
		}

		n.normalize();

		assert(m_vertexAngleWeightedNormals);

		m_vertexAngleWeightedNormals->writable().push_back( n );
	}

	assert( m_vertexAngleWeightedNormals->readable().size() == m_verts->readable().size()  );

	/// Calculate edge connectivity. For any given pair of (connected) vertices we want to be able to find the faces connected to that edge.
	typedef std::map<Edge, std::vector<int> > EdgeConnectivity;

	EdgeConnectivity edgeConnectivity;

	it = m_meshVertexIds->begin();
	triangleIndex = 0;
	while ( it != m_meshVertexIds->end() )
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

		VertexIndex v00 = (*m_meshVertexIds)[ triangle0 * 3 + 0 ];
		VertexIndex v01 = (*m_meshVertexIds)[ triangle0 * 3 + 1 ];
		VertexIndex v02 = (*m_meshVertexIds)[ triangle0 * 3 + 2 ];

		VertexIndex v10 = (*m_meshVertexIds)[ triangle1 * 3 + 0 ];
		VertexIndex v11 = (*m_meshVertexIds)[ triangle1 * 3 + 1 ];
		VertexIndex v12 = (*m_meshVertexIds)[ triangle1 * 3 + 2 ];

		const Imath::V3f &p00 = m_verts->readable()[ v00 ];
		const Imath::V3f &p01 = m_verts->readable()[ v01 ];
		const Imath::V3f &p02 = m_verts->readable()[ v02 ];

		const Imath::V3f &p10 = m_verts->readable()[ v10 ];
		const Imath::V3f &p11 = m_verts->readable()[ v11 ];
		const Imath::V3f &p12 = m_verts->readable()[ v12 ];

		m_edgeAverageNormals[ it->first ] = ( triangleNormal( p00, p01, p02 ) + triangleNormal( p10, p11, p12 ) ) / 2.0f;
	}

	m_haveAverageNormals = true;
}

bool MeshPrimitiveEvaluator::signedDistance( const Imath::V3f &p, float &distance ) const
{
	distance = 0.0f;

	if ( !m_haveAverageNormals )
	{
		calculateAverageNormals();
	}

	ResultPtr result = new Result();

	bool found = closestPoint( p, result.get() );

	if (found)
	{
		const Imath::V3f &bary = result->barycentricCoordinates();

		/// Is nearest feature an edge, or the triangle itself?

		int region = triangleBarycentricFeature( bary );

		if ( region == 0  )
		{
			assert( region == 0 );
			const Imath::V3f &n = result->normal();
			float planeConstant = n.dot( result->point() );
			float sign = n.dot( p ) - planeConstant;
			distance = (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
			return true;
		}
		else  if ( region % 2 == 1 )
		{
			// Closest feature is an edge, so we need to use the average normal of the adjoining triangles

			const V3i &triangleVertexIds = result->vertexIds();
			Edge edge;

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

			const Imath::V3f &n = it->second;
			float planeConstant = n.dot( result->point() );
			float sign = n.dot( p ) - planeConstant;
			distance = (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
			return true;
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

			const V3f &n = m_vertexAngleWeightedNormals->readable()[ triangleVertexIds[ closestVertex ] ];
			float planeConstant = n.dot( result->point() );
			float sign = n.dot( p ) - planeConstant;
			distance = (result->point() - p ).length() * (sign < Imath::limits<float>::epsilon() ? -1.0 : 1.0 );
			return true;
		}
	}
	else
	{
		return false;
	}
}

PrimitiveEvaluator::ResultPtr MeshPrimitiveEvaluator::createResult() const
{
      return new Result();
}

void MeshPrimitiveEvaluator::validateResult( PrimitiveEvaluator::Result *result ) const
{
	if (! dynamic_cast<MeshPrimitiveEvaluator::Result *>( result ) )
	{
		throw InvalidArgumentException("MeshPrimitiveEvaluator: Invalid PrimitiveEvaulator result type");
	}
}

bool MeshPrimitiveEvaluator::closestPoint( const V3f &p, PrimitiveEvaluator::Result *result ) const
{
	assert( dynamic_cast<Result *>( result ) );

	if ( m_triangles.size() == 0)
	{
		return false;
	}

	assert( m_tree );

	Result *mr = static_cast<Result *>( result );

	float maxDistSqrd = limits<float>::max();

	closestPointWalk( m_tree->rootIndex(), p, maxDistSqrd, mr );

	return true;
}

bool MeshPrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const
{
	assert( dynamic_cast<Result *>( result ) );

	if ( ! m_uvTriangles.size() )
	{
		throw Exception("No uvs available for pointAtUV");
	}

	assert( m_uvTree );
	Result *mr = static_cast<Result *>( result );

	return pointAtUVWalk( m_uvTree->rootIndex(), uv, mr );
}

bool MeshPrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
	PrimitiveEvaluator::Result *result, float maxDistance ) const
{
	assert( dynamic_cast<Result *>( result ) );

	if ( m_triangles.size() == 0)
	{
		return false;
	}

	assert( m_tree );

	Result *mr = static_cast<Result *>( result );

	float maxDistSqrd = maxDistance * maxDistance;

	Imath::Line3f ray;
	ray.pos = origin;
	ray.dir = direction.normalized();

	bool hit = false;

	intersectionPointWalk( m_tree->rootIndex(), ray, maxDistSqrd, mr, hit );
	return hit;
}

int MeshPrimitiveEvaluator::intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
	std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{
	results.clear();

	if ( m_triangles.size() == 0)
	{
		return 0;
	}

	assert( m_tree );

	float maxDistSqrd = maxDistance * maxDistance;

	Imath::Line3f ray;
	ray.pos = origin;
	ray.dir = direction.normalized();

	intersectionPointsWalk( m_tree->rootIndex(), ray, maxDistSqrd, results );

	return results.size();
}

bool MeshPrimitiveEvaluator::barycentricPosition( unsigned int triangleIndex, const Imath::V3f &barycentricCoordinates, PrimitiveEvaluator::Result *result ) const
{
	if( triangleIndex > m_triangles.size() )
	{
		return false;
	}

	Result *r = static_cast<Result *>( result );
	
	r->m_triangleIdx = triangleIndex;
	r->m_bary = barycentricCoordinates;
	
	size_t vertIdOffset = triangleIndex * 3;
	r->m_vertexIds = Imath::V3i( (*m_meshVertexIds)[vertIdOffset], (*m_meshVertexIds)[vertIdOffset+1], (*m_meshVertexIds)[vertIdOffset+2] );

	const Imath::V3f &p0 = m_verts->readable()[ r->m_vertexIds[0] ];
	const Imath::V3f &p1 = m_verts->readable()[ r->m_vertexIds[1] ];
	const Imath::V3f &p2 = m_verts->readable()[ r->m_vertexIds[2] ];

	r->m_p = trianglePoint( p0, p1, p2, r->m_bary );

	r->m_n = triangleNormal( p0, p1, p2 );

	if( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
	{
		r->m_uv = V2f(
			r->floatPrimVar( m_u ),
			r->floatPrimVar( m_v )
		);
	}
				
	return true;
}

void MeshPrimitiveEvaluator::closestPointWalk( TriangleBoundTree::NodeIndex nodeIndex, const V3f &p, float &closestDistanceSqrd, Result *result ) const
{
	assert( m_tree );

	const TriangleBoundTree::Node &node = m_tree->node( nodeIndex );
	if( node.isLeaf() )
	{
		TriangleBoundTree::Iterator *permLast = node.permLast();
		for( TriangleBoundTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			size_t triangleIndex = *perm - m_triangles.begin(); // triangle index is just the distance of the triangle from the beginning of the vector
			size_t vertIdOffset = triangleIndex * 3;
			Imath::V3i vertexIds( (*m_meshVertexIds)[vertIdOffset], (*m_meshVertexIds)[vertIdOffset+1], (*m_meshVertexIds)[vertIdOffset+2] );
			
			assert( vertexIds[0] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[1] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[2] < (int)( m_verts->readable().size() ) );

			V3f bary;
			float dSqrd = triangleClosestBarycentric(
				m_verts->readable()[vertexIds[0]],
				m_verts->readable()[vertexIds[1]],
				m_verts->readable()[vertexIds[2]],
				p,
				bary );

			if (dSqrd < closestDistanceSqrd)
			{
				closestDistanceSqrd = dSqrd;

				result->m_bary = bary;
				result->m_vertexIds = vertexIds;
				result->m_triangleIdx = triangleIndex;

				if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
				{
					result->m_uv = V2f(
						result->floatPrimVar( m_u ),
						result->floatPrimVar( m_v )
					);
				}

				const Imath::V3f &p0 = m_verts->readable()[vertexIds[0]];
				const Imath::V3f &p1 = m_verts->readable()[vertexIds[1]];
				const Imath::V3f &p2 = m_verts->readable()[vertexIds[2]];

				result->m_p = trianglePoint( p0, p1, p2, result->m_bary );

				result->m_n = triangleNormal( p0, p1, p2 );
			}
		}
	}
	else
	{
		/// Descend into the closest box first

		float dHigh = vecDistance(
			closestPointInBox( p, m_tree->node( TriangleBoundTree::highChildIndex( nodeIndex ) ).bound() ),
			p
		);

		float dLow = vecDistance(
			closestPointInBox( p, m_tree->node( TriangleBoundTree::lowChildIndex( nodeIndex ) ).bound() ),
			p
		);

		TriangleBoundTree::NodeIndex firstChild, secondChild;

		float dSecond;

		if (dHigh < dLow)
		{
			firstChild = TriangleBoundTree::highChildIndex( nodeIndex );
			secondChild = TriangleBoundTree::lowChildIndex( nodeIndex );
			dSecond = dLow;
		}
		else
		{
			firstChild = TriangleBoundTree::lowChildIndex( nodeIndex );
			secondChild = TriangleBoundTree::highChildIndex( nodeIndex );
			dSecond = dHigh;
		}

		closestPointWalk( firstChild, p, closestDistanceSqrd, result );

		if (dSecond * dSecond < closestDistanceSqrd )
		{
			closestPointWalk( secondChild, p, closestDistanceSqrd, result );
		}
	}
}

bool MeshPrimitiveEvaluator::pointAtUVWalk( UVBoundTree::NodeIndex nodeIndex, const Imath::V2f &targetUV, Result *result ) const
{
	assert( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid);
	assert( m_uvTree );

	const UVBoundTree::Node &node = m_uvTree->node( nodeIndex );
	
	if( !node.bound().intersects( targetUV ) )
	{
		return false;
	}

	if( node.isLeaf() )
	{
		
		UVBoundTree::Iterator *permLast = node.permLast();
		for( UVBoundTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			size_t triangleIndex = *perm - m_uvTriangles.begin(); // triangle index is just the distance of the triangle from the beginning of the vector
			size_t vertIdOffset = triangleIndex * 3;
			Imath::V3i vertexIds( (*m_meshVertexIds)[vertIdOffset], (*m_meshVertexIds)[vertIdOffset+1], (*m_meshVertexIds)[vertIdOffset+2] );

			Imath::V2f uv[3];
			triangleUVs( triangleIndex, vertexIds, uv );

			if( triangleContainsPoint( uv[0], uv[1], uv[2], targetUV, result->m_bary ) )
			{
				result->m_vertexIds = vertexIds;

				result->m_triangleIdx = triangleIndex;

				result->m_uv = targetUV;

				assert( result->m_vertexIds[0] < (int)( m_verts->readable().size() ) );
				assert( result->m_vertexIds[1] < (int)( m_verts->readable().size() ) );
				assert( result->m_vertexIds[2] < (int)( m_verts->readable().size() ) );

				const Imath::V3f &p0 = m_verts->readable()[ result->m_vertexIds[0] ];
				const Imath::V3f &p1 = m_verts->readable()[ result->m_vertexIds[1] ];
				const Imath::V3f &p2 = m_verts->readable()[ result->m_vertexIds[2] ];

				result->m_p = trianglePoint( p0, p1, p2, result->m_bary );

				result->m_n = triangleNormal( p0, p1, p2 );

				return true;
			}
		}
		
		return false;
	}
	else
	{
		if( pointAtUVWalk( UVBoundTree::lowChildIndex( nodeIndex ), targetUV, result ) )
		{
			return true;
		}
		
		if( pointAtUVWalk( UVBoundTree::highChildIndex( nodeIndex ), targetUV, result ) )
		{
			return true;
		}

		return false;
	}

}


bool MeshPrimitiveEvaluator::intersectionPointWalk( TriangleBoundTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float &maxDistSqrd, Result *result, bool &hit ) const
{
	assert( m_tree );

	const TriangleBoundTree::Node &node = m_tree->node( nodeIndex );

	if( node.isLeaf() )
	{
		TriangleBoundTree::Iterator *permLast = node.permLast();
		bool intersects = false;

		for( TriangleBoundTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			size_t triangleIndex = *perm - m_triangles.begin(); // triangle index is just the distance of the triangle from the beginning of the vector
			size_t vertIdOffset = triangleIndex * 3;
			Imath::V3i vertexIds( (*m_meshVertexIds)[vertIdOffset], (*m_meshVertexIds)[vertIdOffset+1], (*m_meshVertexIds)[vertIdOffset+2] );

			assert( vertexIds[0] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[1] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[2] < (int)( m_verts->readable().size() ) );

			const Imath::V3f &p0 = m_verts->readable()[ vertexIds[0] ];
			const Imath::V3f &p1 = m_verts->readable()[ vertexIds[1] ];
			const Imath::V3f &p2 = m_verts->readable()[ vertexIds[2] ];

			V3f hitPoint, bary;
			bool front;

			if ( triangleRayIntersection( p0, p1, p2, ray.pos, ray.dir, hitPoint, bary, front ) )
			{
				float dSqrd = vecDistance2( hitPoint, ray.pos );

				if (dSqrd < maxDistSqrd)
				{
					maxDistSqrd = dSqrd;

					result->m_bary = bary;
					result->m_vertexIds = vertexIds;
					result->m_triangleIdx = triangleIndex;

					result->m_p = hitPoint;

					if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
					{
						result->m_uv = V2f(
							result->floatPrimVar( m_u ),
							result->floatPrimVar( m_v )
						);
					}

					result->m_n = triangleNormal( p0, p1, p2 );

					intersects = true;
					hit = true;
				}
			}
		}

		return intersects;
	}
	else
	{
		V3f highHitPoint;
		bool highHit = boxIntersects(
			m_tree->node( TriangleBoundTree::highChildIndex( nodeIndex ) ).bound(),
			ray.pos,
			ray.dir,
			highHitPoint
		);

		V3f lowHitPoint;
		bool lowHit = boxIntersects(
			m_tree->node( TriangleBoundTree::lowChildIndex( nodeIndex ) ).bound(),
			ray.pos,
			ray.dir,
			lowHitPoint
		);

		float dHigh = -1;
		if ( highHit )
		{
			float distSqrd = vecDistance2( highHitPoint, ray.pos );
			if ( distSqrd > maxDistSqrd )
			{
				highHit = false;
			}
			else
			{
				dHigh = distSqrd;
			}
		}

		float dLow = -1;
		if ( lowHit )
		{
			float distSqrd = vecDistance2( lowHitPoint, ray.pos );
			if ( distSqrd > maxDistSqrd )
			{
				lowHit = false;
			}
			else
			{
				dLow = distSqrd;
			}
		}

		if (lowHit)
		{
			if (highHit)
			{
				/// Descend into the closest intersection first

				TriangleBoundTree::NodeIndex firstChild, secondChild;
				float dSecond;
				if (dHigh < dLow)
				{
					firstChild = TriangleBoundTree::highChildIndex( nodeIndex );
					secondChild = TriangleBoundTree::lowChildIndex( nodeIndex );
					dSecond = dLow;
				}
				else
				{
					firstChild = TriangleBoundTree::lowChildIndex( nodeIndex );
					secondChild = TriangleBoundTree::highChildIndex( nodeIndex );
					dSecond = dHigh;
				}

				bool intersection = intersectionPointWalk( firstChild, ray, maxDistSqrd, result, hit );

				if (intersection)
				{
					if ( dSecond < maxDistSqrd )
					{
						intersectionPointWalk( secondChild, ray, maxDistSqrd, result, hit );
					}

					return true;
				}
				else
				{
					return intersectionPointWalk( secondChild, ray, maxDistSqrd, result, hit );
				}
			}
			else
			{
				return intersectionPointWalk( TriangleBoundTree::lowChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
			}

		}
		else if (highHit)
		{
			return intersectionPointWalk( TriangleBoundTree::highChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
		}


		return false;
	}

}

void MeshPrimitiveEvaluator::intersectionPointsWalk( TriangleBoundTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float maxDistSqrd, std::vector<PrimitiveEvaluator::ResultPtr> &results ) const
{
	assert( m_tree );

	const TriangleBoundTree::Node &node = m_tree->node( nodeIndex );

	if( node.isLeaf() )
	{
		TriangleBoundTree::Iterator *permLast = node.permLast();

		for( TriangleBoundTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			size_t triangleIndex = *perm - m_triangles.begin(); // triangle index is just the distance of the triangle from the beginning of the vector
			size_t vertIdOffset = triangleIndex * 3;
			Imath::V3i vertexIds( (*m_meshVertexIds)[vertIdOffset], (*m_meshVertexIds)[vertIdOffset+1], (*m_meshVertexIds)[vertIdOffset+2] );

			assert( vertexIds[0] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[1] < (int)( m_verts->readable().size() ) );
			assert( vertexIds[2] < (int)( m_verts->readable().size() ) );

			const Imath::V3f &p0 =  m_verts->readable()[ vertexIds[0] ];
			const Imath::V3f &p1 =  m_verts->readable()[ vertexIds[1] ];
			const Imath::V3f &p2 =  m_verts->readable()[ vertexIds[2] ];

			V3f hitPoint, bary;
			bool front;

			if ( triangleRayIntersection( p0, p1, p2, ray.pos, ray.dir, hitPoint, bary, front ) )
			{
				float dSqrd = vecDistance2( hitPoint, ray.pos );

				if (dSqrd < maxDistSqrd)
				{
					ResultPtr result = new Result();

					result->m_bary = bary;
					result->m_vertexIds = vertexIds;
					result->m_triangleIdx = triangleIndex;

					result->m_p = hitPoint;

					if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
					{
						result->m_uv = V2f(
							result->floatPrimVar( m_u ),
							result->floatPrimVar( m_v )
						);
					}

					result->m_n = triangleNormal( p0, p1, p2 );

					results.push_back( result );
				}
			}
		}
	}
	else
	{
		V3f hitPoint;

		/// Test highChild bound for intersection, descending into children if necessary
		bool hit = boxIntersects(
			m_tree->node( TriangleBoundTree::highChildIndex( nodeIndex ) ).bound(),
			ray.pos,
			ray.dir,
			hitPoint
		);

		if ( hit && vecDistance2( hitPoint, ray.pos ) < maxDistSqrd )
		{
			intersectionPointsWalk( TriangleBoundTree::highChildIndex( nodeIndex ), ray, maxDistSqrd, results );
		}

		/// Test lowChild bound for intersection, descending into children if necessary
		hit = boxIntersects(
			m_tree->node( TriangleBoundTree::lowChildIndex( nodeIndex ) ).bound(),
			ray.pos,
			ray.dir,
			hitPoint
		);

		if ( hit && vecDistance2( hitPoint, ray.pos ) < maxDistSqrd )
		{
			intersectionPointsWalk( TriangleBoundTree::lowChildIndex( nodeIndex ), ray, maxDistSqrd, results );
		}
	}
}

const Imath::Box2f MeshPrimitiveEvaluator::uvBound() const
{
	if( !m_uvTree )
	{
		return Imath::Box2f();
	}
	return m_uvTree->node( m_uvTree->rootIndex() ).bound();
}

const MeshPrimitiveEvaluator::TriangleBoundVector *MeshPrimitiveEvaluator::triangleBounds() const
{
	return &m_triangles;
}

const MeshPrimitiveEvaluator::TriangleBoundTree *MeshPrimitiveEvaluator::triangleBoundTree() const
{
	return m_tree;
}

const MeshPrimitiveEvaluator::UVBoundVector *MeshPrimitiveEvaluator::uvBounds() const
{
	return m_uvTree ? &m_uvTriangles : 0;
}

const MeshPrimitiveEvaluator::UVBoundTree *MeshPrimitiveEvaluator::uvBoundTree() const
{
	return m_uvTree;
}

void MeshPrimitiveEvaluator::triangleUVs( size_t triangleIndex, const Imath::V3i &vertexIds, Imath::V2f uv[3] ) const
{
	const std::vector<float> &u = ((FloatVectorData *)(m_u.data.get()))->readable();
	if( m_u.interpolation==PrimitiveVariable::FaceVarying )
	{
		size_t index = triangleIndex * 3;
		uv[0][0] = u[index++];
		uv[1][0] = u[index++];
		uv[2][0] = u[index];
	}
	else
	{
		// vertex interpolation
		uv[0][0] = u[vertexIds[0]];
		uv[1][0] = u[vertexIds[1]];
		uv[2][0] = u[vertexIds[2]];
	}
	
	const std::vector<float> &v = ((FloatVectorData *)(m_v.data.get()))->readable();
	if( m_v.interpolation==PrimitiveVariable::FaceVarying )
	{
		size_t index = triangleIndex * 3;
		uv[0][1] = v[index++];
		uv[1][1] = v[index++];
		uv[2][1] = v[index];
	}
	else
	{
		// vertex interpolation
		uv[0][1] = v[vertexIds[0]];
		uv[1][1] = v[vertexIds[1]];
		uv[2][1] = v[vertexIds[2]];
	}	
}
