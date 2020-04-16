//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECorePython/CamelCaseBinding.h"

#include "IECore/CamelCase.h"

#include "boost/python/suite/indexing/container_utils.hpp"

#include <vector>

using namespace boost::python;
using namespace IECore;
using namespace std;

namespace IECorePython
{

static boost::python::list split( const std::string &camelCase )
{
	vector<string> s;
	CamelCase::split( camelCase, back_insert_iterator<vector<string> >( s ) );

	boost::python::list result;
	for( vector<string>::const_iterator it = s.begin(), end = s.end(); it!=end; ++it )
	{
		result.append( *it );
	}

	return result;
}

static std::string join( object words, CamelCase::Caps caps, const std::string &separator )
{
	vector<string> w;
	boost::python::container_utils::extend_container( w, words );
	return CamelCase::join( w.begin(), w.end(), caps, separator );
}

void bindCamelCase()
{
	using boost::python::arg;

	class_<CamelCase> c( "CamelCase" );

	{
		scope s( c );

		enum_<CamelCase::Caps>( "Caps" )
			.value( "Unchanged", CamelCase::Unchanged )
			.value( "First", CamelCase::First )
			.value( "All", CamelCase::All )
			.value( "AllExceptFirst", CamelCase::AllExceptFirst )
		;
	}

	c.def( "split", &split ).staticmethod( "split" );
	c.def( "join", &join, ( arg( "words" ), arg( "caps" ) = CamelCase::All, arg( "separator" ) = "" ) ).staticmethod( "join" );
	c.def( "toSpaced", &CamelCase::toSpaced, ( arg( "camelCase" ), arg( "caps" ) = CamelCase::All ) ).staticmethod( "toSpaced" );
	c.def( "fromSpaced", &CamelCase::fromSpaced, ( arg( "spaced" ), arg( "caps" ) = CamelCase::All ) ).staticmethod( "fromSpaced" );
}

} // namespace IECorePython
