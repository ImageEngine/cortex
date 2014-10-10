//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ObjectVector.h"
#include "IECore/MurmurHash.h"

#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"

using namespace IECore;

static IndexedIO::EntryID g_sizeEntry("size");
static IndexedIO::EntryID g_membersEntry("members");
const unsigned int ObjectVector::m_ioVersion = 1;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( ObjectVector );

ObjectVector::ObjectVector()
{
}

ObjectVector::~ObjectVector()
{
}

ObjectVector::MemberContainer &ObjectVector::members()
{
	return m_members;
}

const ObjectVector::MemberContainer &ObjectVector::members() const
{
	return m_members;
}

void ObjectVector::copyFrom( const Object *other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const ObjectVector *tOther = static_cast<const ObjectVector *>( other );
	m_members.resize( tOther->m_members.size() );
	for( unsigned i=0; i<m_members.size(); i++ )
	{
		if( !tOther->m_members[i] )
		{
			m_members[i] = 0;
		}
		else
		{
			m_members[i] = context->copy<Object>( tOther->m_members[i].get() );
		}
	}
}

void ObjectVector::save( SaveContext *context ) const
{
	Object::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );

	unsigned int size = m_members.size();
	container->write( g_sizeEntry, size );

	IndexedIOPtr ioMembers = container->subdirectory( g_membersEntry, IndexedIO::CreateIfMissing );

	unsigned i=0;
	for( MemberContainer::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		if( *it )
		{
			std::string name = str( boost::format( "%d" ) % i );
			context->save( it->get(), ioMembers.get(), name );
		}
		i++;
	}
}

void ObjectVector::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	unsigned int size = 0;
	container->read( g_sizeEntry, size );

	m_members.resize( size );
	std::fill( m_members.begin(), m_members.end(), (IECore::Object*)0 );

	ConstIndexedIOPtr ioMembers = container->subdirectory( g_membersEntry );

	IndexedIO::EntryIDList l;
	ioMembers->entryIds(l);
	for( IndexedIO::EntryIDList::const_iterator it=l.begin(); it!=l.end(); it++ )
	{
		MemberContainer::size_type i = boost::lexical_cast<MemberContainer::size_type>( (*it).value() );
		m_members[i] = context->load<Object>( ioMembers.get(), *it );
	}
}

bool ObjectVector::isEqualTo( const Object *other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	const ObjectVector *tOther = static_cast<const ObjectVector *>( other );
	if( m_members.size()!=tOther->m_members.size() )
	{
		return false;
	}
	for( MemberContainer::size_type i=0; i<m_members.size(); i++ )
	{
		if( m_members[i] )
		{
			if( !m_members[i]->isEqualTo( tOther->m_members[i].get() ) )
			{
				return false;
			}
		}
		else
		{
			if( tOther->m_members[i] )
			{
				return false;
			}
		}
	}
	return true;
}

void ObjectVector::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );
	for( MemberContainer::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		if( *it )
		{
			a.accumulate( it->get() );
		}
	}
}

void ObjectVector::hash( MurmurHash &h ) const
{
	Object::hash( h );
	for( MemberContainer::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		if( *it )
		{
			(*it)->hash( h );
		}
		else
		{
			h.append( 0 );
		}
	}
}
