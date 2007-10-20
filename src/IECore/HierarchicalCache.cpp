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
#include <boost/filesystem/path.hpp>
#include "OpenEXR/ImathBoxAlgo.h"

#include <IECore/HierarchicalCache.h>
#include <IECore/VectorTypedData.h>
#include <IECore/CompoundObject.h>
#include <IECore/Group.h>
#include <IECore/IndexedIOInterface.h>
#include <IECore/MessageHandler.h>
#include <IECore/MatrixTransform.h>
#include <IECore/HeaderGenerator.h>

using namespace IECore;
namespace fs = boost::filesystem;

void HierarchicalCache::CacheDependency::compute( const std::string &node )
{
	m_cache->updateNode( node );
}

std::string HierarchicalCache::CacheDependency::rootNode( ) const
{
	return HierarchicalCache::rootName();
}

bool HierarchicalCache::CacheDependency::isDescendant( const std::string &node1, const std::string &node2 ) const
{
	if ( node1.compare( 0, node2.size(), node2 ) == 0 )
	{
		if ( node1.size() == node2.size() )
		{
			return true;
		}
		if ( node2.size() > 1 )
		{
			if ( node1[ node2.size() ]  == '/' )
			{
				return true;
			}
		}
		else if ( node2.size() == 1 )
		{
			//  node2 is "/"...
			return true;
		}
	}
	return false;
}

HierarchicalCache::HierarchicalCache( const std::string &filename, IndexedIO::OpenMode mode ) : m_dependency( new CacheDependency( this ) )
{
	m_io = IndexedIOInterface::create(filename, "/", mode );

	if ( mode == IndexedIO::Write || mode == IndexedIO::Append )
	{
		m_io->mkdir("/HierarchicalCache");
		m_io->mkdir("/children");
		CompoundObjectPtr header = HeaderGenerator::header();
		for ( CompoundObject::ObjectMap::const_iterator it = header->members().begin(); it != header->members().end(); it++ )
		{
			writeHeader( it->first, it->second );
		}
	}
	if ( mode == IndexedIO::Read )
	{
		try
		{
			m_io->chdir("/HierarchicalCache");
			m_io->chdir("/children");
		}
		catch (IECore::Exception &e)
		{
			throw Exception("Not a HierarchyCache file.");
		}
	}
}

HierarchicalCache::~HierarchicalCache()
{
	try
	{
		flush();
	}
	catch (IECore::Exception &e)
	{
		msg( MessageHandler::Error, "HierarchyCache", e.type() + std::string(" while flushing data to file: ") + e.what() );
	}
}

void HierarchicalCache::flush()
{
	m_dependency->update();
}

HierarchicalCache::ObjectHandle HierarchicalCache::absoluteName( const ObjectHandle &relativeName, const ObjectHandle &parent )
{
	fs::path parentPath( parent );
	if ( !parentPath.has_root_directory() )
	{
		throw Exception( std::string( "Invalid object name ") + parent );
	}
	fs::path relativePath( relativeName );
	if ( relativePath.has_root_directory() )
	{
		throw Exception( std::string( "Invalid relative name ") + relativeName );
	}
	return ( parentPath / relativePath ).string();
}

HierarchicalCache::ObjectHandle HierarchicalCache::relativeName( const ObjectHandle &obj )
{
	fs::path pathObj( obj );
	if ( !pathObj.has_root_directory() )
	{
		throw Exception( std::string( "Invalid object name ") + obj );
	}
	return pathObj.leaf();
}

HierarchicalCache::ObjectHandle HierarchicalCache::parentName( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		throw Exception( std::string( "Root node has no parents.") );
	}
	fs::path pathObj( obj );
	if ( !pathObj.has_root_directory() )
	{
		throw Exception( std::string( "Invalid object name ") + obj );
	}
	return pathObj.branch_path().string();
}

HierarchicalCache::ObjectHandle HierarchicalCache::canonicalName( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		return obj;
	}
	fs::path pathObj( obj );
	if ( !pathObj.has_root_directory() )
	{
		throw Exception( std::string( "Invalid object name ") + obj );
	}
	if ( obj[ obj.size() - 1 ] == '/' )
	{
		return obj.substr( 0, obj.size() - 1 );
	}
	return obj;
}

void HierarchicalCache::objectPath( const ObjectHandle &obj, IndexedIO::EntryID &path )
{
	if ( obj == rootName() )
	{
		path = "/";
		return;
	}
	fs::path objPath( obj );
	if ( !objPath.has_root_directory() )
	{
		throw Exception( std::string( "Invalid object name ") + obj );
	}
	bool first = true;
	while ( objPath.has_leaf() )
	{
		std::string leaf = objPath.leaf();
		if (first)
		{
			if ( leaf == "/" )
			{
				path = "/children";
			}
			else
			{
				path = leaf;
			}
			first = false;
		}
		else
		{
			path = ( fs::path(leaf) / "children" / path).string();
		}
		
		objPath = objPath.branch_path();
	}
}

