//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>
#include <numeric>

#include "IECore/MeshPrimitive.h"
#include "IECore/Renderer.h"
#include "IECore/PolygonIterator.h"
#include "IECore/MurmurHash.h"
#include "IECore/Math.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

static IndexedIO::EntryID g_verticesPerFaceEntry("verticesPerFace");
static IndexedIO::EntryID g_vertexIdsEntry("vertexIds");
static IndexedIO::EntryID g_numVerticesEntry("numVertices");
static IndexedIO::EntryID g_interpolationEntry("interpolation");

const unsigned int MeshPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(MeshPrimitive);

MeshPrimitive::MeshPrimitive()
	: m_verticesPerFace( new IntVectorData ), m_vertexIds( new IntVectorData ), m_numVertices( 0 ), m_interpolation( "linear" ), m_minVerticesPerFace(0), m_maxVerticesPerFace(0)
{
}

MeshPrimitive::MeshPrimitive( ConstIntVectorDataPtr verticesPerFace, ConstIntVectorDataPtr vertexIds,
	const std::string &interpolation, V3fVectorDataPtr p )
{
	setTopology( verticesPerFace, vertexIds, interpolation );
	if( p )
	{
		V3fVectorDataPtr pData = p->copy();
		pData->setInterpretation( GeometricData::Point );
		variables.insert( PrimitiveVariableMap::value_type("P", PrimitiveVariable(PrimitiveVariable::Vertex, pData)) );
	}
}

size_t MeshPrimitive::numFaces() const
{
	return m_verticesPerFace->readable().size();
}

const IntVectorData *MeshPrimitive::verticesPerFace() const
{
	return m_verticesPerFace.get();
}

void MeshPrimitive::computeMinMaxVertsPerFace() const
{
	int minVertsPerFace = 0x7fffffff;
	int maxVertsPerFace = 0;
	unsigned int numExpectedVertexIds = 0;
	for ( vector<int>::const_iterator it = m_verticesPerFace->readable().begin(); it != m_verticesPerFace->readable().end(); it++ )
	{
		int vertsPerFace = *it;
		minVertsPerFace = std::min( minVertsPerFace, vertsPerFace );
		maxVertsPerFace = std::max( maxVertsPerFace, vertsPerFace );
		numExpectedVertexIds += vertsPerFace;
	}
	if( minVertsPerFace<3 )
	{
		throw Exception( "Bad topology - number of vertices per face less than 3." );
	}

	if( numExpectedVertexIds!=m_vertexIds->readable().size() )
	{
		throw Exception( "Bad topology - number of vertexIds not equal to sum of verticesPerFace" );
	}

	m_minVerticesPerFace = ( m_verticesPerFace->readable().size() ? minVertsPerFace : 0 );
	m_maxVerticesPerFace = maxVertsPerFace;
}
		
int MeshPrimitive::minVerticesPerFace() const
{
	if( m_maxVerticesPerFace == 0 )
	{
		computeMinMaxVertsPerFace();
	}
	return m_minVerticesPerFace;
}

int MeshPrimitive::maxVerticesPerFace() const
{
	if( m_maxVerticesPerFace == 0 )
	{
		computeMinMaxVertsPerFace();
	}
	return m_maxVerticesPerFace;
}

const IntVectorData *MeshPrimitive::vertexIds() const
{
	return m_vertexIds.get();
}

const std::string &MeshPrimitive::interpolation() const
{
	return m_interpolation;
}

void MeshPrimitive::setTopology( ConstIntVectorDataPtr verticesPerFace, ConstIntVectorDataPtr vertexIds, const std::string &interpolation )
{
	assert( verticesPerFace );
	assert( vertexIds );

	int maxVertexId = -1;
	for ( vector<int>::const_iterator it = vertexIds->readable().begin(); it != vertexIds->readable().end(); it++ )
	{
		int id = *it;
		if( id<0 )
		{
			throw Exception( "Bad topology - vertexId less than 0." );
		}
		maxVertexId = std::max( maxVertexId, id );
	}
	
	m_verticesPerFace = verticesPerFace->copy();
	m_vertexIds = vertexIds->copy();
	
	computeMinMaxVertsPerFace();
	
	if( m_vertexIds->readable().size() )
	{
		m_numVertices = 1 + maxVertexId;
	}
	else
	{
		m_numVertices = 0;
	}
	m_interpolation = interpolation;
}

void MeshPrimitive::setTopologyUnchecked( ConstIntVectorDataPtr verticesPerFace, ConstIntVectorDataPtr vertexIds, size_t numVertices, const std::string &interpolation )
{
	m_interpolation = interpolation;
	m_verticesPerFace = verticesPerFace->copy();
	m_vertexIds = vertexIds->copy();
	m_numVertices = numVertices;
	m_minVerticesPerFace = 0;
	m_maxVerticesPerFace = 0;
}

