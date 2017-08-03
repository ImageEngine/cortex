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

#include "boost/format.hpp"

#include "OpenEXR/ImathFun.h"

#include "IECore/MatrixMotionTransform.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace boost;
using namespace std;
using namespace Imath;

static IndexedIO::EntryID g_snapshotsEntry("snapshots");
static IndexedIO::EntryID g_timeEntry("time");
static IndexedIO::EntryID g_matrixEntry("matrix");

const unsigned int MatrixMotionTransform::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(MatrixMotionTransform);

MatrixMotionTransform::MatrixMotionTransform()
{
}

MatrixMotionTransform::~MatrixMotionTransform()
{
}

void MatrixMotionTransform::render( Renderer *renderer ) const
{
	if( !m_snapshots.size() )
	{
		return;
	}
	if( m_snapshots.size()==1 )
	{
		renderer->concatTransform( m_snapshots.begin()->second );
		return;
	}

	set<float> times;
	for( SnapshotMap::const_iterator it = m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		times.insert( it->first );
	}
	renderer->motionBegin( times );
		for( SnapshotMap::const_iterator it = m_snapshots.begin(); it!=m_snapshots.end(); it++ )
		{
			renderer->concatTransform( it->second );
		}
	renderer->motionEnd();
}

Imath::M44f MatrixMotionTransform::transform( float time ) const
{
	if( !m_snapshots.size() )
	{
		return M44f();
	}
	if( m_snapshots.size()==1 || time <= m_snapshots.begin()->first )
	{
		return m_snapshots.begin()->second;
	}
	if( time >= m_snapshots.rbegin()->first )
	{
		return m_snapshots.rbegin()->second;
	}
	SnapshotMap::const_iterator uIt = m_snapshots.upper_bound( time );
	SnapshotMap::const_iterator lIt = uIt;
	lIt--;

	/// \todo We should probably do something to interpolate rotations better.
	return lerp( lIt->second, uIt->second, lerpfactor( time, lIt->first, uIt->first ) );
}

const MatrixMotionTransform::SnapshotMap &MatrixMotionTransform::snapshots() const
{
	return m_snapshots;
}

MatrixMotionTransform::SnapshotMap &MatrixMotionTransform::snapshots()
{
	return m_snapshots;
}

void MatrixMotionTransform::copyFrom( const Object *other, CopyContext *context )
{
	Transform::copyFrom( other, context );
	const MatrixMotionTransform *t = static_cast<const MatrixMotionTransform *>( other );
	m_snapshots = t->m_snapshots;
}

void MatrixMotionTransform::save( SaveContext *context ) const
{
	Transform::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container = container->subdirectory( g_snapshotsEntry, IndexedIO::CreateIfMissing );
	int i = 0;
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		string is = str( boost::format( "%d" ) % i );
		IndexedIOPtr snapshotContainer = container->subdirectory( is, IndexedIO::CreateIfMissing );
		snapshotContainer->write( g_timeEntry, it->first );
		snapshotContainer->write( g_matrixEntry, it->second.getValue(), 16 );
		i++;
	}
}

void MatrixMotionTransform::load( LoadContextPtr context )
{
	Transform::load( context );
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	container = container->subdirectory( g_snapshotsEntry );
	m_snapshots.clear();
	IndexedIO::EntryIDList names;
	container->entryIds( names, IndexedIO::Directory );
	IndexedIO::EntryIDList::const_iterator it;
	for( it=names.begin(); it!=names.end(); it++ )
	{
		ConstIndexedIOPtr snapshotContainer = container->subdirectory( *it );
		float t; snapshotContainer->read( g_timeEntry, t );
		M44f m;
		float *f = m.getValue();
		snapshotContainer->read( g_matrixEntry, f, 16 );
		m_snapshots[t] = m;
	}
}

bool MatrixMotionTransform::isEqualTo( const Object *other ) const
{
	if( !Transform::isEqualTo( other ) )
	{
		return false;
	}
	const MatrixMotionTransform *t = static_cast<const MatrixMotionTransform *>( other );
	return m_snapshots == t->m_snapshots;
}

void MatrixMotionTransform::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Transform::memoryUsage( a );
	a.accumulate( sizeof( M44f ) * m_snapshots.size() );
	a.accumulate( sizeof( float ) * m_snapshots.size() );
}

void MatrixMotionTransform::hash( MurmurHash &h ) const
{
	Transform::hash( h );
	for( SnapshotMap::const_iterator it=m_snapshots.begin(); it!=m_snapshots.end(); it++ )
	{
		h.append( it->first );
		h.append( it->second );
	}
}
