//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include <map>
#include <set>
#include <algorithm>
#include "boost/regex.hpp"

#include "IECore/AttributeCache.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundObject.h"
#include "IECore/HeaderGenerator.h"
#include "IECore/IndexedIO.h"

using namespace IECore;

AttributeCache::AttributeCache( const std::string &filename, IndexedIO::OpenMode mode )
{
	IndexedIOPtr io = IndexedIO::create(filename, IndexedIO::rootPath, mode );

	if ( mode == IndexedIO::Write || mode == IndexedIO::Append )
	{
		m_headersIO = io->subdirectory("headers", IndexedIO::CreateIfMissing );
		m_objectsIO = io->subdirectory("objects", IndexedIO::CreateIfMissing );

		CompoundObjectPtr header = HeaderGenerator::header();
		for ( CompoundObject::ObjectMap::const_iterator it = header->members().begin(); it != header->members().end(); it++ )
		{
			writeHeader( it->first, it->second.get() );
		}
	}
	if ( mode == IndexedIO::Read )
	{
		try
		{
			m_headersIO = io->subdirectory("headers");
			m_objectsIO = io->subdirectory("objects");
		}
		catch (IECore::Exception &e)
		{
			throw Exception("Not an AttributeCache file.");
		}
	}
}

void AttributeCache::write( const ObjectHandle &obj, const AttributeHandle &attr, const Object *data)
{
	data->save( m_objectsIO->subdirectory(obj, IndexedIO::CreateIfMissing), attr );
}

void AttributeCache::writeHeader( const HeaderHandle &hdr, const Object *data)
{
	data->save( m_headersIO, hdr );
}

ObjectPtr AttributeCache::read( const ObjectHandle &obj, const AttributeHandle &attr )
{
	ObjectPtr data = Object::load( m_objectsIO->subdirectory(obj), attr );
	return data;
}

CompoundObjectPtr AttributeCache::read( const ObjectHandle &obj )
{
	CompoundObjectPtr dict = new CompoundObject();

	IndexedIO::EntryIDList directories;
	IndexedIOPtr object = m_objectsIO->subdirectory( obj );
	object->entryIds( directories, IndexedIO::Directory );

	for (IndexedIO::EntryIDList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		ObjectPtr data = Object::load( object, *it );
		dict->members()[ *it ] = data;
	}

	return dict;
}

ObjectPtr AttributeCache::readHeader( const HeaderHandle &hdr )
{
	ObjectPtr data = Object::load( m_headersIO, hdr );
	return data;
}

CompoundObjectPtr AttributeCache::readHeader( )
{
	CompoundObjectPtr dict = new CompoundObject();

	IndexedIO::EntryIDList directories;
	m_headersIO->entryIds( directories, IndexedIO::Directory );

	for (IndexedIO::EntryIDList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		ObjectPtr data = Object::load( m_headersIO, *it );
		dict->members()[ *it ] = data;
	}

	return dict;
}

void AttributeCache::headers(std::vector<AttributeCache::HeaderHandle> &hds)
{
	m_headersIO->entryIds( hds, IndexedIO::Directory );
}

void AttributeCache::objects(std::vector<AttributeCache::ObjectHandle> &objs)
{
	m_objectsIO->entryIds( objs, IndexedIO::Directory );
}

bool AttributeCache::contains( const ObjectHandle &obj )
{
	return m_objectsIO->hasEntry( obj );
}

bool AttributeCache::contains( const ObjectHandle &obj, const AttributeHandle &attr )
{
	IndexedIOPtr object = m_objectsIO->subdirectory( obj, IndexedIO::NullIfMissing );
	if ( !object )
	{
		return false;
	}
	return object->hasEntry(attr);
}

void AttributeCache::attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs)
{
	m_objectsIO->subdirectory(obj)->entryIds( attrs );
}

void AttributeCache::attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs)
{
	IndexedIO::EntryIDList directories;
	attributes( obj, directories );

	boost::regex regexTest(regex);
	boost::cmatch what;

	attrs.clear();
	attrs.reserve( directories.size() );
	for (IndexedIO::EntryIDList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		if ( regex_match( (*it).value().c_str(), what, regexTest) )
		{
			attrs.push_back( *it );
		}
	}
}

void AttributeCache::remove( const ObjectHandle &obj )
{
	m_objectsIO->remove( obj );
}

void AttributeCache::remove( const ObjectHandle &obj, const AttributeHandle &attr )
{
	IndexedIOPtr object = m_objectsIO->subdirectory(obj);
	object->remove(attr);
}

void AttributeCache::removeHeader( const HeaderHandle &hdr )
{
	m_headersIO->remove( hdr );
}
