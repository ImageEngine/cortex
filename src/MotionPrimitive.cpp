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

#include "IECore/MotionPrimitive.h"
#include "IECore/Renderer.h"

#include "boost/format.hpp"

#include <algorithm>

using namespace IECore;
using namespace std;

const unsigned int MotionPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( MotionPrimitive );

MotionPrimitive::MotionPrimitive()
{
}

MotionPrimitive::~MotionPrimitive()
{
}

void MotionPrimitive::addSnapshot( float time, PrimitivePtr primitive )
{
	m_snapshots[time] = primitive;
}

void MotionPrimitive::removeSnapshot( float time )
{
	m_snapshots.erase( time );
}

void MotionPrimitive::removeSnapshot( PrimitivePtr primitive )
{
	SnapshotMap::iterator it=m_snapshots.begin();
	while( it!=m_snapshots.end() )
	{
		SnapshotMap::iterator next = it; next++;
		if( it->second==primitive )
		{
			m_snapshots.erase( it );
		}
		it = next;
	}
}

const MotionPrimitive::SnapshotMap &MotionPrimitive::snapshots() const
{
	return m_snapshots;
}


MotionPrimitive::SnapshotMap &MotionPrimitive::snapshots()
{
	return m_snapshots;
}

void MotionPrimitive::render( RendererPtr renderer )
{
	if( !m_snapshots.size() )
	{
		return;
	}
	if( m_snapshots.size()==1 )
	{
		m_snapshots.begin()->second->render( renderer );
		return;
	}

	TypeId t = m_snapshots.begin()->second->typeId();
	size_t cs = m_snapshots.begin()->second->variableSize( PrimitiveVariable::Constant );
	size_t us = m_snapshots.begin()->second->variableSize( PrimitiveVariable::Uniform );
	size_t vs = m_snapshots.begin()->second->variableSize( PrimitiveVariable::Varying );
	size_t ves = m_snapshots.begin()->second->variableSize( PrimitiveVariable::Vertex );
	size_t fs = m_snapshots.begin()->second->variableSize( PrimitiveVariable::FaceVarying );
	
	set<float> times;
	for( SnapshotMap::iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		if( it->second->typeId()!=t )
		{
			throw Exception( "Primitive types do not match." );
		}
		if( it->second->variableSize( PrimitiveVariable::Constant )!=cs ||
			it->second->variableSize( PrimitiveVariable::Uniform )!=us ||
			it->second->variableSize( PrimitiveVariable::Varying )!=vs ||
			it->second->variableSize( PrimitiveVariable::Vertex )!=ves ||
			it->second->variableSize( PrimitiveVariable::FaceVarying )!=fs
		)
		{
			throw Exception( "Primitive variable sizes do not match." );
		}
		times.insert( it->first );
	}
	renderer->motionBegin( times );
		for( SnapshotMap::iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
		{
			it->second->render( renderer );
		}
	renderer->motionEnd();
}

Imath::Box3f MotionPrimitive::bound() const
{
	Imath::Box3f result;
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		result.extendBy( it->second->bound() );
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Object interface
////////////////////////////////////////////////////////////////////////////////////////////

void MotionPrimitive::copyFrom( IECore::ConstObjectPtr other, IECore::Object::CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const MotionPrimitive *tOther = static_cast<const MotionPrimitive *>( other.get() );
	m_snapshots.clear();
	for( SnapshotMap::const_iterator it=tOther->m_snapshots.begin(); it!=tOther->m_snapshots.end(); it++ )
	{
		m_snapshots[it->first] = context->copy<Primitive>( it->second );
	}
}

void MotionPrimitive::save( IECore::Object::SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->mkdir( "snapshots" );
	container->chdir( "snapshots" );
		int i = 0;
		for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
		{
			string is = str( boost::format( "%d" ) % i );
			container->mkdir( is );
			container->chdir( is );
				container->write( "time", it->first );
				context->save( it->second, container, "primitive" );
			container->chdir( ".." );
			i++;
		}
	container->chdir( ".." );
}

void MotionPrimitive::load( IECore::Object::LoadContextPtr context )
{
	VisibleRenderable::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	container->chdir( "snapshots" );
		m_snapshots.clear();
		IndexedIO::EntryList names = container->ls();
		IndexedIO::EntryList::const_iterator it;
		for( it=names.begin(); it!=names.end(); it++ )
		{
			container->chdir( it->id() );
				float t; container->read( "time", t );
				m_snapshots[t] = context->load<Primitive>( container, "primitive" );
			container->chdir( ".." );
		}
	container->chdir( ".." );
}

bool MotionPrimitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const MotionPrimitive *tOther = static_cast<const MotionPrimitive *>( other.get() );
	if( tOther->m_snapshots.size()!=m_snapshots.size() )
	{
		return false;
	}
	SnapshotMap::const_iterator tIt = m_snapshots.begin();
	SnapshotMap::const_iterator oIt = tOther->m_snapshots.begin();
	while( tIt!=m_snapshots.end() )
	{
		if( tIt->first!=oIt->first )
		{
			return false;
		}
		if( !tIt->second->isEqualTo( oIt->second ) )
		{
			return false;
		}
		tIt++;
		oIt++;
	}
	return true;
}

void MotionPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	VisibleRenderable::memoryUsage( a );
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		a.accumulate( it->second );
	}
}
