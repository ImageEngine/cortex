//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, John Haddon. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#include "IECore/StringAlgo.h"

#include "boost/algorithm/string/replace.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

using namespace std;
using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

const InternedString g_ellipsis( "..." );
const InternedString g_ellipsisSubstitute( "!!!" );

bool matchInternal(
	vector<InternedString>::const_iterator pathBegin,
	vector<InternedString>::const_iterator pathEnd,
	StringAlgo::MatchPatternPath::const_iterator matchPathBegin,
	StringAlgo::MatchPatternPath::const_iterator matchPathEnd
)
{
	while( true )
	{
		if( matchPathBegin == matchPathEnd )
		{
			return pathBegin == pathEnd;
		}
		else if( *matchPathBegin == g_ellipsis )
		{
			auto nextMatchPathBegin = std::next( matchPathBegin );
			if( nextMatchPathBegin == matchPathEnd )
			{
				return true;
			}
			for( auto pb = pathBegin; pb != pathEnd; ++pb )
			{
				if( matchInternal( pb, pathEnd, nextMatchPathBegin, matchPathEnd ) )
				{
					return true;
				}
			}
			return false;
		}
		else if( pathBegin == pathEnd )
		{
			return false;
		}
		else if( !StringAlgo::match( *pathBegin, *matchPathBegin ) )
		{
			return false;
		}
		++pathBegin;
		++matchPathBegin;
	}
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////

namespace IECore
{

namespace StringAlgo
{

bool match( const std::vector<InternedString> &path, const MatchPatternPath &patternPath )
{
	return matchInternal( path.begin(), path.end(), patternPath.begin(), patternPath.end() );
}

MatchPatternPath matchPatternPath( const std::string &patternPath, char separator )
{
	string path = patternPath;
	if( separator == '.' )
	{
		boost::replace_all( path, "...", "." + g_ellipsisSubstitute.string() + "." );
	}

	vector<InternedString> result;
	StringAlgo::tokenize( path, separator, result );

	if( separator == '.' )
	{
		std::replace( result.begin(), result.end(), g_ellipsisSubstitute, g_ellipsis );
	}
	return result;
}

int numericSuffix( const std::string &s, std::string *stem )
{
	static boost::regex g_regex( "^(.*[^0-9]+)([0-9]+)$" );
	boost::cmatch match;
	if( regex_match( s.c_str(), match, g_regex ) )
	{
		if( stem )
		{
			*stem = match[1];
		}
		return boost::lexical_cast<int>( match[2] );
	}
	if( stem )
	{
		*stem = s;
	}
	return -1;
}

int numericSuffix( const std::string &s, int defaultSuffix, std::string *stem )
{
	int n = numericSuffix( s, stem );
	return n < 0 ? defaultSuffix : n;
}

} // namespace StringAlgo

} // namespace IECore
