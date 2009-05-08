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

#include <algorithm>

#include "boost/filesystem/operations.hpp"
#include "boost/tokenizer.hpp"

#include "IECore/SearchPath.h"

using namespace IECore;
using namespace std;
using namespace boost::filesystem;
using namespace boost;

SearchPath::SearchPath()
{
}

SearchPath::SearchPath( const std::string &paths, const std::string &separators )
{
	setPaths( paths, separators );
}

bool SearchPath::operator == ( const SearchPath &s ) const
{
	return paths==s.paths;
}

void SearchPath::setPaths( const std::string &paths, const std::string &separators )
{
	this->paths.clear();
	boost::tokenizer<boost::char_separator<char> > t( paths, char_separator<char>( separators.c_str() ) );
	copy( t.begin(), t.end(), back_insert_iterator<list<path> >( this->paths ) );
}

std::string SearchPath::getPaths( const std::string &separator ) const
{
	bool first = true;
	string result;
	for( list<path>::const_iterator it=paths.begin(); it!=paths.end(); it++ )
	{
		if( first )
		{
			first = false;
		}
		else
		{
			result += separator;
		}
		result += it->string();
	}
	return result;
}

boost::filesystem::path SearchPath::find( const boost::filesystem::path &file ) const
{
	// if it's a full path then there's no need to do any searching
	if( file.is_complete() )
	{
		if( exists( file ) )
		{
			return file;
		}
		else
		{
			return "";
		}
	}

	// do some searching
	for( list<path>::const_iterator it = paths.begin(); it!=paths.end(); it++ )
	{
		path f = *it / file;
		if( exists( f ) )
		{
			return f;
		}
	}
	return "";
}
