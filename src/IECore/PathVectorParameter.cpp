//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PathVectorParameter.h"
#include "IECore/CompoundObject.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <algorithm>

using namespace boost;
using namespace std;
using namespace IECore;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( PathVectorParameter );
const unsigned int PathVectorParameter::g_ioVersion = 1;	

PathVectorParameter::PathVectorParameter()
{
}

PathVectorParameter::PathVectorParameter( const std::string &name, const std::string &description,
                              const std::vector<std::string> &defaultValue, bool allowEmptyList, CheckType check,
                              const StringVectorParameter::PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
		:	StringVectorParameter( name, description, defaultValue, presets, presetsOnly, userData ), m_allowEmptyList( allowEmptyList ), m_check( check )
{
}

bool PathVectorParameter::allowEmptyList() const
{
	return m_allowEmptyList;
}

bool PathVectorParameter::mustExist() const
{
	return ( m_check == PathVectorParameter::MustExist );
}

bool PathVectorParameter::mustNotExist() const
{
	return ( m_check == PathVectorParameter::MustNotExist );
}

bool PathVectorParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	/// \todo Validate the individual path names using utilities found in boost::filesystem

	if ( !StringVectorParameter::valueValid( value, reason ) )
	{
		return false;
	}
	// if the above passed we know we have a string vector
	ConstStringVectorDataPtr s = static_pointer_cast<const StringVectorData>( value );

	// empty check
	if ( !allowEmptyList() && !s->readable().size() )
	{
		if ( reason )
		{
			*reason = "Empty filename not allowed";
		}
		return false;
	}
	else
	{
		if ( !s->readable().size() )
		{
			// if empty lists are allowed and we have one then we should
			// skip the other checks
			return true;
		}
	}

	// valid path check
	try
	{
		for ( StringVectorData::ValueType::const_iterator it = s->readable().begin(); it != s->readable().end(); ++it )
		{		
			boost::filesystem::path p( *it );
		}
	}
	catch ( ... )
	{
		if ( reason )
		{
			*reason = "Path has invalid form";
		}
		return false;
	}

	// existence check
	for ( StringVectorData::ValueType::const_iterator it = s->readable().begin(); it != s->readable().end(); ++it )
	{
		if ( boost::filesystem::exists(boost::filesystem::path( *it )))
		{
			if ( mustNotExist() )
			{
				if ( reason )
				{
					*reason = "\"" + *it + "\" already exists.";
				}
				return false;
			}
		}
		else
		{
			if ( mustExist() )
			{
				if ( reason )
				{
					*reason = "\"" + *it + "\" does not exist.";
				}
				return false;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void PathVectorParameter::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	StringVectorParameter::copyFrom( other, context );
	const PathVectorParameter *tOther = static_cast<const PathVectorParameter *>( other.get() );
	
	m_allowEmptyList = tOther->m_allowEmptyList;
	m_check = tOther->m_check;
}

void PathVectorParameter::save( SaveContext *context ) const
{
	StringVectorParameter::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), g_ioVersion );
	
	unsigned char tmp = m_allowEmptyList;
	container->write( "m_allowEmptyList", tmp );
	tmp = m_check;
	container->write( "check", tmp );
}

void PathVectorParameter::load( LoadContextPtr context )
{
	StringVectorParameter::load( context );
	unsigned int v = g_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	unsigned char tmp;
	container->read( "m_allowEmptyList", tmp );
	m_allowEmptyList = tmp;
	container->read( "check", tmp );
	m_check = (CheckType)tmp;
	
}

bool PathVectorParameter::isEqualTo( ConstObjectPtr other ) const
{
	if( !StringVectorParameter::isEqualTo( other ) )
	{
		return false;
	}
	
	const PathVectorParameter *tOther = static_cast<const PathVectorParameter *>( other.get() );
	return m_allowEmptyList==tOther->m_allowEmptyList && m_check==tOther->m_check;
}

void PathVectorParameter::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StringVectorParameter::memoryUsage( a );
	a.accumulate( sizeof( m_allowEmptyList ) + sizeof( m_check ) );
}
