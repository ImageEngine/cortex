//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshPrimitive.h"
#include "IECore/Renderer.h"

#include <algorithm>
#include <numeric>

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

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
		variables.insert( PrimitiveVariableMap::value_type("P", PrimitiveVariable(PrimitiveVariable::Vertex, p)) );
	}
}

ConstIntVectorDataPtr MeshPrimitive::verticesPerFace() const
{
	return m_verticesPerFace;
}

ConstIntVectorDataPtr MeshPrimitive::vertexIds() const
{
	return m_vertexIds;
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

	if (vertexIds->readable().size() < 3)
	{
		throw Exception( "Bad topology - insufficient vertexIds." );
	}
	
	minIt = min_element( vertexIds->readable().begin(), vertexIds->readable().end() );
	{
		if( *minIt<0 )
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
		
size_t MeshPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation )
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

void MeshPrimitive::render( RendererPtr renderer )
{
	renderer->mesh( m_verticesPerFace, m_vertexIds, m_interpolation, variables );
}

void MeshPrimitive::copyFrom( ConstObjectPtr other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const MeshPrimitive *tOther = static_cast<const MeshPrimitive *>( other.get() );
	m_verticesPerFace = tOther->m_verticesPerFace->copy();
	m_vertexIds = tOther->m_vertexIds->copy();
	m_numVertices = tOther->m_numVertices;
	m_interpolation = tOther->m_interpolation;
}

void MeshPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_verticesPerFace, container, "verticesPerFace" );
	context->save( m_vertexIds, container, "vertexIds" );

	/// \todo: mac has problems with the size_t type, resulting in the write() call being
	/// ambiguous to the compiler
	unsigned int numVertices = m_numVertices;
	container->write( "numVertices", numVertices );

	container->write( "interpolation", m_interpolation );
}

void MeshPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;
	
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	m_verticesPerFace = context->load<IntVectorData>( container, "verticesPerFace" );
	m_vertexIds = context->load<IntVectorData>( container, "vertexIds" );

	unsigned int numVertices;
	container->read( "numVertices", numVertices );
	m_numVertices = numVertices;
	
	container->read( "interpolation", m_interpolation );
}

bool MeshPrimitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	
	const MeshPrimitive *tOther = static_cast<const MeshPrimitive *>( other.get() );
	
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

MeshPrimitivePtr MeshPrimitive::createBox( Box3f b )
{
	vector< int > verticesPerFaceVec;
	vector< int > vertexIdsVec;
	std::string interpolation = "linear";
	vector< V3f > p;
	int verticesPerFace[] = {
		4, 4, 4, 4, 4, 4
	};
	int vertexIds[] = {
		0,1,2,3,
		1,4,5,2,
		4,6,7,5,
		6,0,3,7,
		3,2,5,7,
		0,6,4,1
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

MeshPrimitivePtr MeshPrimitive::createPlane( Box2f b )
{
	IntVectorDataPtr verticesPerFace = new IntVectorData;
	verticesPerFace->writable().push_back( 4 );
	
	IntVectorDataPtr vertexIds = new IntVectorData;
	vertexIds->writable().push_back( 0 );
	vertexIds->writable().push_back( 1 );
	vertexIds->writable().push_back( 2 );
	vertexIds->writable().push_back( 3 );
	
	V3fVectorDataPtr p = new V3fVectorData;
	p->writable().push_back( V3f( b.min.x, b.min.y, 0 ) );
	p->writable().push_back( V3f( b.min.x, b.max.y, 0 ) );
	p->writable().push_back( V3f( b.max.x, b.max.y, 0 ) );
	p->writable().push_back( V3f( b.max.x, b.min.y, 0 ) );
	
	return new MeshPrimitive( verticesPerFace, vertexIds, "linear", p );
}
