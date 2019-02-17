//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

// Windows defines SearchPath
#ifdef SearchPath
#undef SearchPath
#endif

#ifndef IE_CORE_SEARCHPATH_H
#define IE_CORE_SEARCHPATH_H

#include "IECore/Export.h"

#include "boost/filesystem/path.hpp"

#include <list>

namespace IECore
{

/// The SearchPath class provides a simple means of finding a file on a set of searchpaths.
/// On Linux/MacOS paths must be supplied in generic format (path elements separated by forward slashes)
/// On Windows paths can be supplied in generic or Windows native format (path elements separated by back slashes)
/// \ingroup utilityGroup
class IECORE_API SearchPath
{
	public :

		typedef std::list<boost::filesystem::path> Paths;

		/// Constructs with an empty paths list.
		SearchPath();
		SearchPath( const Paths &paths );
		/// Constructs from a series of paths separated by ':' on Linux/MacOS and
		/// ';' on Windows. Typically this constructor would be used with
		/// a value fetched from an environment variable such as `$PATH`.
		SearchPath( const std::string &paths );
		/// \deprecated
		SearchPath( const std::string &paths, const std::string &separators );

		bool operator == ( const SearchPath &s ) const;
		bool operator != ( const SearchPath &s ) const;

		/// A list of paths to search on. This is public and can be manipulated at will.
		Paths paths;

		/// \deprecated
		void setPaths( const std::string &paths, const std::string &separators );
		/// \deprecated
		std::string getPaths( const std::string &separator ) const;

		/// Tries to find the specified file on the paths defined in the paths public member.
		/// If found, the path is returned in the native format to the OS (path elements separated by "/" on Linux/MacOS, "\" on Windows)
		/// Use result.empty() to determine failure.
		boost::filesystem::path find( const boost::filesystem::path &file ) const;

};

} // namespace IECore

#endif // IE_CORE_SEARCHPATH_H
