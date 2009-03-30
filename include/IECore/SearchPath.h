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

#ifndef IE_CORE_SEARCHPATH_H
#define IE_CORE_SEARCHPATH_H

#include <list>

#include "boost/filesystem/path.hpp"

namespace IECore
{

/// The SearchPath class provides a simple means of finding a file on a set of searchpaths.
class SearchPath
{
	public :

		/// Constructs with an empty paths list.
		SearchPath();
		/// Calls setPaths() with paths and separators.
		SearchPath( const std::string &paths, const std::string &separators );

		bool operator == ( const SearchPath &s ) const;

		/// A list of paths to search on. This is public and can be manipulated at will.
		std::list<boost::filesystem::path> paths;
		
		/// Sets paths by tokenizing the paths string according to the separators specified.
		void setPaths( const std::string &paths, const std::string &separators );
		/// Returns the paths concatenated together by separator.
		std::string getPaths( const std::string &separator ) const;
		
		/// Tries to find the specified file on the paths defined in the paths public member.
		/// Use result.empty() to determine failure.
		boost::filesystem::path find( const boost::filesystem::path &file ) const;
	
};
	
} // namespace IECore

#endif // IE_CORE_SEARCHPATH_H
