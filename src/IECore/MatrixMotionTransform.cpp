//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MatrixMotionTransform.h"
#include "IECore/Renderer.h"

#include "OpenEXR/ImathFun.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace boost;
using namespace std;
using namespace Imath;

const unsigned int MatrixMotionTransform::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(MatrixMotionTransform);

MatrixMotionTransform::MatrixMotionTransform()
{
}

MatrixMotionTransform::~MatrixMotionTransform()
{
}

void MatrixMotionTransform::render( RendererPtr renderer ) const
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
	SnapshotMap::const_reverse_iterator lIt( uIt );
	lIt++;
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

void MatrixMotionTransform::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Transform::copyFrom( other, context );
	const MatrixMotionTransform *t = static_cast<const MatrixMotionTransform *>( other.get() );
	m_snapshots = t->m_snapshots;
}

void MatrixMotionTransform::save( SaveContext *context ) const
{
	Transform::save( context );
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
				container->write( "matrix", it->second.getValue(), 16 );
			container->chdir( ".." );
			i++;
		}
	container->chdir( ".." );
}

void MatrixMotionTransform::load( LoadContextPtr context )
{
	Transform::load( context );
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
				M44f m;
				float *f = m.getValue();
				container->read( "matrix", f, 16 );
				m_snapshots[t] = m;
			container->chdir( ".." );
		}
	container->chdir( ".." );
}

bool MatrixMotionTransform::isEqualTo( ConstObjectPtr other ) const
{
	if( !Transform::isEqualTo( other ) )
	{
		return false;
	}
	ConstMatrixMotionTransformPtr t = static_pointer_cast<const MatrixMotionTransform>( other );
	return m_snapshots == t->m_snapshots;
}

void MatrixMotionTransform::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Transform::memoryUsage( a );
	a.accumulate( sizeof( M44f ) * m_snapshots.size() );
	a.accumulate( sizeof( float ) * m_snapshots.size() );
}
