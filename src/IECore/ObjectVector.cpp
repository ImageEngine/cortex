//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"

using namespace IECore;

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

void ObjectVector::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const ObjectVector *tOther = static_cast<const ObjectVector *>( other.get() );
	m_members.resize( tOther->m_members.size() );
	for( unsigned i=0; i<m_members.size(); i++ )
	{
		if( !tOther->m_members[i] )
		{
			m_members[i] = 0;
		}
		else
		{
			m_members[i] = context->copy<Object>( tOther->m_members[i] );
		}
	}
}

void ObjectVector::save( SaveContext *context ) const
{
	Object::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	
	container->write( "size", m_members.size() );
	
	container->mkdir( "members" );
	container->chdir( "members" );
	
		unsigned i=0;
		for( MemberContainer::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
		{
			if( *it )
			{
				std::string name = str( boost::format( "%d" ) % i );
				context->save( *it, container, name );
			}
			i++;
		}
	
	container->chdir( ".." );
}

void ObjectVector::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	MemberContainer::size_type size = m_members.size();
	container->read( "size", size );
	
	m_members.resize( size );
	std::fill( m_members.begin(), m_members.end(), (IECore::Object*)0 );
	
	container->chdir( "members" );
	
		IndexedIO::EntryList l = container->ls();
		for( IndexedIO::EntryList::const_iterator it=l.begin(); it!=l.end(); it++ )
		{
			MemberContainer::size_type i = boost::lexical_cast<MemberContainer::size_type>( it->id() );
			m_members[i] = context->load<Object>( container, it->id() );
		}
	
	container->chdir( ".." );

}

bool ObjectVector::isEqualTo( ConstObjectPtr other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	ObjectVector::ConstPtr tOther = boost::static_pointer_cast<const ObjectVector>( other );
	if( m_members.size()!=tOther->m_members.size() )
	{
		return false;
	}
	for( MemberContainer::size_type i=0; i<m_members.size(); i++ )
	{
		if( m_members[i] )
		{
			if( !m_members[i]->isEqualTo( tOther->m_members[i] ) )
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
			a.accumulate( *it );
		}
	}
}
