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

#include "IECoreScene/MeshPrimitive.h"

#include "IECoreScene/PolygonIterator.h"
#include "IECoreScene/Renderer.h"

#include "IECore/Math.h"
#include "IECore/MurmurHash.h"

#include "boost/format.hpp"

#include <algorithm>
#include <numeric>

using namespace std;
using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace boost;

namespace
{

IndexedIO::EntryID g_verticesPerFaceEntry("verticesPerFace");
IndexedIO::EntryID g_vertexIdsEntry("vertexIds");
IndexedIO::EntryID g_numVerticesEntry("numVertices");
IndexedIO::EntryID g_interpolationEntry("interpolation");
IndexedIO::EntryID g_cornerIdsEntry("cornerIds");
IndexedIO::EntryID g_cornerSharpnessesEntry("cornerSharpnesses");
IndexedIO::EntryID g_creaseLengthsEntry("creaseLengths");
IndexedIO::EntryID g_creaseIdsEntry("creaseIds");
IndexedIO::EntryID g_creaseSharpnessesEntry("creaseSharpnesses");

const IntVectorData *emptyIntVectorData()
{
	static IntVectorDataPtr g_d = new IntVectorData;
	return g_d.get();
}

const FloatVectorData *emptyFloatVectorData()
{
	static FloatVectorDataPtr g_d = new FloatVectorData;
	return g_d.get();
}

} // namespace

const unsigned int MeshPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(MeshPrimitive);

