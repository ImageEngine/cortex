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

#include "IECore/CompoundObject.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CompoundObject );

const unsigned int CompoundObject::m_ioVersion = 0;	

CompoundObject::CompoundObject()
{
}

CompoundObject::~CompoundObject()
{
}

const CompoundObject::ObjectMap &CompoundObject::members() const
{
	return m_members;
}

CompoundObject::ObjectMap &CompoundObject::members()
{
	return m_members;
}

void CompoundObject::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const CompoundObject *tOther = static_cast<const CompoundObject *>( other.get() );
	m_members.clear();
	for( ObjectMap::const_iterator it=tOther->m_members.begin(); it!=tOther->m_members.end(); it++ )
	{
		m_members[it->first] = context->copy<Object>( it->second );
	}
}

void CompoundObject::save( SaveContext *context ) const
{
	Object::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->mkdir( "members" );
	container->chdir( "members" );
		ObjectMap::const_iterator it;
		for( it=m_members.begin(); it!=m_members.end(); it++ )
		{
			context->save( it->second, container, it->first );
		}
	container->chdir( ".." );
}

void CompoundObject::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	m_members.clear();
	container->chdir( "members" );
		IndexedIO::EntryList l = container->ls();
		IndexedIO::EntryList::const_iterator it;
		for( it=l.begin(); it!=l.end(); it++ )
		{
			m_members[it->id()] = context->load<Object>( container, it->id() );
		}
	container->chdir( ".." );
}

bool CompoundObject::isEqualTo( ConstObjectPtr other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	boost::intrusive_ptr<const CompoundObject> tOther = boost::static_pointer_cast<const CompoundObject>( other );
	if( m_members.size()!=tOther->m_members.size() )
	{
		return false;
	}
	ObjectMap::const_iterator it1 = m_members.begin();
	ObjectMap::const_iterator it2 = tOther->m_members.begin();
	while( it1!=m_members.end() )
	{
		if( it1->first!=it2->first )
		{
			return false;
		}
		if( ! it1->second->isEqualTo( it2->second ) )
		{
			return false;
		}
		it1++;
		it2++;
	}
	return true;
}

void CompoundObject::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );
	a.accumulate( sizeof( ObjectMap ) );
	for( ObjectMap::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		a.accumulate( it->first.capacity() );
		a.accumulate( it->second );
	}
}