void MeshPrimitive::setInterpolation( const std::string &interpolation )
{
	m_interpolation = interpolation;
}

PolygonIterator MeshPrimitive::faceBegin()
{
	return PolygonIterator( m_verticesPerFace->readable().begin(), m_vertexIds->readable().begin(), 0 );
}

PolygonIterator MeshPrimitive::faceEnd()
{
	return PolygonIterator( m_verticesPerFace->readable().end(), m_vertexIds->readable().end(), m_vertexIds->readable().size() );
}
		
size_t MeshPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch(interpolation)
	{
		case PrimitiveVariable::Constant :
			return 1;

		case PrimitiveVariable::Uniform :
			return m_verticesPerFace->readable().size();

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
			return m_numVertices;

		case PrimitiveVariable::FaceVarying:
			return m_vertexIds->readable().size();

		default :
			return 0;

	}
}

void MeshPrimitive::render( Renderer *renderer ) const
{
	renderer->mesh( m_verticesPerFace, m_vertexIds, m_interpolation, variables );
}

void MeshPrimitive::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const MeshPrimitive *tOther = static_cast<const MeshPrimitive *>( other );
	m_verticesPerFace = tOther->m_verticesPerFace->copy();
	m_vertexIds = tOther->m_vertexIds->copy();
	m_numVertices = tOther->m_numVertices;
	m_interpolation = tOther->m_interpolation;
}

void MeshPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_verticesPerFace.get(), container.get(), g_verticesPerFaceEntry );
	context->save( m_vertexIds.get(), container.get(), g_vertexIdsEntry );

	/// \todo: mac has problems with the size_t type, resulting in the write() call being
	/// ambiguous to the compiler
	unsigned int numVertices = m_numVertices;
	container->write( g_numVerticesEntry, numVertices );

	container->write( g_interpolationEntry, m_interpolation );
}

void MeshPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	m_verticesPerFace = context->load<IntVectorData>( container.get(), g_verticesPerFaceEntry );
	m_vertexIds = context->load<IntVectorData>( container.get(), g_vertexIdsEntry );

	unsigned int numVertices;
	container->read( g_numVerticesEntry, numVertices );
	m_numVertices = numVertices;

	container->read( g_interpolationEntry, m_interpolation );
}

bool MeshPrimitive::isEqualTo( const Object *other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}

	const MeshPrimitive *tOther = static_cast<const MeshPrimitive *>( other );

	if( m_numVertices!=tOther->m_numVertices )
	{
		return false;
	}
	if( m_interpolation!=tOther->m_interpolation )
	{
		return false;
	}
	if( !m_verticesPerFace->isEqualTo( tOther->m_verticesPerFace.get() ) )
	{
		return false;
	}
	if( !m_vertexIds->isEqualTo( tOther->m_vertexIds.get() ) )
	{
		return false;
	}

	return true;
}

void MeshPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( m_verticesPerFace.get() );
	a.accumulate( m_vertexIds.get() );
}

void MeshPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void MeshPrimitive::topologyHash( MurmurHash &h ) const
{
	m_verticesPerFace->hash( h );
	m_vertexIds->hash( h );
	h.append( m_interpolation );
}

MeshPrimitivePtr MeshPrimitive::createBox( const Box3f &b )
{
	vector< int > verticesPerFaceVec;
	vector< int > vertexIdsVec;
	std::string interpolation = "linear";
	vector< V3f > p;
	int verticesPerFace[] = {
		4, 4, 4, 4, 4, 4
	};
	int vertexIds[] = {
		3,2,1,0,
		1,2,5,4,
		4,5,7,6,
		6,7,3,0,
		2,3,7,5,
		0,1,4,6
	};

	p.push_back( V3f( b.min.x, b.min.y, b.min.z ) );	// 0
	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );	// 1
	p.push_back( V3f( b.max.x, b.max.y, b.min.z ) );	// 2
	p.push_back( V3f( b.min.x, b.max.y, b.min.z ) );	// 3
	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );	// 4
	p.push_back( V3f( b.max.x, b.max.y, b.max.z ) );	// 5
	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );	// 6
	p.push_back( V3f( b.min.x, b.max.y, b.max.z ) );	// 7

	verticesPerFaceVec.resize( sizeof( verticesPerFace ) / sizeof( int ) );
	memcpy( &verticesPerFaceVec[0], &verticesPerFace[0], sizeof( verticesPerFace ) );

	vertexIdsVec.resize( sizeof( vertexIds ) / sizeof( int ) );
	memcpy( &vertexIdsVec[0], &vertexIds[0], sizeof( vertexIds ) );

	return new MeshPrimitive( new IntVectorData(verticesPerFaceVec), new IntVectorData(vertexIdsVec), interpolation, new V3fVectorData(p) );
}

