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

#include "boost/format.hpp"

#include "IECore/MotionPrimitive.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace std;

static IndexedIO::EntryID g_snapshotsEntry("snapshots");
static IndexedIO::EntryID g_primitiveEntry("primitive");
static IndexedIO::EntryID g_timeEntry("time");

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

void MotionPrimitive::render( Renderer *renderer ) const
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
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
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
		for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
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

void MotionPrimitive::copyFrom( const Object *other, Object::CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const MotionPrimitive *tOther = static_cast<const MotionPrimitive *>( other );
	m_snapshots.clear();
	for( SnapshotMap::const_iterator it=tOther->m_snapshots.begin(); it!=tOther->m_snapshots.end(); it++ )
	{
		m_snapshots[it->first] = context->copy<Primitive>( it->second.get() );
	}
}

void MotionPrimitive::save( IECore::Object::SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	IndexedIOPtr snapshots = container->subdirectory( g_snapshotsEntry, IndexedIO::CreateIfMissing );

	int i = 0;
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		string is = str( boost::format( "%d" ) % i );
		IndexedIOPtr snapshot = snapshots->subdirectory( is, IndexedIO::CreateIfMissing );
		snapshot->write( g_timeEntry, it->first );
		context->save( it->second.get(), snapshot.get(), g_primitiveEntry );
		i++;
	}
}

void MotionPrimitive::load( IECore::Object::LoadContextPtr context )
{
	VisibleRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	ConstIndexedIOPtr snapshots = container->subdirectory( g_snapshotsEntry );
	m_snapshots.clear();
	IndexedIO::EntryIDList names;
	snapshots->entryIds( names, IndexedIO::Directory );
	IndexedIO::EntryIDList::const_iterator it;
	for( it=names.begin(); it!=names.end(); it++ )
	{
		ConstIndexedIOPtr snapshot = snapshots->subdirectory( *it );
		float t; 
		snapshot->read( g_timeEntry, t );
		m_snapshots[t] = context->load<Primitive>( snapshot.get(), g_primitiveEntry );
	}
}

bool MotionPrimitive::isEqualTo( const Object *other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const MotionPrimitive *tOther = static_cast<const MotionPrimitive *>( other );
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
		if( !tIt->second->isEqualTo( oIt->second.get() ) )
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
		a.accumulate( it->second.get() );
	}
}

void MotionPrimitive::hash( MurmurHash &h ) const
{
	VisibleRenderable::hash( h );
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		h.append( it->first );
		it->second->hash( h );	
	}
}
