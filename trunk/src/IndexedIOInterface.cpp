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

#include <math.h>
#include <iostream>

#include <boost/filesystem/convenience.hpp>

#include "IECore/Exception.h"
#include "IECore/IndexedIOInterface.h"

using namespace IECore;

namespace fs = boost::filesystem;

IndexedIOInterfacePtr IndexedIOInterface::create( const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode )
{
	IndexedIOInterfacePtr result = 0;
	
	std::string extension = fs::extension(path);
	
	const CreatorMap &createFns = getCreateFns();
	
	CreatorMap::const_iterator it = createFns.find(extension);
	if (it == createFns.end())
	{
		throw IOException(path);
	}
	
	return (it->second)(path, root, mode);
}

void IndexedIOInterface::registerCreator( const std::string &extension, CreatorFn f )
{
	CreatorMap &createFns = getCreateFns();
	
	assert( createFns.find(extension) == createFns.end() );
	
	createFns.insert( CreatorMap::value_type(extension, f) );
}

IndexedIOInterface::~IndexedIOInterface()
{
}

void IndexedIOInterface::readable(const IndexedIO::EntryID &name) const
{
}

void IndexedIOInterface::writable(const IndexedIO::EntryID &name) const
{
	if (m_mode & (IndexedIO::Write | IndexedIO::Append) == 0)
	{
		throw PermissionDeniedIOException(name);
	}
}

void IndexedIOInterface::validateOpenMode(IndexedIO::OpenMode mode)
{
	m_mode = mode;
	
	// Clear 'other' bits
	m_mode &= IndexedIO::Read | IndexedIO::Write | IndexedIO::Append 
			| IndexedIO::Shared | IndexedIO::Exclusive;
	
	// Check for mutual exclusivity
	if ((m_mode & IndexedIO::Shared)
		&& (m_mode & IndexedIO::Exclusive))
	{		
		throw InvalidArgumentException("Incorrect IndexedIO open mode specified");
	}
	
	if ((m_mode & IndexedIO::Write)
		&& (m_mode & IndexedIO::Append))
	{	
		throw InvalidArgumentException("Incorrect IndexedIO open mode specified");
	}
		
	// Set up default as 'read'
	if (!(m_mode & IndexedIO::Read
		|| m_mode & IndexedIO::Write
		|| m_mode & IndexedIO::Append)
	)
	{
		m_mode |= IndexedIO::Read;
	}
	
	// Set up default as 'shared'
	if (!(m_mode & IndexedIO::Shared
		|| m_mode & IndexedIO::Exclusive))
	{
		m_mode |= IndexedIO::Shared;
	}
	
}