MeshPrimitive::MeshPrimitive()
	: m_verticesPerFace( new IntVectorData ), m_vertexIds( new IntVectorData ), m_numVertices( 0 ), m_interpolation( "linear" ), m_minVerticesPerFace( 0 ), m_maxVerticesPerFace( 0 )
{
	removeCorners();
	removeCreases();
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

	// Assume that changing the topology has invalidated
	// the corners and creases.
	removeCorners();
	removeCreases();
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

void MeshPrimitive::setCorners( const IECore::IntVectorData *ids, const IECore::FloatVectorData *sharpnesses )
{
	for( auto id : ids->readable() )
	{
		if( id < 0 || (size_t)id >= m_numVertices )
		{
			throw Exception( boost::str(
				boost::format(
					"Bad corners : id (%1%) is out of expected range (0-%2%)"
				) % id % (m_numVertices - 1)
			) );
		}
	}

	if( sharpnesses->readable().size() != ids->readable().size() )
	{
		throw Exception( boost::str(
			boost::format(
				"Bad corners : number of sharpnesses (%1%) does not match number of ids (%2%)"
			) % sharpnesses->readable().size() % ids->readable().size()
		) );
	}

	m_cornerIds = ids->copy();
	m_cornerSharpnesses = sharpnesses->copy();
}

const IECore::IntVectorData *MeshPrimitive::cornerIds() const
{
	return m_cornerIds ? m_cornerIds.get() : emptyIntVectorData();
}

const IECore::FloatVectorData *MeshPrimitive::cornerSharpnesses() const
{
	return m_cornerSharpnesses ? m_cornerSharpnesses.get() : emptyFloatVectorData();
}

void MeshPrimitive::removeCorners()
{
	m_cornerIds = emptyIntVectorData();
	m_cornerSharpnesses = emptyFloatVectorData();
}

void MeshPrimitive::setCreases( const IECore::IntVectorData *lengths, const IECore::IntVectorData *ids, const IECore::FloatVectorData *sharpnesses )
{
	size_t expectedIds = 0;
	for( auto length : lengths->readable() )
	{
		if( length < 2 )
		{
			throw Exception( boost::str(
				boost::format(
					"Bad creases : length (%1%) is less than 2"
				) % length
			) );
		}
		expectedIds += length;
	}

	if( ids->readable().size() != expectedIds )
	{
		throw Exception( boost::str(
			boost::format(
				"Bad creases : expected %1% ids but given %2%"
			) % expectedIds % ids->readable().size()
		) );
	}

	for( auto id : ids->readable() )
	{
		if( id < 0 || (size_t)id >= m_numVertices )
		{
			throw Exception( boost::str(
				boost::format(
					"Bad creases : id (%1%) is out of expected range (0-%2%)"
				) % id % (m_numVertices - 1)
			) );
		}
	}

	if( sharpnesses->readable().size() != lengths->readable().size() )
	{
		throw Exception( boost::str(
			boost::format(
				"Bad creases : number of sharpnesses (%1%) does not match number of lengths (%2%)"
			) % sharpnesses->readable().size() % lengths->readable().size()
		) );
	}

	m_creaseLengths = lengths->copy();
	m_creaseIds = ids->copy();
	m_creaseSharpnesses = sharpnesses->copy();
}

const IECore::IntVectorData *MeshPrimitive::creaseLengths() const
{
	return m_creaseLengths ? m_creaseLengths.get() : emptyIntVectorData();
}

const IECore::IntVectorData *MeshPrimitive::creaseIds() const
{
	return m_creaseIds ? m_creaseIds.get() : emptyIntVectorData();
}

const IECore::FloatVectorData *MeshPrimitive::creaseSharpnesses() const
{
	return m_creaseSharpnesses ? m_creaseSharpnesses.get() : emptyFloatVectorData();
}

void MeshPrimitive::removeCreases()
{
	m_creaseLengths = emptyIntVectorData();
	m_creaseIds = emptyIntVectorData();
	m_creaseSharpnesses = emptyFloatVectorData();
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
	// Because our corners and creases are stored as const data, we
	// don't need to take copies because they will never be modified
	// anyway.
	/// \todo Could we do the same for m_vertexIds and m_numVertices?
	m_cornerIds = tOther->m_cornerIds;
	m_cornerSharpnesses = tOther->m_cornerSharpnesses;
	m_creaseLengths = tOther->m_creaseLengths;
	m_creaseIds = tOther->m_creaseIds;
	m_creaseSharpnesses = tOther->m_creaseSharpnesses;
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

	if( m_cornerIds->readable().size() )
	{
		context->save( m_cornerIds.get(), container.get(), g_cornerIdsEntry );
		context->save( m_cornerSharpnesses.get(), container.get(), g_cornerSharpnessesEntry );
	}

	if( m_creaseLengths->readable().size() )
	{
		context->save( m_creaseLengths.get(), container.get(), g_creaseLengthsEntry );
		context->save( m_creaseIds.get(), container.get(), g_creaseIdsEntry );
		context->save( m_creaseSharpnesses.get(), container.get(), g_creaseSharpnessesEntry );
	}
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

	if( container->hasEntry( g_cornerIdsEntry ) )
	{
		m_cornerIds = context->load<IntVectorData>( container.get(), g_cornerIdsEntry );
		m_cornerSharpnesses = context->load<FloatVectorData>( container.get(), g_cornerSharpnessesEntry );
	}
	else
	{
		removeCorners();
	}

	if( container->hasEntry( g_creaseLengthsEntry ) )
	{
		m_creaseLengths = context->load<IntVectorData>( container.get(), g_creaseLengthsEntry );
		m_creaseIds = context->load<IntVectorData>( container.get(), g_creaseIdsEntry );
		m_creaseSharpnesses = context->load<FloatVectorData>( container.get(), g_creaseSharpnessesEntry );
	}
	else
	{
		removeCreases();
	}
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
	if( !m_cornerIds->isEqualTo( tOther->m_cornerIds.get() ) )
	{
		return false;
	}
	if( !m_cornerSharpnesses->isEqualTo( tOther->m_cornerSharpnesses.get() ) )
	{
		return false;
	}
	if( !m_creaseLengths->isEqualTo( tOther->m_creaseLengths.get() ) )
	{
		return false;
	}
	if( !m_creaseIds->isEqualTo( tOther->m_creaseIds.get() ) )
	{
		return false;
	}
	if( !m_creaseSharpnesses->isEqualTo( tOther->m_creaseSharpnesses.get() ) )
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
	a.accumulate( m_cornerIds.get() );
	a.accumulate( m_cornerSharpnesses.get() );
	a.accumulate( m_creaseLengths.get() );
	a.accumulate( m_creaseIds.get() );
	a.accumulate( m_creaseSharpnesses.get() );
}

void MeshPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
	m_cornerIds->hash( h );
	m_cornerSharpnesses->hash( h );
	m_creaseLengths->hash( h );
	m_creaseIds->hash( h );
	m_creaseSharpnesses->hash( h );
}

