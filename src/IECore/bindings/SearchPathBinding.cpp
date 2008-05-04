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

#include <boost/python.hpp>
#include <boost/python/suite/indexing/container_utils.hpp>

#include "IECore/SearchPath.h"

#include <vector>

using namespace boost;
using namespace boost::python;
using namespace boost::filesystem;
using namespace std;

namespace IECore {

static string find( const SearchPath &s, const char *f )
{
	return s.find( f ).string();
}

static object getPaths( const SearchPath &s )
{
	boost::python::list l;
	for( std::list<path>::const_iterator it=s.paths.begin(); it!=s.paths.end(); it++ )
	{
		l.append( it->string() );
	}
	return l;
}

static void setPaths( SearchPath &s, const object &p )
{
	s.paths.clear();
	std::vector<std::string> ss;
	boost::python::container_utils::extend_container( ss, p );
	s.paths.resize( ss.size() );
	std::copy( ss.begin(), ss.end(), s.paths.begin() );
}

void bindSearchPath()
{
	class_<SearchPath>( "SearchPath" )
		.def( init<string, string>() )
		.def( "find", &find )
		.def( "setPaths", &SearchPath::setPaths )
		.def( "getPaths", &SearchPath::getPaths )
		.def( self == self  )
		.add_property( "paths", &getPaths, &setPaths )	
	;
}

} // namespace IECore
