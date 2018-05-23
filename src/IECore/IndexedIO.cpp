//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/IndexedIO.h"

#include "IECore/Exception.h"

#include "boost/filesystem/convenience.hpp"

#include <iostream>

#include <math.h>

using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Internal implementation details
//////////////////////////////////////////////////////////////////////////

namespace
{

typedef std::map<std::string, IndexedIO::CreatorFn> CreatorMap;
static CreatorMap &creators()
{
	static CreatorMap *g_createFns = new CreatorMap();
	return *g_createFns;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// IndexedIO
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( IndexedIO )

namespace fs = boost::filesystem;

const IndexedIO::EntryID IndexedIO::rootName("/");
const IndexedIO::EntryIDList IndexedIO::rootPath;

IndexedIOPtr IndexedIO::create( const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode, const CompoundData *options )
{
	IndexedIOPtr result = nullptr;

	std::string extension = fs::extension(path);

	const CreatorMap &createFns = creators();

	CreatorMap::const_iterator it = createFns.find(extension);
	if (it == createFns.end())
	{
		throw IOException(path);
	}

	return (it->second)(path, root, mode, options);
}

void IndexedIO::supportedExtensions( std::vector<std::string> &extensions )
{
	CreatorMap &m = creators();
	for( CreatorMap::const_iterator it=m.begin(); it!=m.end(); it++ )
	{
		extensions.push_back( it->first.substr( 1 ) );
	}
}

void IndexedIO::registerCreator( const std::string &extension, CreatorFn f )
{
	CreatorMap &createFns = creators();

	assert( createFns.find(extension) == createFns.end() );

	createFns.insert( CreatorMap::value_type(extension, f) );
}

IndexedIO::~IndexedIO()
{
}

void IndexedIO::readable(const IndexedIO::EntryID &name) const
{
}

void IndexedIO::writable(const IndexedIO::EntryID &name) const
{
	if ( ( openMode() & (IndexedIO::Write | IndexedIO::Append) ) == 0)
	{
		throw PermissionDeniedIOException(name);
	}
}

void IndexedIO::validateOpenMode(IndexedIO::OpenMode &mode)
{
	// Clear 'other' bits
	mode &= IndexedIO::Read | IndexedIO::Write | IndexedIO::Append
			| IndexedIO::Shared | IndexedIO::Exclusive;

	// Check for mutual exclusivity
	if ((mode & IndexedIO::Shared)
		&& (mode & IndexedIO::Exclusive))
	{
		throw InvalidArgumentException("Incorrect IndexedIO open mode specified");
	}

	if ((mode & IndexedIO::Write)
		&& (mode & IndexedIO::Append))
	{
		throw InvalidArgumentException("Incorrect IndexedIO open mode specified");
	}

	// Set up default as 'read'
	if (!(mode & IndexedIO::Read
		|| mode & IndexedIO::Write
		|| mode & IndexedIO::Append)
	)
	{
		mode |= IndexedIO::Read;
	}

	// Set up default as 'shared'
	if (!(mode & IndexedIO::Shared
		|| mode & IndexedIO::Exclusive))
	{
		mode |= IndexedIO::Shared;
	}

}

//
// Entry
//

static InternedString emptyString("");

IndexedIO::Entry::Entry() : m_ID(emptyString), m_entryType( IndexedIO::Directory), m_dataType( IndexedIO::Invalid), m_arrayLength(0)
{
}

IndexedIO::Entry::Entry( const IndexedIO::EntryID &id, IndexedIO::EntryType eType, IndexedIO::DataType dType, unsigned long arrayLength)
: m_ID(id), m_entryType(eType), m_dataType(dType), m_arrayLength(arrayLength)
{
}

const IndexedIO::EntryID &IndexedIO::Entry::id() const
{
	return m_ID;
}

IndexedIO::EntryType IndexedIO::Entry::entryType() const
{
	return m_entryType;
}

IndexedIO::DataType IndexedIO::Entry::dataType() const
{
	if (m_entryType == IndexedIO::Directory)
	{
		throw IOException( "IndexedIO Entry '" + m_ID.value() + "' has no data type - it is a directory" );
	}

	return m_dataType;
}

bool IndexedIO::Entry::isArray() const
{
	return isArray( m_dataType );
}

bool IndexedIO::Entry::isArray( IndexedIO::DataType dType )
{
	switch( dType )
	{
		case IndexedIO::FloatArray:
		case IndexedIO::DoubleArray:
		case IndexedIO::HalfArray:
		case IndexedIO::IntArray:
		case IndexedIO::LongArray:
		case IndexedIO::StringArray:
		case IndexedIO::UIntArray:
		case IndexedIO::CharArray:
		case IndexedIO::UCharArray:
		case IndexedIO::ShortArray:
		case IndexedIO::UShortArray:
		case IndexedIO::Int64Array:
		case IndexedIO::UInt64Array:
		case IndexedIO::InternedStringArray:
			return true;
		default:
			return false;
	}
}

unsigned long IndexedIO::Entry::arrayLength() const
{
	if ( !isArray() )
	{
		throw IOException( "IndexedIO Entry '" + m_ID.value() + "' is not an array" );
	}

	return m_arrayLength;
}