void MeshPrimitive::topologyHash( MurmurHash &h ) const
{
	m_verticesPerFace->hash( h );
	m_vertexIds->hash( h );
	h.append( m_interpolation );
}

MeshPrimitivePtr MeshPrimitive::createBox( const Box3f &b )
{
	std::string interpolation = "linear";
	vector< V3f > p;
	std::vector<int> verticesPerFace {
		4, 4, 4, 4, 4, 4
	};
	std::vector<int> vertexIds {
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

	MeshPrimitivePtr result = new MeshPrimitive( new IntVectorData(verticesPerFace), new IntVectorData(vertexIds), interpolation, new V3fVectorData(p) );


	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	std::vector<Imath::V2f> &uvs = uvData->writable();

	for( int i = 0; i < 5; i++ )
	{
		uvs.push_back( Imath::V2f( 0.375f, 0.25f * i ) );
		uvs.push_back( Imath::V2f( 0.625f, 0.25f * i ) );
	}

	for( int i = 0; i < 2; i++ )
	{
		uvs.push_back( Imath::V2f( 0.125f, 0.25f * i ) );
		uvs.push_back( Imath::V2f( 0.875f, 0.25f * i ) );
	}

	std::vector<int> uvIndices {
		4,5,7,6,
		11,13,3,1,
		1,3,2,0,
		0,2,12,10,
		5,4,2,3,
		6,7,9,8,
	};

	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData, new IntVectorData ( uvIndices ) );

	return result;
}