MeshPrimitivePtr MeshPrimitive::createPlane( const Box2f &b, const Imath::V2i &divisions )
{
	V3fVectorDataPtr pData = new V3fVectorData;
	std::vector<V3f> &p = pData->writable();
	
	// add vertices
	float xStep = b.size().x / (float)divisions.x;
	float yStep = b.size().y / (float)divisions.y;
	for ( int i = 0; i <= divisions.y; ++i )
	{
		for ( int j = 0; j <= divisions.x; ++j )
		{
			p.push_back( V3f( b.min.x + j * xStep, b.min.y + i * yStep, 0 ) );
		}
	}
	
	IntVectorDataPtr vertexIds = new IntVectorData;
	IntVectorDataPtr verticesPerFace = new IntVectorData;
	std::vector<int> &vpf = verticesPerFace->writable();
	std::vector<int> &vIds = vertexIds->writable();
	
	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	std::vector<Imath::V2f> &uvs = uvData->writable();

	float uStep = 1.0f / (float)divisions.x;
	float vStep = 1.0f / (float)divisions.y;
	
	// add faces
	int v0, v1, v2, v3;
	for ( int i = 0; i < divisions.y; ++i )
	{
		for ( int j = 0; j < divisions.x; ++j )
		{
			v0 = j + (divisions.x+1) * i;
			v1 = j + 1 + (divisions.x+1) * i;;
			v2 = j + 1 + (divisions.x+1) * (i+1);
			v3 = j + (divisions.x+1) * (i+1);
			
			vpf.push_back( 4 );
			vIds.push_back( v0 );
			vIds.push_back( v1 );
			vIds.push_back( v2 );
			vIds.push_back( v3 );

			uvs.emplace_back( j * uStep, i * vStep );
			uvs.emplace_back( (j+1) * uStep, i * vStep );
			uvs.emplace_back( (j+1) * uStep, (i+1) * vStep );
			uvs.emplace_back( j * uStep, (i+1) * vStep );
		}
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFace, vertexIds, "linear", pData );
	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData );

	return result;
}

MeshPrimitivePtr MeshPrimitive::createSphere( float radius, float zMin, float zMax, float thetaMax, const Imath::V2i &divisions )
{
	IntVectorDataPtr vertexIds = new IntVectorData;
	IntVectorDataPtr verticesPerFace = new IntVectorData;
	std::vector<int> &vpf = verticesPerFace->writable();
	std::vector<int> &vIds = vertexIds->writable();
	
	V3fVectorDataPtr pData = new V3fVectorData;
	V3fVectorDataPtr nData = new V3fVectorData;
	std::vector<V3f> &pVector = pData->writable();
	std::vector<V3f> &nVector = nData->writable();

	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	std::vector<Imath::V2f> &uvs = uvData->writable();

	/// \todo: Rewrite this such that the poles are aligned to Y rather than Z.
	/// The centroid should remain at origin, uv(0,0) should be at the south
	/// pole (e.g -Y), uv(1,1) should be at the north pole (e.g. +Y). The stable
	/// seam (i.e. the edge that doesn't move when theta < 360) should remain
	/// aligned to +X with uv(0,0.5), and the moving edge should be uv(1,0.5).
	/// Remember to update SpherePrimitive at the same time.

	float oMin = Math<float>::asin( zMin );
	float oMax = Math<float>::asin( zMax );
	const unsigned int nO = max( 4u, (unsigned int)( ( divisions.x + 1 ) * (oMax - oMin) / M_PI ) );

	float thetaMaxRad = thetaMax / 180.0f * M_PI;
	const unsigned int nT = max( 7u, (unsigned int)( ( divisions.y + 1 ) * thetaMaxRad / (M_PI*2) ) );

	for ( unsigned int i=0; i<nO; i++ )
	{
		float v = (float)i/(float)(nO-1);
		float o = lerp( oMin, oMax, v );
		float z = radius * Math<float>::sin( o );
		float r = radius * Math<float>::cos( o );
		
		for ( unsigned int j=0; j<nT; j++ )
		{
			float u = (float)j/(float)(nT-1);
			float theta = thetaMaxRad * u;
			V3f p( r * Math<float>::cos( theta ), r * Math<float>::sin( theta ), z );
			uvs.emplace_back( u, v );
			pVector.push_back( p );
			nVector.push_back( p );
			if( i < nO - 1 && j < nT - 1 )
			{
				unsigned int i0 = i * nT + j;
				unsigned int i1 = i0 + 1;
				unsigned int i2 = i0 + nT;
				unsigned int i3 = i2 + 1;
				vpf.push_back( 3 );
				vIds.push_back( i0 );
				vIds.push_back( i1 );
				vIds.push_back( i2 );
				vpf.push_back( 3 );
				vIds.push_back( i1 );
				vIds.push_back( i3 );
				vIds.push_back( i2 );
			}
		}
	}
	
	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFace, vertexIds, "linear", pData );
	result->variables["N"] = PrimitiveVariable( PrimitiveVariable::Vertex, nData );
	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::Vertex, uvData );

	return result;
}
