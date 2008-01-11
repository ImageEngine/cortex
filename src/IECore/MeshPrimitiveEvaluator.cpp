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

#include "IECore/PrimitiveVariable.h"
#include "IECore/Exception.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace Imath;

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
		
		V3f p0 = m_verts->readable()[ triangleVertexIds[0] ];
		V3f p1 = m_verts->readable()[ triangleVertexIds[1] ];
		V3f p2 = m_verts->readable()[ triangleVertexIds[2] ];
		
		Box3f bound( p0 );
		bound.extendBy( p1 );		
		bound.extendBy( p2 );		
		
		m_triangles.push_back( BoundedTriangle( bound, triangleVertexIds, triangleIdx ) );				
	}
	
	m_tree = new BoundedTriangleTree( m_triangles.begin(), m_triangles.end() );
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
	throw NotImplementedException( __PRETTY_FUNCTION__ );
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
				
				Imath::V3f p[3];
				p[0] =  m_verts->readable()[ result->m_vertexIds[0] ];
				p[1] =  m_verts->readable()[ result->m_vertexIds[1] ];
				p[2] =  m_verts->readable()[ result->m_vertexIds[2] ];								
				
				result->m_p = trianglePoint( p[0], p[1], p[2], result->m_bary );
				
				result->m_n = triangleNormal( p[0], p[1], p[2] );
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


bool MeshPrimitiveEvaluator::intersectionPointWalk( BoundedTriangleTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float &maxDistSqrd, const ResultPtr &result, bool &hit ) const
{
	assert( m_tree );
	
	const BoundedTriangleTree::Node &node = m_tree->node( nodeIndex );
	
	if( node.isLeaf() )
	{	
		BoundedTriangleTree::Iterator *permLast = node.permLast();
		bool intersects = false;
		
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;
			
			Imath::V3f p[3];
			p[0] =  m_verts->readable()[ bb.m_vertexIds[0] ];
			p[1] =  m_verts->readable()[ bb.m_vertexIds[1] ];
			p[2] =  m_verts->readable()[ bb.m_vertexIds[2] ];
			
			V3f hitPoint, bary;			
			bool front;
			
			if ( intersect( ray, p[0], p[1], p[2], hitPoint, bary, front ) )
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

					result->m_n = triangleNormal( p[0], p[1], p[2] );
					
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
					float distSqrd = vecDistance2( result->point(), ray.pos );
					if ( distSqrd > maxDistSqrd )
					{
						return intersectionPointWalk( secondChild, ray, maxDistSqrd, result, hit );
					}
					else
					{
						maxDistSqrd = distSqrd;
						return true;
					}
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
	assert( m_tree );
	
	const BoundedTriangleTree::Node &node = m_tree->node( nodeIndex );
	
	if( node.isLeaf() )
	{
		BoundedTriangleTree::Iterator *permLast = node.permLast();
				
		for( BoundedTriangleTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const BoundedTriangle &bb = **perm;
			
			Imath::V3f p[3];
			p[0] =  m_verts->readable()[ bb.m_vertexIds[0] ];
			p[1] =  m_verts->readable()[ bb.m_vertexIds[1] ];
			p[2] =  m_verts->readable()[ bb.m_vertexIds[2] ];
			
			V3f hitPoint, bary;			
			bool front;
			
			if ( intersect( ray, p[0], p[1], p[2], hitPoint, bary, front ) )
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

					result->m_n = triangleNormal( p[0], p[1], p[2] );

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

