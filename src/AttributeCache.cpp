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

#include <map>
#include <set>
#include <algorithm>

#include <IECore/AttributeCache.h>
#include <IECore/VectorTypedData.h>
#include <IECore/CompoundObject.h>

using namespace IECore;

AttributeCache::AttributeCache( const std::string &filename, IndexedIO::OpenMode mode )
{
	m_io = IndexedIOInterface::create(filename, "/", mode );
	
	if ( mode == IndexedIO::Write || mode == IndexedIO::Append )
	{
		m_io->mkdir("/headers");
		m_io->mkdir("/objects");
	}
}

void AttributeCache::write( const ObjectHandle &obj, const AttributeHandle &attr, ObjectPtr data)
{
	m_io->chdir("/objects");
	m_io->mkdir(obj);
	m_io->chdir(obj);
	
	m_io->mkdir(attr);
	m_io->chdir(attr);
		
	data->save(m_io->resetRoot());
}

void AttributeCache::writeHeader( const HeaderHandle &hdr, ObjectPtr data)
{
	m_io->chdir("/headers");
	m_io->mkdir(hdr);
	m_io->chdir(hdr);
	
	data->save(m_io->resetRoot());
}

ObjectPtr AttributeCache::read( const ObjectHandle &obj, const AttributeHandle &attr )
{
	m_io->chdir("/objects");
	m_io->chdir(obj);
	m_io->chdir(attr);
	ObjectPtr data = Object::load( m_io->resetRoot() );

	return data;
}

CompoundObjectPtr AttributeCache::read( const ObjectHandle &obj )
{
	CompoundObjectPtr dict = new CompoundObject();

	m_io->chdir("/objects");
	m_io->chdir(obj);

	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		m_io->chdir( it->id() );
	
		ObjectPtr data = Object::load( m_io->resetRoot() );

		dict->members()[ it->id() ] = data;

		m_io->chdir( ".." );
	}

	return dict;
}

ObjectPtr AttributeCache::readHeader( const HeaderHandle &hdr )
{
	m_io->chdir("/headers");
	m_io->chdir(hdr);
	ObjectPtr data = Object::load( m_io->resetRoot() );

	return data;
}

CompoundObjectPtr AttributeCache::readHeader( )
{
	CompoundObjectPtr dict = new CompoundObject();

	m_io->chdir("/headers");

	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		m_io->chdir( it->id() );
	
		ObjectPtr data = Object::load( m_io->resetRoot() );

		dict->members()[ it->id() ] = data;

		m_io->chdir( ".." );
	}

	return dict;
}

void AttributeCache::headers(std::vector<AttributeCache::HeaderHandle> &hds)
{
	hds.clear();
	
	m_io->chdir("/headers");
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	hds.reserve( directories.size() );	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		hds.push_back( it->id() );
	}
}

void AttributeCache::objects(std::vector<AttributeCache::ObjectHandle> &objs)
{
	objs.clear();
	
	m_io->chdir("/objects");
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	objs.reserve( directories.size() );	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		objs.push_back( it->id() );
	}
}
		
bool AttributeCache::contains( const ObjectHandle &obj )
{
	m_io->chdir("/objects");
	try
	{
		m_io->chdir( obj );
	} 
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

bool AttributeCache::contains( const ObjectHandle &obj, const AttributeHandle &attr )
{
	m_io->chdir("/objects");
	try
	{
		m_io->chdir( obj );
		m_io->chdir( attr );
	} 
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}
				
void AttributeCache::attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs)
{
	attrs.clear();
	
	m_io->chdir("/objects");
	m_io->chdir(obj);
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	attrs.reserve( directories.size() );
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		attrs.push_back( it->id() );
	}
}

void AttributeCache::attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs)
{
	attrs.clear();
	
	m_io->chdir("/objects");
	m_io->chdir(obj);
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	filter->add( new IndexedIORegexFilter(  regex ) );
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	attrs.reserve( directories.size() );
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		attrs.push_back( it->id() );
	}
}

void AttributeCache::remove( const ObjectHandle &obj )
{
	m_io->chdir("/objects");

	m_io->rm( obj );
}

void AttributeCache::remove( const ObjectHandle &obj, const AttributeHandle &attr )
{
	m_io->chdir("/objects");

	m_io->chdir( obj );
	m_io->rm( attr );
}

void AttributeCache::removeHeader( const HeaderHandle &hdr )
{
	m_io->chdir("/headers");

	m_io->rm( hdr );
}
