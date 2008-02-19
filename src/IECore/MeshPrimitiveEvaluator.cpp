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

#include <cassert>

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathLineAlgo.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/PrimitiveVariable.h"
#include "IECore/Exception.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Deleter.h"
#include "IECore/ClassData.h"

using namespace IECore;
using namespace Imath;

struct MeshPrimitiveEvaluator::ExtraData
{	
	ExtraData() : m_uvTree(0), m_haveMassProperties( false ), m_haveSurfaceArea( false )
	{
	}

	BoundedTriangleVector m_uvTriangles;
	BoundedTriangleTree *m_uvTree;
	
	PrimitiveVariable m_u;
	PrimitiveVariable m_v;
	
	bool m_haveMassProperties;
	float m_volume;
	V3f m_centerOfGravity;	
	M33f m_inertia;
	
	bool m_haveSurfaceArea;
	float m_surfaceArea;
};

typedef ClassData< MeshPrimitiveEvaluator, MeshPrimitiveEvaluator::ExtraData*, Deleter<MeshPrimitiveEvaluator::ExtraData*> > MeshPrimitiveEvaluatorClassData;
static MeshPrimitiveEvaluatorClassData g_classData;

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
	StringDataPtr data = runTimeCast< StringData >( pv.data );
		
	if (data)
	{
		return data->readable();
	}	
	else
	{
		StringVectorDataPtr data = runTimeCast< StringVectorData >( pv.data );
		
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
		typedef typename Data::Ptr DataPtr;
		
		DataPtr data = runTimeCast< Data >( pv.data );
		
		if (data)
		{
			return data->readable();
		}
	}
	
	typedef TypedData< std::vector<T> > VectorData;
	typedef typename VectorData::Ptr VectorDataPtr;
	VectorDataPtr data = runTimeCast< VectorData >( pv.data );

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

MeshPrimitiveEvaluator::MeshPrimitiveEvaluator( ConstMeshPrimitivePtr mesh )
{	
	if (! mesh )
	{
		throw InvalidArgumentException( "No mesh given to MeshPrimitiveEvaluator");
	}
	
	if (! ( boost::const_pointer_cast< MeshPrimitive >(mesh) )->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to MeshPrimitiveEvaluator");
	}
	
	m_mesh = mesh->copy();
	
	IntVectorData::ValueType::const_iterator vertexIdIt = m_mesh->vertexIds()->readable().begin();
	
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
	
	ExtraData *extraData = g_classData.create( this, new ExtraData() );
	assert( extraData );
	BoundedTriangleTree*    &m_uvTree      = extraData->m_uvTree;
	BoundedTriangleVector   &m_uvTriangles = extraData->m_uvTriangles;
	PrimitiveVariable       &m_u           = extraData->m_u;
	PrimitiveVariable       &m_v           = extraData->m_v;	
	
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
	
	ResultPtr input = new Result();
	
	unsigned int triangleIdx = 0;
	for ( IntVectorData::ValueType::const_iterator it = m_mesh->verticesPerFace()->readable().begin(); 
		it != m_mesh->verticesPerFace()->readable().end(); ++it, ++triangleIdx)
	{
	
		if (*it != 3 )
		{
			throw InvalidArgumentException( "Non-triangular mesh given to MeshPrimitiveEvaluator");
		}		
		
		Imath::V3i triangleVertexIds;
		
		triangleVertexIds[0] = *vertexIdIt++;
		triangleVertexIds[1] = *vertexIdIt++;		
		triangleVertexIds[2] = *vertexIdIt++;		
		
		const V3f &p0 = m_verts->readable()[ triangleVertexIds[0] ];
		const V3f &p1 = m_verts->readable()[ triangleVertexIds[1] ];
		const V3f &p2 = m_verts->readable()[ triangleVertexIds[2] ];
		
		Box3f bound( p0 );
		bound.extendBy( p1 );		
		bound.extendBy( p2 );		
		
		m_triangles.push_back( BoundedTriangle( bound, triangleVertexIds, triangleIdx ) );
						
		if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
		{	
			input->m_vertexIds = triangleVertexIds;
			input->m_triangleIdx = triangleIdx;
			
			input->m_bary = V3f( 1, 0, 0 );			
			Box3f uvBound(				
				V3f( 
					input->floatPrimVar( m_u ),
					input->floatPrimVar( m_v ),
					0.0f
				)
			);

			input->m_bary = V3f( 0, 1, 0 );			
			uvBound.extendBy(
				V3f( 
					input->floatPrimVar( m_u ),
					input->floatPrimVar( m_v ),
					0.0f
				)
			);

			input->m_bary = V3f( 0, 0, 1 );			
			uvBound.extendBy(
				V3f( 
					input->floatPrimVar( m_u ),
					input->floatPrimVar( m_v ),
					0.0f
				)
			);
			
			uvBound.min.z = - Imath::limits<float>::epsilon();
			uvBound.max.z =   Imath::limits<float>::epsilon();				

			m_uvTriangles.push_back( BoundedTriangle( uvBound, triangleVertexIds, triangleIdx ) );
		
		}						
	}
	
	m_tree = new BoundedTriangleTree( m_triangles.begin(), m_triangles.end() );
				
	if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
	{
		m_uvTree = new BoundedTriangleTree( m_uvTriangles.begin(), m_uvTriangles.end() );
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
	
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	BoundedTriangleTree* &m_uvTree = extraData->m_uvTree;
	delete m_uvTree;
	m_uvTree = 0;
		
	g_classData.erase( this );
}

float MeshPrimitiveEvaluator::volume() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	if ( !extraData->m_haveMassProperties )
	{
		const_cast<MeshPrimitiveEvaluator*>(this)->calculateMassProperties();
	}
	
	return extraData->m_volume;
}

