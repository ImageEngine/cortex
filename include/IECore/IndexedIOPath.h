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

#ifndef _IECORE_INDEXEDIOPATH_H
#define _IECORE_INDEXEDIOPATH_H

#include <string>
#include <vector>

namespace IECore
{

/// A class which encapsulates the concept of a path consisting of an immutable
/// root directory, and a path relative to the rootfor use in IndexedIO operations. More or
/// less mimics boost::filesystem::path functionality with improved efficiency.
class IndexedIOPath
{
	public:
	
		static const char g_separator = '/';
	
		/// Create a default path rooted at "/"
		IndexedIOPath();
		
		/// Create a path with a user-defined root and relative path
		IndexedIOPath( const std::string &root, const std::string &path = "");
		
		/// Assignment operator.		
		IndexedIOPath& operator=(const IndexedIOPath &other);
			
		/// Retrieve the root path of this directory, as specified in the
		/// constructor		
		const std::string &rootPath() const;
		
		/// Retrieve just the relative portion of this path
		const std::string &relativePath() const;
		
		/// Retrieve the full path, root + relative
		const std::string &fullPath() const;
		
		/// Append a path, similar to a "chdir" operation. Valid paths
		/// might be "/", "..", "/a/b", "a/b", etc.		
		void append( const std::string &path );
		
		/// Returns a copy of this object with the given path appended.
		IndexedIOPath appended( const std::string &path ) const;
		
		/// Test the validity of the given file/directory name.	
		static bool validFilename( const std::string &n );
		
		std::string head() const;
		
		std::string tail() const;
		
	protected:
	
		void buildPath() const;
		void buildRelativePath() const;
	
		typedef std::vector<std::string> StringVector;
			
		std::string m_root;
		StringVector m_relativePathParts;

		mutable bool m_relativePathValid;	
		mutable std::string m_relativePath;
		
		mutable bool m_pathValid;
		mutable std::string m_path;
		
		unsigned m_relativePathSize;
	
		unsigned m_rootSize;
		
		bool m_isAbsolute;
		
		mutable bool m_headValid;	
		mutable std::string m_head;
		
		mutable bool m_tailValid;	
		mutable std::string m_tail;		

};

}

#endif