MeshPrimitivePtr MeshPrimitive::createPlane( const Box2f &b, const Imath::V2i &divisions )
{
	V3fVectorDataPtr pData = new V3fVectorData;
	std::vector<V3f> &p = pData->writable();

	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	std::vector<Imath::V2f> &uvs = uvData->writable();

	// add vertices

	const float xStep = b.size().x / (float)divisions.x;
	const float yStep = b.size().y / (float)divisions.y;
	const float uStep = 1.0f / (float)divisions.x;
	const float vStep = 1.0f / (float)divisions.y;
	for ( int i = 0; i <= divisions.y; ++i )
	{
		for ( int j = 0; j <= divisions.x; ++j )
		{
			p.push_back( V3f( b.min.x + j * xStep, b.min.y + i * yStep, 0 ) );
			uvs.push_back( V2f( j * uStep, i * vStep ) );
		}
	}

	IntVectorDataPtr vertexIds = new IntVectorData;
	IntVectorDataPtr verticesPerFace = new IntVectorData;
	std::vector<int> &vpf = verticesPerFace->writable();
	std::vector<int> &vIds = vertexIds->writable();

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
		}
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFace, vertexIds, "linear", pData );
	// We could use Vertex interpolation here, since that's logicaly the same
	// as FaceVarying interpolation using `vertexIds` as the indices. We don't
	// do that for a few reasons :
	//
	//   - Most DCCs represent UVs using explicit indices anyway
	//   - Some of our code is currently incompatible with Vertex UVs
	//   - This gives us extra test coverage for indexed UVs "for free",
	//     since several tests use `createPlane()`.
	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData, vertexIds );

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
	nData->setInterpretation( GeometricData::Normal );
	std::vector<V3f> &pVector = pData->writable();
	std::vector<V3f> &nVector = nData->writable();

	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	IntVectorDataPtr uvIndicesData = new IntVectorData;
	std::vector<Imath::V2f> &uvs = uvData->writable();
	std::vector<int> &uvIndices = uvIndicesData->writable();

	/// \todo: Rewrite this such that the poles are aligned to Y rather than Z.
	/// The centroid should remain at origin, uv(0,0) should be at the south
	/// pole (e.g -Y), uv(1,1) should be at the north pole (e.g. +Y). The stable
	/// seam (i.e. the edge that doesn't move when theta < 360) should remain
	/// aligned to +X with uv(0,0.5), and the moving edge should be uv(1,0.5).
	/// Remember to update SpherePrimitive at the same time.

	float oMin = Math<float>::asin( zMin );
	float oMax = Math<float>::asin( zMax );
	const unsigned int nO = max( 4u, (unsigned int)round( divisions.x * (oMax - oMin) / M_PI ) + 1 );

	float thetaMaxRad = thetaMax / 180.0f * M_PI;
	const unsigned int nT = max( 7u, (unsigned int)round( divisions.y * thetaMaxRad / (M_PI*2) ) + 1 );

	for ( unsigned int i=0; i<nO; i++ )
	{
		float v = (float)i/(float)(nO-1);
		float o = lerp( oMin, oMax, v );
		float z = radius * Math<float>::sin( o );
		float r = radius * Math<float>::cos( o );

		const bool atPole =
			( i == 0 && zMin == -1 ) ||
			( i == nO - 1 && zMax == 1 )
		;

		const int baseVertexId = pVector.size();
		const int baseUVIndex = uvs.size();

		for ( unsigned int j=0; j<nT; j++ )
		{
			float u = (float)j/(float)(nT-1);
			if( atPole )
			{
				u += 0.5f / (float)(nT-1);
			}

			float theta = thetaMaxRad * u;

			// Add vertex P

			const bool addP = atPole ? j == 0 : !(thetaMax == 360 && j == nT - 1 );
			if( addP )
			{
				V3f p( r * Math<float>::cos( theta ), r * Math<float>::sin( theta ), z );
				pVector.push_back( p );
				nVector.push_back( p );
			}

			// Add facevarying UV

			const bool addUV = atPole ? j < nT - 1 : true;
			if( addUV )
			{
				uvs.emplace_back( u, v );
			}

			// Add face. We use triangles at the end caps
			// and quads in between.

			if( i < nO - 1 && j < nT - 1 )
			{
				const int rowStride = thetaMax == 360 ? nT - 1 : nT;
				if( i == 0 && zMin == -1 )
				{
					vpf.push_back( 3 );

					vIds.push_back( 0 );
					vIds.push_back( 1 + ( j + 1 ) % rowStride );
					vIds.push_back( 1 + j );

					uvIndices.push_back( baseUVIndex + j );
					uvIndices.push_back( baseUVIndex + nT + j );
					uvIndices.push_back( baseUVIndex + nT - 1 + j );
				}
				else if( i == nO - 2 && zMax == 1 )
				{
					vpf.push_back( 3 );

					vIds.push_back( baseVertexId + j );
					vIds.push_back( baseVertexId + ( j + 1 ) % rowStride );
					vIds.push_back( baseVertexId + rowStride );

					uvIndices.push_back( baseUVIndex + j );
					uvIndices.push_back( baseUVIndex + j + 1 );
					uvIndices.push_back( baseUVIndex + nT + j );
				}
				else
				{
					vpf.push_back( 4 );

					vIds.push_back( baseVertexId + j );
					vIds.push_back( baseVertexId + ( j + 1 ) % rowStride );
					vIds.push_back( baseVertexId + rowStride + ( j + 1 ) % rowStride );
					vIds.push_back( baseVertexId + rowStride + j );

					uvIndices.push_back( baseUVIndex + j );
					uvIndices.push_back( baseUVIndex + j + 1 );
					uvIndices.push_back( baseUVIndex + nT + j + 1 );
					uvIndices.push_back( baseUVIndex + nT + j );
				}
			}
		}
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFace, vertexIds, "linear", pData );
	result->variables["N"] = PrimitiveVariable( PrimitiveVariable::Vertex, nData );
	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData, uvIndicesData );

	return result;
}
