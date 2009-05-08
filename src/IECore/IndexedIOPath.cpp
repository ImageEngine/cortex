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

#include <iostream>
#include <cassert>

#include "boost/filesystem/convenience.hpp"

#include "IECore/Exception.h"
#include "IECore/IndexedIOPath.h"

using namespace IECore;
namespace fs = boost::filesystem;

IndexedIOPath::IndexedIOPath()
{
	m_pathValid = false;
	m_relativePathValid = false;

	m_rootSize = 0;
	m_relativePathSize = 0;
}

IndexedIOPath::IndexedIOPath( const std::string &root, const std::string &path)
{
	std::string r = root;
	m_isAbsolute = false;
	if (r.size() && r[0] == g_separator)
	{
		r = r.substr(1, std::string::npos);
		m_isAbsolute = true;
	}

	m_pathValid = false;
	m_relativePathValid = false;
	m_headValid = false;
	m_tailValid = false;

	m_rootSize = r.size();
	m_root = r;

	m_relativePath = "";
	m_relativePathSize = 0;

	if (path.size())
	{
		append(path);
	}
}

IndexedIOPath& IndexedIOPath::operator=(const IndexedIOPath &other)
{
	m_root = other.m_root;
	m_relativePathParts = other.m_relativePathParts;
	m_isAbsolute = other.m_isAbsolute;


	m_pathValid = other.m_pathValid;
	if (m_pathValid)
	{
		m_path = other.m_path;
	}

	m_relativePathValid = other.m_relativePathValid;
	if (m_relativePathValid)
	{
		m_relativePath = other.m_relativePath;
	}
	m_relativePathSize = other.m_relativePathSize;

	m_rootSize = other.m_rootSize;

	m_headValid = other.m_headValid;
	if (m_headValid)
	{
		m_head = other.m_head;
	}

	m_tailValid = other.m_tailValid;
	if (m_tailValid)
	{
		m_tail = other.m_tail;
	}

	return *this;
}

void IndexedIOPath::buildPath() const
{
	const std::string &r = relativePath();

	unsigned sz = m_rootSize + m_relativePathSize + m_relativePathParts.size() + 2;
	if (sz > m_path.capacity())
	{
		m_path.reserve(sz);
	}

	m_path = rootPath();

	/// Make sure we don't have a duplicate separator (if root ends with one, and relative path starts with one)
	if (r.size() > 1)
	{
		if (m_path.size() && m_path[ m_path.size() -1 ] == g_separator && r[0] == g_separator )
		{
			m_path += r.substr(1, std::string::npos);
		}
		else
		{
			m_path += r;
		}
	}

	if (m_isAbsolute && (!m_path.size() || m_path[0] != g_separator))
	{
		m_path = g_separator + m_path;
	}

	if (!m_isAbsolute)
	{
		if (m_path.size() && m_path[0] == g_separator)
		{
			m_path = m_path.substr(1, std::string::npos);
		}
	}

	m_pathValid = true;
}


const std::string &IndexedIOPath::fullPath() const
{
	if (!m_pathValid || !m_relativePathValid)
	{
		buildPath();
	}

	return m_path;
}


