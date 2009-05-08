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

#include "IECore/FileNameParameter.h"
#include "IECore/CompoundObject.h"

#include "boost/format.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/join.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <algorithm>

using namespace boost;
using namespace std;
using namespace IECore;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( FileNameParameter );
const unsigned int FileNameParameter::g_ioVersion = 1;

FileNameParameter::FileNameParameter()
{
}

FileNameParameter::FileNameParameter( const std::string &name, const std::string &description,
			const std::string &extensions, const std::string &defaultValue, bool allowEmptyString, PathParameter::CheckType check,
			const StringParameter::PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	PathParameter( name, description, defaultValue, allowEmptyString, check, presets, presetsOnly, userData )
{
	if( extensions!="" )
	{
		split( m_extensions, extensions, is_any_of( " " ) );
	}
}

const std::vector<std::string> &FileNameParameter::extensions() const
{
	return m_extensions;
}

bool FileNameParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if (!PathParameter::valueValid( value, reason ) )
	{
		return false;
	}

	ConstStringDataPtr s = static_pointer_cast<const StringData>( value );

	// empty check
	if( s->readable()=="" )
	{
		// if empty strings are allowed and we have one then we should
		// skip the other checks
		return true;
	}

	// extensions check
	if( extensions().size() )
	{
		string ext = boost::filesystem::extension(boost::filesystem::path( s->readable()));

		const vector<string> &exts = extensions();
		bool found = false;
		for( vector<string>::const_iterator it=exts.begin(); it!=exts.end(); it++ )
		{
			if( ("."+*it)==ext )
			{
				found = true;
				break;
			}
		}

		if( !found )
		{
			if( reason )
			{
				*reason = ( boost::format( "Filename extension '%s' not valid" ) % ext ).str();
			}
			return false;
		}
	}

	// file type check
	if( boost::filesystem::exists(boost::filesystem::path( s->readable())) &&
		 boost::filesystem::is_directory(boost::filesystem::path( s->readable())) )
	{
		if( reason )
		{
			*reason = "\"" + s->readable() + "\" is not a file!";
		}
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileNameParameter::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	PathParameter::copyFrom( other, context );
	const FileNameParameter *tOther = static_cast<const FileNameParameter *>( other.get() );
	m_extensions = tOther->m_extensions;
}

void FileNameParameter::save( SaveContext *context ) const
{
	PathParameter::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), g_ioVersion );

	std::string extensions = join( m_extensions, " " );
	container->write( "extensions", extensions );
}

void FileNameParameter::load( LoadContextPtr context )
{
	PathParameter::load( context );
	unsigned int v = g_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	m_extensions.clear();
	std::string extensions;
	container->read( "extensions", extensions );
	if( extensions!="" )
	{
		split( m_extensions, extensions, is_any_of( " " ) );
	}
}

bool FileNameParameter::isEqualTo( ConstObjectPtr other ) const
{
	if( !PathParameter::isEqualTo( other ) )
	{
		return false;
	}
	const FileNameParameter *tOther = static_cast<const FileNameParameter *>( other.get() );
	return m_extensions == tOther->m_extensions;
}

void FileNameParameter::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PathParameter::memoryUsage( a );
	for( std::vector<std::string>::const_iterator it=m_extensions.begin(); it!=m_extensions.end(); it++ )
	{
		a.accumulate( it->capacity() );
	}
}