void HierarchicalCache::attributesPath( const ObjectHandle &obj, IndexedIO::EntryID &path )
{
	IndexedIO::EntryID ioPath;
	objectPath( obj, ioPath );
	fs::path objPath( ioPath );
	objPath /= "attributes";
	path = objPath.string();	
}

void HierarchicalCache::attributePath( const ObjectHandle &obj, const AttributeHandle &attr, IndexedIO::EntryID &path )
{
	IndexedIO::EntryID ioPath;
	objectPath( obj, ioPath );
	fs::path objPath( ioPath );
	objPath /= "attributes";
	objPath /= attr;
	path = objPath.string();	
}

void HierarchicalCache::updateNode( const ObjectHandle &obj )
{
	// compute bounding box.
	Imath::Box3f myBox;

	IndexedIO::EntryID p;
	objectPath( obj, p );
	try
	{
		m_io->chdir(p);
	}
	catch( IECore::Exception &e )
	{
		// object doesn't exist anymore...
		return;
	}
	
	VisibleRenderablePtr s = loadShape();
	if ( s )
	{
		myBox.extendBy( s->bound() );
	}
	// get children bounding boxes.
	bool hasChildren = false;
	m_io->chdir(p);
	try
	{
		m_io->chdir( "children" );
		hasChildren = true;
	}
	catch( IECore::Exception &e )
	{
		// no children...
	}
	if ( hasChildren )
	{
		IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
		IndexedIO::EntryList directories = m_io->ls(filter);
		
		for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
		{
			m_io->chdir( it->id() );
			Imath::Box3f box;
			
			if ( loadBound( box ) )
			{
				myBox.extendBy( box );
			}
			m_io->chdir( ".." );
		}
		m_io->chdir( ".." );
	}

	if ( !myBox.isEmpty() )
	{
		Imath::M44f m;
		
		if ( loadTransform( m ) )
		{
			myBox = Imath::transform( myBox, m );
		}
	}

	// update bounding box in file.
	updateBound( obj, myBox );
}

void HierarchicalCache::updateBound( const ObjectHandle &obj, Imath::Box3f box )
{
	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir( p );
	Imath::Box3f oldBox;
	
	bool hasOldBox = loadBound( oldBox );

	if ( hasOldBox && oldBox == box )
	{
		// no need to update file.
		return;
	}
	if ( !hasOldBox && box.isEmpty() )
	{
		// if the bounding box is empty we don't have to write it.
		return;
	}

	m_io->write( "boundingBox", (const float *)&(box), 6 );

	if ( obj != rootName() )
	{
		// mark parent node as dirty.
		m_dependency->setDirty( parentName( obj ) );
	}
}

IndexedIO::EntryID HierarchicalCache::guaranteeObject( const ObjectHandle &obj )
{
	IndexedIO::EntryID p;
	objectPath( obj, p );
	try
	{
		m_io->chdir( p );
	}
	catch( IECore::Exception &e )
	{
		m_io->chdir( guaranteeObject( parentName( obj ) ) );
		m_io->mkdir( "children" );
		m_io->chdir( "children" );
		m_io->mkdir( relativeName( obj ) );

		// set this node as dirty
		m_dependency->setDirty( obj );
	}
	return p;
}

void HierarchicalCache::write( const ObjectHandle &obj, const AttributeHandle &attr, ObjectPtr data)
{
	guaranteeObject( canonicalName( obj ) );

	IndexedIO::EntryID p;
	attributePath( obj, "", p );
	try
	{
		m_io->chdir( p );
	}
	catch (IECore::Exception &e)
	{
		// there's no attributes directory...
		objectPath( obj, p );
		m_io->chdir( p );
		m_io->mkdir( "attributes" );
		m_io->chdir( "attributes" );
	}

	data->save(m_io, attr);
}

void HierarchicalCache::write( const ObjectHandle &obj, const Imath::M44f &matrix )
{
	if ( obj == rootName() )
	{
		throw Exception( "Root node cannot have a transform." );
	}

	ObjectHandle objName = canonicalName( obj );

	IndexedIO::EntryID p = guaranteeObject( objName );
	m_io->chdir( p );

	m_io->write( "transformMatrix", (const float *)&(matrix), 16 );

	try
	{
		m_io->rm( "shape" );
	}
	catch (IECore::Exception &e)
	{
	}
	// set this node as dirty
	m_dependency->setDirty( objName );

}