Imath::V3f MeshPrimitiveEvaluator::centerOfGravity() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	if ( !extraData->m_haveMassProperties )
	{
		const_cast<MeshPrimitiveEvaluator*>(this)->calculateMassProperties();
	}	
	
	return extraData->m_centerOfGravity;
}

float MeshPrimitiveEvaluator::surfaceArea() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	if ( !extraData->m_haveSurfaceArea )
	{
		extraData->m_surfaceArea = 0.0f;
		IntVectorData::ValueType::const_iterator vertexIdIt = m_mesh->vertexIds()->readable().begin();
	
		for ( IntVectorData::ValueType::const_iterator it = m_mesh->verticesPerFace()->readable().begin(); 
			it != m_mesh->verticesPerFace()->readable().end(); ++it )
		{	
			assert ( *it == 3 );	

			const V3f &p0 = m_verts->readable()[ *vertexIdIt++ ];
			const V3f &p1 = m_verts->readable()[ *vertexIdIt++ ];
			const V3f &p2 = m_verts->readable()[ *vertexIdIt++ ];
			
			extraData->m_surfaceArea += triangleArea( p0, p1, p2 );
		}	
		
		extraData->m_haveSurfaceArea = true;
	}	
	
	return extraData->m_surfaceArea;
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license
void MeshPrimitiveEvaluator::calculateMassProperties()
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	assert( !extraData->m_haveMassProperties );
	
	double integral[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	
	IntVectorData::ValueType::const_iterator vertexIdIt = m_mesh->vertexIds()->readable().begin();
	
	for ( IntVectorData::ValueType::const_iterator it = m_mesh->verticesPerFace()->readable().begin(); 
		it != m_mesh->verticesPerFace()->readable().end(); ++it )
	{	
		assert ( *it == 3 );	
		
		const V3f &p0 = m_verts->readable()[ *vertexIdIt++ ];
		const V3f &p1 = m_verts->readable()[ *vertexIdIt++ ];
		const V3f &p2 = m_verts->readable()[ *vertexIdIt++ ];
		
		/// Winding order has to be correct here
		V3f n = ( p2 - p0 ).cross( p1 - p0 );
		
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
			
	extraData->m_volume = integral[0];
	extraData->m_centerOfGravity = V3f( integral[1], integral[2], integral[3] ) / integral[0];
	extraData->m_inertia[0][0] = integral[5] + integral[6];
	extraData->m_inertia[0][1] = -integral[7];
	extraData->m_inertia[0][2] = -integral[9];
	extraData->m_inertia[1][0] = extraData->m_inertia[0][1];
	extraData->m_inertia[1][1] = integral[4] + integral[6];
	extraData->m_inertia[1][2] = -integral[8];
	extraData->m_inertia[2][0] = extraData->m_inertia[0][2];
	extraData->m_inertia[2][1] = extraData->m_inertia[1][2];
	extraData->m_inertia[2][2] = integral[4] + integral[5];			
	
	extraData->m_haveMassProperties = true;
}

PrimitiveEvaluator::ResultPtr MeshPrimitiveEvaluator::createResult() const
{
      return new Result();
}

bool MeshPrimitiveEvaluator::closestPoint( const V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	if ( m_triangles.size() == 0)
	{
		return false;
	}
	
	assert( m_tree );
	
	ResultPtr mr = boost::static_pointer_cast< Result >( result );
	
	float maxDistSqrd = limits<float>::max();
		
	closestPointWalk( m_tree->rootIndex(), p, maxDistSqrd, mr );
	
	return true;
}

bool MeshPrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	BoundedTriangleVector &m_uvTriangles = extraData->m_uvTriangles;	
	
	if ( ! m_uvTriangles.size() )
	{
		throw Exception("No uvs available for pointAtUV");
	}

#ifndef NDEBUG	
	BoundedTriangleTree* &m_uvTree = extraData->m_uvTree;		
	assert( m_uvTree );
#endif	
	
	ResultPtr mr = boost::static_pointer_cast< Result >( result );
	
	float maxDistSqrd = 2.0f;
	
	Imath::Line3f ray;
	ray.pos = V3f( uv.x, uv.y, 1.0 );
	ray.dir = V3f( 0, 0, -1 );
	
	bool hit = false;
			
	pointAtUVWalk( m_tree->rootIndex(), ray, maxDistSqrd, mr, hit );
	return hit;
}

