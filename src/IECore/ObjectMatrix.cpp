//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Cinesite VFX Ltd. All rights reserved.
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

#include "IECore/ObjectMatrix.h"

#include "IECore/MurmurHash.h"

using namespace IECore;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( ObjectMatrix );

ObjectMatrix::ObjectMatrix( size_t rows, size_t columns )
	: m_rows( rows ), m_columns( columns )
{
	m_members.resize( rows * columns, nullptr );
}

ObjectMatrix::~ObjectMatrix()
{
}

size_t ObjectMatrix::numRows() const
{
	return m_rows;
}

size_t ObjectMatrix::numColumns() const
{
	return m_columns;
}

void ObjectMatrix::resize( size_t rows, size_t columns )
{
	MemberContainer originalMembers( m_members );
	m_members.clear();
	m_members.resize( rows * columns, nullptr );
	for( size_t i = 0; i < rows && i < m_rows; ++i )
	{
		for( size_t j = 0; j < columns && j < m_columns; ++j )
		{
			m_members[ i * columns + j ] = originalMembers[ i * m_columns + j ];
		}
	}
	m_rows = rows;
	m_columns = columns;
}

void ObjectMatrix::copyFrom( const Object *other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const ObjectMatrix *tOther = static_cast<const ObjectMatrix *>( other );
	m_members.resize( tOther->m_members.size() );
	for( MemberContainer::size_type i = 0; i < m_members.size(); ++i )
	{
		if( !tOther->m_members[i] )
		{
			m_members[i] = nullptr;
		}
		else
		{
			m_members[i] = context->copy<Object>( tOther->m_members[i].get() );
		}
	}
	m_rows = tOther->m_rows;
	m_columns = tOther->m_columns;
}

void ObjectMatrix::save( Object::SaveContext *context ) const
{
	Object::save( context );
	throw IECore::NotImplementedException( "ObjectMatrix::save" );
}

void ObjectMatrix::load( Object::LoadContextPtr context )
{
	Object::load( context );
	throw IECore::NotImplementedException( "ObjectMatrix::load" );
}

bool ObjectMatrix::isEqualTo( const Object *other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	const ObjectMatrix *tOther = static_cast<const ObjectMatrix *>( other );
	if( m_rows != tOther->m_rows || m_columns != tOther->m_columns )
	{
		return false;
	}
	for( MemberContainer::size_type i = 0; i < m_members.size(); ++i )
	{
		if( m_members[i] )
		{
			if( !tOther->m_members[i] || !m_members[i]->isEqualTo( tOther->m_members[i].get() ) )
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

void ObjectMatrix::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );
	for( const auto &m : m_members )
	{
		if( m )
		{
			a.accumulate( m.get() );
		}
	}
	a.accumulate( sizeof( m_rows ) );
	a.accumulate( sizeof( m_columns ) );
}

void ObjectMatrix::hash( MurmurHash &h ) const
{
	Object::hash( h );
	for( const auto &m : m_members )
	{
		if( m )
		{
			m->hash( h );
		}
		else
		{
			h.append( 0 );
		}
	}
	h.append( m_rows );
	h.append( m_columns );
}