void HierarchicalCache::write( const ObjectHandle &obj, ConstVisibleRenderablePtr shape )
{
	if ( obj == rootName() )
	{
		throw Exception( "Root node cannot have a shape." );
	}

	ObjectHandle objName = canonicalName( obj );
	IndexedIO::EntryID p = guaranteeObject( objName );
	m_io->chdir( p );

	// first make sure it's not a transform node.
	try
	{
		m_io->rm( "transformMatrix" );
	}
	catch( ... )
	{
	}
	
	boost::static_pointer_cast<const Object>(shape)->save(m_io, "shape" );
	m_io->chdir( ".." );

	// ok, so check if this node has children...
	try
	{
		m_io->chdir( "children" );
	}
	catch( IECore::Exception &e )
	{
		// no children, then compute bounding box for shape right now ( that will mark the parent node as dirty ).
		updateBound( obj, shape->bound() );
		return;
	}
	// may have children... then set this as dirty
	m_dependency->setDirty( objName );
}

void HierarchicalCache::writeHeader( const HeaderHandle &hdr, ObjectPtr data)
{
	m_io->chdir("/HierarchicalCache");
	data->save(m_io, hdr);
}

ObjectPtr HierarchicalCache::read( const ObjectHandle &obj, const AttributeHandle &attr )
{
	IndexedIO::EntryID p;
	attributesPath( obj, p );
	m_io->chdir( p );
	ObjectPtr data = Object::load( m_io, attr );
	return data;
}

CompoundObjectPtr HierarchicalCache::read( const ObjectHandle &obj )
{
	CompoundObjectPtr dict = new CompoundObject();

	IndexedIO::EntryID p;
	attributesPath( obj, p );
	m_io->chdir( p );

	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		ObjectPtr data = Object::load( m_io, it->id() );
		dict->members()[ it->id() ] = data;
	}

	return dict;
}

ObjectPtr HierarchicalCache::readHeader( const HeaderHandle &hdr )
{
	m_io->chdir("/HierarchicalCache");
	ObjectPtr data = Object::load( m_io, hdr );
	return data;
}

CompoundObjectPtr HierarchicalCache::readHeader( )
{
	CompoundObjectPtr dict = new CompoundObject();

	m_io->chdir("/HierarchicalCache");

	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		ObjectPtr data = Object::load( m_io, it->id() );
		dict->members()[ it->id() ] = data;
	}

	return dict;
}

void HierarchicalCache::headers(std::vector<HierarchicalCache::HeaderHandle> &hds)
{
	hds.clear();
	
	m_io->chdir("/HierarchicalCache");
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	hds.reserve( directories.size() );	
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		hds.push_back( it->id() );
	}
}

void HierarchicalCache::objects(std::vector<HierarchicalCache::ObjectHandle> &objs)
{
	objs.clear();
	m_io->chdir( "/" );
	recursiveObjects( objs );
}

void HierarchicalCache::recursiveObjects(std::vector<ObjectHandle> &objs, const ObjectHandle parent, size_t totalSize)
{
	try
	{
		m_io->chdir( "children" );
	}
	catch (IECore::Exception &e)
	{
		// no directory, no children
		return;
	}
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	totalSize += directories.size();
	objs.reserve( totalSize );

	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		ObjectHandle objName = absoluteName( it->id(), parent );
		objs.push_back( objName );
		m_io->chdir( it->id() );
		recursiveObjects( objs, objName, totalSize );
		m_io->chdir( ".." );
	}
	m_io->chdir( ".." );
}

bool HierarchicalCache::contains( const ObjectHandle &obj )
{
	IndexedIO::EntryID p;
	objectPath( obj, p );
	try
	{
		m_io->chdir( p );
	} 
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

bool HierarchicalCache::contains( const ObjectHandle &obj, const AttributeHandle &attr )
{
	IndexedIO::EntryID p;
	attributePath( obj, attr, p );
	try
	{
		m_io->chdir( p );
	} 
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

void HierarchicalCache::attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs)
{
	attrs.clear();

	IndexedIO::EntryID p;
	attributePath( obj, "", p );

	try 
	{
		m_io->chdir(p);
	}
	catch (IECore::Exception &e)
	{
		objectPath( obj, p );
		m_io->chdir( p );
		// if the object exists, returns empty list because there could be no "attributes" directory.
		return;
	}
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	attrs.reserve( directories.size() );
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		attrs.push_back( it->id() );
	}
}

