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
	: m_verticesPerFace( new IntVectorData ), m_vertexIds( new IntVectorData ), m_numVertices( 0 ), m_interpolation( "linear" )
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

	vector<int>::const_iterator minIt = min_element( verticesPerFace->readable().begin(), verticesPerFace->readable().end() );
	if( minIt!=verticesPerFace->readable().end() )
	{
		if( *minIt<3 )
		{
			throw Exception( "Bad topology - number of vertices per face less than 3." );
		}
	}

	minIt = min_element( vertexIds->readable().begin(), vertexIds->readable().end() );
	{
		if( minIt!=vertexIds->readable().end() && *minIt<0 )
		{
			throw Exception( "Bad topology - vertexId less than 0." );
		}
	}

	unsigned int numExpectedVertexIds = accumulate( verticesPerFace->readable().begin(), verticesPerFace->readable().end(), 0 );
	if( numExpectedVertexIds!=vertexIds->readable().size() )
	{
		throw Exception( "Bad topology - number of vertexIds not equal to sum of verticesPerFace" );
	}

	m_verticesPerFace = verticesPerFace->copy();
	m_vertexIds = vertexIds->copy();
	if( m_vertexIds->readable().size() )
	{
		m_numVertices = 1 + *max_element( m_vertexIds->readable().begin(), m_vertexIds->readable().end() );
	}
	else
	{
		m_numVertices = 0;
	}
	m_interpolation = interpolation;
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
	context->save( m_verticesPerFace, container, g_verticesPerFaceEntry );
	context->save( m_vertexIds, container, g_vertexIdsEntry );

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

	m_verticesPerFace = context->load<IntVectorData>( container, g_verticesPerFaceEntry );
	m_vertexIds = context->load<IntVectorData>( container, g_vertexIdsEntry );

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
	if( !m_verticesPerFace->isEqualTo( tOther->m_verticesPerFace ) )
	{
		return false;
	}
	if( !m_vertexIds->isEqualTo( tOther->m_vertexIds ) )
	{
		return false;
	}

	return true;
}

void MeshPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( m_verticesPerFace );
	a.accumulate( m_vertexIds );
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
	
	FloatVectorDataPtr sData = new FloatVectorData;
	FloatVectorDataPtr tData = new FloatVectorData;
	std::vector<float> &s = sData->writable();
	std::vector<float> &t = tData->writable();
	
	float sStep = 1.0f / (float)divisions.x;
	float tStep = 1.0f / (float)divisions.y;
	
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
			
			s.push_back( j * sStep );
			s.push_back( (j+1) * sStep );
			s.push_back( (j+1) * sStep );
			s.push_back( j * sStep );
			
			t.push_back( 1 - i * tStep );
			t.push_back( 1 - i * tStep );
			t.push_back( 1 - (i+1) * tStep );
			t.push_back( 1 - (i+1) * tStep );
		}
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFace, vertexIds, "linear", pData );
	result->variables["s"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sData );
	result->variables["t"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, tData );

	return result;
}