bool MeshPrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
	const PrimitiveEvaluator::ResultPtr &result, float maxDistance ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	if ( m_triangles.size() == 0)
	{
		return false;
	}
	
	assert( m_tree );
	
	ResultPtr mr = boost::static_pointer_cast< Result >( result );
	
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

void MeshPrimitiveEvaluator::closestPointWalk( BoundedTriangleTree::NodeIndex nodeIndex, const V3f &p, float &closestDistanceSqrd, const ResultPtr &result ) const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );
	
	PrimitiveVariable &m_u  = extraData->m_u;
	PrimitiveVariable &m_v  = extraData->m_v;
	assert( m_tree );
	
	const BoundedTriangleTree::Node &node = m_tree->node( nodeIndex );
	if( node.isLeaf() )
	{
		BoundedTriangleTree::Iterator *permLast = node.permLast();
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;
			
			V3f bary;
			float dSqrd = triangleClosestBarycentric( 
				m_verts->readable()[bb.m_vertexIds[0]], 
				m_verts->readable()[bb.m_vertexIds[1]], 
				m_verts->readable()[bb.m_vertexIds[2]], 
				p, 
				bary );
				
			if (dSqrd < closestDistanceSqrd)
			{
				closestDistanceSqrd = dSqrd;
				
				result->m_bary = bary;
				result->m_vertexIds[0] = bb.m_vertexIds[0];
				result->m_vertexIds[1] = bb.m_vertexIds[1];
				result->m_vertexIds[2] = bb.m_vertexIds[2];
				
				result->m_triangleIdx = bb.m_triangleIndex;
				
				if ( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid )
				{				
					result->m_uv = V2f( 
						result->floatPrimVar( m_u ),
						result->floatPrimVar( m_v )
					);	
				}
				
				const Imath::V3f &p0 = m_verts->readable()[ bb.m_vertexIds[0] ];
				const Imath::V3f &p1 = m_verts->readable()[ bb.m_vertexIds[1] ];
				const Imath::V3f &p2 = m_verts->readable()[ bb.m_vertexIds[2] ];
					
				result->m_p = trianglePoint( p0, p1, p2, result->m_bary );

				result->m_n = triangleNormal( p0, p1, p2 );
			}			
		}		
	}
	else
	{	
		/// Descend into the closest box first
		
		float dHigh = vecDistance(
			closestPointInBox( p, m_tree->node( BoundedTriangleTree::highChildIndex( nodeIndex ) ).bound() ),
			p
		);
		
		float dLow = vecDistance(
			closestPointInBox( p, m_tree->node( BoundedTriangleTree::lowChildIndex( nodeIndex ) ).bound() ),
			p
		);

		BoundedTriangleTree::NodeIndex firstChild, secondChild;
		
		float dSecond;
				
		if (dHigh < dLow)
		{
			firstChild = BoundedTriangleTree::highChildIndex( nodeIndex );
			secondChild = BoundedTriangleTree::lowChildIndex( nodeIndex );	
			dSecond = dLow;		
		}
		else
		{
			firstChild = BoundedTriangleTree::lowChildIndex( nodeIndex );
			secondChild = BoundedTriangleTree::highChildIndex( nodeIndex );
			dSecond = dHigh;			
		}	
			
		closestPointWalk( firstChild, p, closestDistanceSqrd, result );
			
		if (dSecond * dSecond < closestDistanceSqrd )
		{
			closestPointWalk( secondChild, p, closestDistanceSqrd, result );
		}		
	}
}