void IndexedIOPath::append( const std::string &path )
{
	if (path.size() == 0)
	{
		throw InvalidArgumentException(path);
	}
	else if (path.size() == 1 && path[0] == g_separator)
	{
		m_relativePathParts.clear();
		m_relativePathSize = 0;
		m_relativePathValid = false;
		m_pathValid = false;
		m_headValid = false;
		m_tailValid = false;
	}
	else if (path == ".")
	{

	}
	else if (path == "..")
	{
		if (m_relativePathParts.size())
		{
			m_relativePathSize -= m_relativePathParts.back().size();
			m_relativePathParts.pop_back();
			m_relativePathValid = false;
			m_pathValid = false;
			m_headValid = false;
			m_tailValid = false;
		}
	}
	else if ( path.find_first_of(g_separator) != std::string::npos)
	{
		std::string::size_type startPos = 0;

		if (path[0] == g_separator)
		{
			m_relativePathParts.clear();
			m_relativePathSize = 0;
			m_relativePathValid = false;
			m_pathValid = false;
			m_headValid = false;
			m_tailValid = false;

			if (path.substr(1, m_rootSize) == m_root)
			{
				startPos = m_rootSize+1;
			}
			else
			{
				startPos = 1;
			}

		}

		const std::string::size_type pathSize = path.size();
		std::string::size_type endPos = startPos ;

		while (endPos < pathSize)
		{
			if (path[endPos] == g_separator || endPos == pathSize-1)
			{
				assert(startPos >= 0);
				assert(endPos < pathSize);
				assert(endPos >= startPos);

				std::string::size_type n = endPos-startPos;
				if(endPos == pathSize-1)
					n++;

				if ( startPos == 0 && n == path.size() )
				{
					assert( path[path.size()-1] == g_separator );

					append( path.substr( startPos, n-1 ) );
				}
				else
				{
					append( path.substr( startPos, n ) );
				}
				startPos = endPos + 1;
				endPos = startPos;
			}
			else
			{
				endPos ++;
			}
		}
	}
	else
	{
		if (!validFilename(path))
		{
			throw InvalidArgumentException(path);
		}

		m_relativePathParts.push_back( path );
		m_relativePathSize += path.size();
		m_relativePathValid = false;
		m_pathValid = false;
		m_headValid = false;
		m_tailValid = false;
	}

}


IndexedIOPath IndexedIOPath::appended( const std::string &path ) const
{
	IndexedIOPath result = *this;

	result.append(path);

	return result;
}


void IndexedIOPath::buildRelativePath() const
{
	unsigned sz = m_relativePathSize + m_relativePathParts.size();

	m_relativePath = "";
	if (sz > m_relativePath.capacity())
	{
		m_relativePath.reserve( sz );
	}

	for (StringVector::const_iterator it = m_relativePathParts.begin(); it != m_relativePathParts.end(); ++it)
	{
		m_relativePath += g_separator;
		m_relativePath += *it;
	}

	if (!m_relativePathSize || m_relativePath[0] != g_separator)
	{
		m_relativePath = g_separator + m_relativePath;
	}

	m_relativePathValid = true;
}

bool IndexedIOPath::validFilename(const std::string &n)
{
	static const std::string validChars( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-:" );

	return (n.size() != 0) && (n.find_first_not_of( validChars ) == std::string::npos);
}

const std::string &IndexedIOPath::rootPath() const
{
	return m_root;
}

const std::string &IndexedIOPath::relativePath() const
{
	if (!m_relativePathValid)
	{
		buildRelativePath();
	}

	return m_relativePath;
}

std::string IndexedIOPath::head() const
{
	if (m_headValid)
	{
		return m_head;
	}

	if (fullPath() == "/")
	{
		m_head = "/";
		m_headValid = true;
		return m_head;
	}
	std::string::size_type s = fullPath().find_last_of(g_separator);
	m_head = fullPath().substr(0, s);
	if (m_head == "")
	{
		m_head = "/";
		m_headValid = true;
		return m_head;
	}

	// Ensure path doesn't contain trailing slash
	if (*(m_head.rbegin()) == '/')
	{
		m_head = m_head.substr(0, m_head.size()-1);
	}

	m_headValid = true;
	return m_head;
}

std::string IndexedIOPath::tail() const
{
	if (m_tailValid)
	{
		return m_tail;
	}

	std::string::size_type s = fullPath().find_last_of(g_separator);

	if (s == std::string::npos)
	{
		m_tail = fullPath();
		m_tailValid = true;
		return m_tail;
	}
	else
	{
		m_tail = fullPath().substr(s+1);
		m_tailValid = true;
		return m_tail;
	}

}

bool IndexedIOPath::hasRootDirectory() const
{
	return m_isAbsolute;
}