void HierarchicalCache::attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs)
{
	attrs.clear();
	
	IndexedIO::EntryID p;
	attributePath( obj, "", p );
	try 
	{
		m_io->chdir(p);
	}
	catch (IECore::Exception &e)
	{
		objectPath( obj, p );
		m_io->chdir( p );
		// if the object exists, returns empty list because there could be no "attributes" directory.
		return;
	}
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	filter->add( new IndexedIORegexFilter(  regex ) );
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	attrs.reserve( directories.size() );
	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		attrs.push_back( it->id() );
	}
}

void HierarchicalCache::remove( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		throw Exception("Root node is not removable.");
	}
	ObjectHandle parent = parentName(obj);
	IndexedIO::EntryID p;
	objectPath( parent, p );
	m_io->chdir(p);
	m_io->chdir("children");
	m_io->rm( relativeName(obj) );

	// deleted items are "clean" 
	m_dependency->clear( canonicalName( obj ) );
	// parent is dirty now.
	m_dependency->setDirty( parent );
}

void HierarchicalCache::remove( const ObjectHandle &obj, const AttributeHandle &attr )
{
	IndexedIO::EntryID p;
	attributePath( obj, "", p );
	m_io->chdir(p);
	m_io->rm( attr );
}

void HierarchicalCache::removeHeader( const HeaderHandle &hdr )
{
	m_io->chdir("/HierarchicalCache");

	m_io->rm( hdr );
}

bool HierarchicalCache::isShape( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		return false;
	}

	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);
	try
	{
		m_io->chdir("shape");
	}
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

bool HierarchicalCache::isTransform( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		return false;
	}

	try
	{
		transformMatrix( obj );
	}
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

void HierarchicalCache::children(  const ObjectHandle &obj, std::vector<ObjectHandle> &children )
{
	children.clear();

	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);

	try 
	{
		m_io->chdir( "children" );
	}
	catch (IECore::Exception &e)
	{
		// if children doesn't exist, returns empty list.
		return;
	}
	
	IndexedIOEntryTypeFilterPtr filter = new IndexedIOEntryTypeFilter(IndexedIO::Directory);
	
	IndexedIO::EntryList directories = m_io->ls(filter);
	
	children.reserve( directories.size() );

	for (IndexedIO::EntryList::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		children.push_back( absoluteName( it->id(), obj ) );
	}
}

bool HierarchicalCache::loadTransform( Imath::M44f &m )
{
	try
	{
		float *p = (float *)&m;
		m_io->read( "transformMatrix", p, 16 );
	}
	catch (IECore::Exception &e)
	{
		return false;
	}
	return true;
}

Imath::M44f HierarchicalCache::transformMatrix( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		throw Exception( "Root node does not have transform." );
	}

	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);

	Imath::M44f m;
	
	if ( !loadTransform( m ) )
	{
		throw Exception( "Invalid transform!" );
	}
	return m;
}

VisibleRenderablePtr HierarchicalCache::loadShape( )
{
	try
	{
		ObjectPtr data = Object::load( m_io, "shape" );
		return runTimeCast<VisibleRenderable>( data );
	}
	catch( IECore::Exception &e)
	{
		return 0;
	}
}

VisibleRenderablePtr HierarchicalCache::shape( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		throw Exception("Root node does not have shape.");
	}
	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);
	VisibleRenderablePtr s = loadShape();
	if ( !s )
	{
		throw Exception( "Invalid shape!" );
	}
	return s;
}

Imath::M44f HierarchicalCache::globalTransformMatrix( const ObjectHandle &obj )
{
	if ( obj == rootName() )
	{
		return Imath::M44f();
	}
	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);

	return recursiveTransformMatrix( obj, Imath::M44f() );
}

Imath::M44f HierarchicalCache::recursiveTransformMatrix( const ObjectHandle &obj, const Imath::M44f &world )
{
	ObjectHandle parent = parentName( obj );
	Imath::M44f m;
	if ( !loadTransform( m ) )
	{
		m = Imath::M44f();
	}
	if ( parent == rootName() )
	{
		// end of recursion
		return m * world;
	}
	// go to parent node
	m_io->chdir("../..");
	return recursiveTransformMatrix( parent, m * world );
}

bool HierarchicalCache::loadBound( Imath::Box3f &b )
{
	try
	{
		float *p = (float *)&(b);
		m_io->read( "boundingBox", p, 6 );
	}
	catch( IECore::Exception &e)
	{
		return false;
	}
	return true;
}

Imath::Box3f HierarchicalCache::bound( const ObjectHandle &obj )
{
	m_dependency->update( canonicalName( obj ) );

	IndexedIO::EntryID p;
	objectPath( obj, p );
	m_io->chdir(p);

	Imath::Box3f box;
	
	if ( !loadBound( box ) )
	{
		// return an empty bounding box
		return Imath::Box3f();
	}
	return box;
}