bool MeshPrimitiveEvaluator::pointAtUVWalk( BoundedTriangleTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float &maxDistSqrd, const ResultPtr &result, bool &hit ) const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );
	
	PrimitiveVariable &m_u  = extraData->m_u;
	PrimitiveVariable &m_v  = extraData->m_v;
	BoundedTriangleTree*    &m_uvTree = extraData->m_uvTree;
	
	assert( m_u.interpolation != PrimitiveVariable::Invalid && m_v.interpolation != PrimitiveVariable::Invalid);	
	assert( m_uvTree );
	
	const BoundedTriangleTree::Node &node = g_classData[this]->m_uvTree->node( nodeIndex );
	
	if( node.isLeaf() )
	{	
		BoundedTriangleTree::Iterator *permLast = node.permLast();
		bool intersects = false;
		
		ResultPtr input = new Result();
		
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;
			
			Imath::V3f uv[3];
									
			input->m_triangleIdx = bb.m_triangleIndex;
			input->m_vertexIds = bb.m_vertexIds;
			
			input->m_bary = V3f( 1, 0, 0 );			
			uv[0] = V3f( input->floatPrimVar( m_u ), input->floatPrimVar( m_v ), 0.0f );
			input->m_bary = V3f( 0, 1, 0 );			
			uv[1] = V3f( input->floatPrimVar( m_u ), input->floatPrimVar( m_v ), 0.0f );
			input->m_bary = V3f( 0, 0, 1 );			
			uv[2] = V3f( input->floatPrimVar( m_u ), input->floatPrimVar( m_v ), 0.0f );			
			
			V3f hitPoint, bary;			
			bool front;
			
			if ( intersect( ray, uv[0], uv[1], uv[2], hitPoint, bary, front ) )
			{				
				float dSqrd = vecDistance2( hitPoint, ray.pos );
				
				if (dSqrd < maxDistSqrd)
				{
					maxDistSqrd = dSqrd;

					result->m_bary = bary;
					result->m_vertexIds[0] = bb.m_vertexIds[0];
					result->m_vertexIds[1] = bb.m_vertexIds[1];
					result->m_vertexIds[2] = bb.m_vertexIds[2];

					result->m_triangleIdx = bb.m_triangleIndex;				

					result->m_uv = Imath::V2f( hitPoint.x, hitPoint.y );
					
					const Imath::V3f &p0 = m_verts->readable()[ bb.m_vertexIds[0] ];
					const Imath::V3f &p1 = m_verts->readable()[ bb.m_vertexIds[1] ];
					const Imath::V3f &p2 = m_verts->readable()[ bb.m_vertexIds[2] ];
					
					result->m_p = trianglePoint( p0, p1, p2, result->m_bary );

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
		bool highHit = intersects(
			m_uvTree->node( BoundedTriangleTree::highChildIndex( nodeIndex ) ).bound(),
			ray, 
			highHitPoint
		);
		
		V3f lowHitPoint;
		bool lowHit = intersects(
			m_uvTree->node( BoundedTriangleTree::lowChildIndex( nodeIndex ) ).bound(),
			ray, 
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
				
				BoundedTriangleTree::NodeIndex firstChild, secondChild;
				float dSecond;
				if (dHigh < dLow)
				{
					firstChild = BoundedTriangleTree::highChildIndex( nodeIndex );
					secondChild = BoundedTriangleTree::lowChildIndex( nodeIndex );	
					dSecond = dLow;		
				}
				else
				{
					firstChild = BoundedTriangleTree::lowChildIndex( nodeIndex );
					secondChild = BoundedTriangleTree::highChildIndex( nodeIndex );
					dSecond = dHigh;			
				}	

				bool intersection = pointAtUVWalk( firstChild, ray, maxDistSqrd, result, hit );
				
				if (intersection)
				{					
					if ( dSecond < maxDistSqrd )
					{
						pointAtUVWalk( secondChild, ray, maxDistSqrd, result, hit );						
					}
					
					return true;
				}
				else
				{
					return pointAtUVWalk( secondChild, ray, maxDistSqrd, result, hit );
				}
			}
			else
			{
				return pointAtUVWalk( BoundedTriangleTree::lowChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
			}
				
		}
		else if (highHit)
		{
			return pointAtUVWalk( BoundedTriangleTree::highChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
		}

	
		return false;
	}
	
}


bool MeshPrimitiveEvaluator::intersectionPointWalk( BoundedTriangleTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float &maxDistSqrd, const ResultPtr &result, bool &hit ) const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );
	
	PrimitiveVariable &m_u  = extraData->m_u;
	PrimitiveVariable &m_v  = extraData->m_v;
	
	assert( m_tree );
	
	const BoundedTriangleTree::Node &node = m_tree->node( nodeIndex );
	
	if( node.isLeaf() )
	{	
		BoundedTriangleTree::Iterator *permLast = node.permLast();
		bool intersects = false;
		
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;
			
			const Imath::V3f &p0 = m_verts->readable()[ bb.m_vertexIds[0] ];
			const Imath::V3f &p1 = m_verts->readable()[ bb.m_vertexIds[1] ];
			const Imath::V3f &p2 = m_verts->readable()[ bb.m_vertexIds[2] ];
			
			V3f hitPoint, bary;			
			bool front;
			
			if ( intersect( ray, p0, p1, p2, hitPoint, bary, front ) )
			{				
				float dSqrd = vecDistance2( hitPoint, ray.pos );
				
				if (dSqrd < maxDistSqrd)
				{
					maxDistSqrd = dSqrd;

					result->m_bary = bary;
					result->m_vertexIds[0] = bb.m_vertexIds[0];
					result->m_vertexIds[1] = bb.m_vertexIds[1];
					result->m_vertexIds[2] = bb.m_vertexIds[2];

					result->m_triangleIdx = bb.m_triangleIndex;				

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
		bool highHit = intersects(
			m_tree->node( BoundedTriangleTree::highChildIndex( nodeIndex ) ).bound(),
			ray, 
			highHitPoint
		);
		
		V3f lowHitPoint;
		bool lowHit = intersects(
			m_tree->node( BoundedTriangleTree::lowChildIndex( nodeIndex ) ).bound(),
			ray, 
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
				
				BoundedTriangleTree::NodeIndex firstChild, secondChild;
				float dSecond;
				if (dHigh < dLow)
				{
					firstChild = BoundedTriangleTree::highChildIndex( nodeIndex );
					secondChild = BoundedTriangleTree::lowChildIndex( nodeIndex );	
					dSecond = dLow;		
				}
				else
				{
					firstChild = BoundedTriangleTree::lowChildIndex( nodeIndex );
					secondChild = BoundedTriangleTree::highChildIndex( nodeIndex );
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
				return intersectionPointWalk( BoundedTriangleTree::lowChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
			}
				
		}
		else if (highHit)
		{
			return intersectionPointWalk( BoundedTriangleTree::highChildIndex( nodeIndex ), ray, maxDistSqrd, result, hit );
		}

	
		return false;
	}
	
}

void MeshPrimitiveEvaluator::intersectionPointsWalk( BoundedTriangleTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float maxDistSqrd, std::vector<PrimitiveEvaluator::ResultPtr> &results ) const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );
	
	PrimitiveVariable &m_u  = extraData->m_u;
	PrimitiveVariable &m_v  = extraData->m_v;

	assert( m_tree );
	
	const BoundedTriangleTree::Node &node = m_tree->node( nodeIndex );
	
	if( node.isLeaf() )
	{
		BoundedTriangleTree::Iterator *permLast = node.permLast();
				
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;

			const Imath::V3f &p0 =  m_verts->readable()[ bb.m_vertexIds[0] ];
			const Imath::V3f &p1 =  m_verts->readable()[ bb.m_vertexIds[1] ];
			const Imath::V3f &p2 =  m_verts->readable()[ bb.m_vertexIds[2] ];
			
			V3f hitPoint, bary;			
			bool front;
			
			if ( intersect( ray, p0, p1, p2, hitPoint, bary, front ) )
			{				
				float dSqrd = vecDistance2( hitPoint, ray.pos );
				
				if (dSqrd < maxDistSqrd)
				{			
					ResultPtr result = new Result();

					result->m_bary = bary;
					result->m_vertexIds[0] = bb.m_vertexIds[0];
					result->m_vertexIds[1] = bb.m_vertexIds[1];
					result->m_vertexIds[2] = bb.m_vertexIds[2];

					result->m_triangleIdx = bb.m_triangleIndex;				

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
		bool hit = intersects(
			m_tree->node( BoundedTriangleTree::highChildIndex( nodeIndex ) ).bound(),
			ray, 
			hitPoint
		);
				
		if ( hit && vecDistance2( hitPoint, ray.pos ) < maxDistSqrd )
		{		
			intersectionPointsWalk( BoundedTriangleTree::highChildIndex( nodeIndex ), ray, maxDistSqrd, results );
		}
			
		/// Test lowChild bound for intersection, descending into children if necessary				
		hit = intersects(
			m_tree->node( BoundedTriangleTree::lowChildIndex( nodeIndex ) ).bound(),
			ray, 
			hitPoint
		);
				
		if ( hit && vecDistance2( hitPoint, ray.pos ) < maxDistSqrd )
		{		
			intersectionPointsWalk( BoundedTriangleTree::lowChildIndex( nodeIndex ), ray, maxDistSqrd, results );
		}
	}	
}
