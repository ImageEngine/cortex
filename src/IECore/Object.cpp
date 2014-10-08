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

#include "IECore/Object.h"
#include "IECore/MurmurHash.h"

#include "boost/format.hpp"
#include "boost/tokenizer.hpp"

#include <iostream>


using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Object );

const Object::AbstractTypeDescription<Object> Object::m_typeDescription;

static IndexedIO::EntryID g_ioVersionEntry("ioVersion");
static IndexedIO::EntryID g_dataEntry("data");
static IndexedIO::EntryID g_typeEntry("type");
const unsigned int Object::m_ioVersion = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// structors
//////////////////////////////////////////////////////////////////////////////////////////

Object::Object()
{

}

Object::~Object()
{

}

//////////////////////////////////////////////////////////////////////////////////////////
// type information structure
//////////////////////////////////////////////////////////////////////////////////////////

struct Object::TypeInformation
{
	typedef std::pair< CreatorFn, void *> CreatorAndData;
	typedef std::map< TypeId, CreatorAndData > TypeIdsToCreatorsMap;
	typedef std::map< std::string, CreatorAndData > TypeNamesToCreatorsMap;

	TypeIdsToCreatorsMap typeIdsToCreators;
	TypeNamesToCreatorsMap typeNamesToCreators;
};

Object::TypeInformation *Object::typeInformation()
{
	// we have a function to return the type information rather than
	// just have it as a static data member as we can't guarantee
	// initialisation order in the latter case - the type information
	// structure might not be initialised when the different types
	// start registering themselves (registration occurs in the constructors
	// for static TypeDescription objects).
	static TypeInformation *i = new TypeInformation;
	return i;
}

//////////////////////////////////////////////////////////////////////////////////////////
// copy context stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::CopyContext::CopyContext()
{
}

ObjectPtr Object::CopyContext::copyInternal( const Object *toCopy )
{
	if( toCopy->refCount() > 1 )
	{
		// object may occur multiple times in the data structure
		// being copied - ensure we don't copy it twice.
		std::map<const Object *, Object *>::const_iterator it = m_copies.find( toCopy );
		if( it!=m_copies.end() )
		{
			return it->second;
		}
		ObjectPtr copy = create( toCopy->typeId() );
		copy->copyFrom( toCopy, this );
		m_copies.insert( std::pair<const Object *, Object *>( toCopy, copy.get() ) );
		return copy;	
	}
	else
	{
		// object can only occur once in the data structure being
		// counted - avoid unnecessary bookkeeping overhead.
		ObjectPtr copy = create( toCopy->typeId() );
		copy->copyFrom( toCopy, this );
		return copy;	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// save context stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::SaveContext::SaveContext( IndexedIOPtr ioInterface )
	:	m_ioInterface( ioInterface ), m_savedObjects( new SavedObjectMap )
{
}

Object::SaveContext::SaveContext( IndexedIOPtr ioInterface, boost::shared_ptr<SavedObjectMap> savedObjects )
	:	m_ioInterface( ioInterface ), m_savedObjects( savedObjects )
{
}

IndexedIOPtr Object::SaveContext::container( const std::string &typeName, unsigned int ioVersion )
{
	IndexedIOPtr typeIO = m_ioInterface->subdirectory( typeName, IndexedIO::CreateIfMissing );
	typeIO->write( g_ioVersionEntry, ioVersion );
	IndexedIOPtr dataIO = typeIO->subdirectory( g_dataEntry, IndexedIO::CreateIfMissing );
	dataIO->removeAll();
	return dataIO;
}

IndexedIO *Object::SaveContext::rawContainer()
{
	return m_ioInterface.get();
}

void Object::SaveContext::save( const Object *toSave, IndexedIO *container, const IndexedIO::EntryID &name )
{
	if ( !toSave )
	{
		throw Exception( "Error trying to save NULL pointer object!" );
	}

	SavedObjectMap::const_iterator it = m_savedObjects->find( toSave );
	if( it!=m_savedObjects->end() )
	{
		container->write( name, &(it->second[0]), it->second.size() );
	}
	else
	{
		bool rootObject = ( m_savedObjects->size() == 0 );
		if ( rootObject )
		{
			if ( container->hasEntry( name ) )
			{
				container->remove( name );
			}
		}
		IndexedIOPtr nameIO = container->createSubdirectory( name );

		IndexedIO::EntryIDList pathParts;
		nameIO->path( pathParts );
		(*m_savedObjects)[toSave] = pathParts;

		nameIO->write( g_typeEntry, toSave->typeName() );

		IndexedIOPtr dataIO = nameIO->createSubdirectory( g_dataEntry );
		dataIO->removeAll();

		SaveContext context( dataIO, m_savedObjects );
		toSave->save( &context );

		// Objects saved on a file can be committed to disk to free memory.
		if ( rootObject )
		{
			nameIO->commit();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// load context stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::LoadContext::LoadContext( ConstIndexedIOPtr ioInterface )
	:	m_ioInterface( ioInterface ), m_loadedObjects( new LoadedObjectMap )
{
}

Object::LoadContext::LoadContext( ConstIndexedIOPtr ioInterface, boost::shared_ptr<LoadedObjectMap> loadedObjects )
	:	m_ioInterface( ioInterface ), m_loadedObjects( loadedObjects )
{
}

ConstIndexedIOPtr Object::LoadContext::container( const std::string &typeName, unsigned int &ioVersion, bool throwIfMissing )
{
	ConstIndexedIOPtr typeIO = m_ioInterface->subdirectory( typeName, throwIfMissing ? IndexedIO::ThrowIfMissing : IndexedIO::NullIfMissing );
	if ( !typeIO )
	{
		return 0;
	}
	unsigned int v;
	typeIO->read( g_ioVersionEntry, v );
	if( v > ioVersion )
	{
		throw( IOException( "File version greater than library version." ) );
	}
	ioVersion = v;
	return typeIO->subdirectory( g_dataEntry, throwIfMissing ? IndexedIO::ThrowIfMissing : IndexedIO::NullIfMissing );
}

const IndexedIO *Object::LoadContext::rawContainer()
{
	return m_ioInterface.get();
}

ObjectPtr Object::LoadContext::loadObjectOrReference( const IndexedIO *container, const IndexedIO::EntryID &name )
{
	IndexedIO::Entry e = container->entry( name );
	if( e.entryType()==IndexedIO::File )
	{
		IndexedIO::EntryIDList pathParts;
		if ( e.dataType() == IndexedIO::InternedStringArray )
		{
			pathParts.resize( e.arrayLength() );
			InternedString *p = &(pathParts[0]); 
			container->read( name, p, e.arrayLength() );
		}
		else 
		{
			// for backward compatibility...
			string path;
			container->read( name, path );
			typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
			// \todo: this would have trouble if the name of the object contains slashes...
			Tokenizer tokens(path, boost::char_separator<char>("/"));
			Tokenizer::iterator t = tokens.begin();

			for ( ; t != tokens.end(); t++ )
			{
				pathParts.push_back( *t );
			}
		}
		std::pair< LoadedObjectMap::iterator,bool > ret = m_loadedObjects->insert( std::pair<IndexedIO::EntryIDList, ObjectPtr>( pathParts, NULL ) );
		if ( ret.second )
		{
			// jump to the path..
			ConstIndexedIOPtr ioObject = m_ioInterface->directory( pathParts );
			// add the loaded object to the map.
			ret.first->second = loadObject( ioObject.get() );
		}
		return ret.first->second;
	}
	else
	{
		ConstIndexedIOPtr ioObject = container->subdirectory( name );

		IndexedIO::EntryIDList pathParts;
		ioObject->path( pathParts );

		std::pair< LoadedObjectMap::iterator,bool > ret = m_loadedObjects->insert( std::pair<IndexedIO::EntryIDList, ObjectPtr>( pathParts, NULL ) );
		if ( ret.second )
		{
			// add the loaded object to the map.
			ret.first->second = loadObject( ioObject.get() );
		}
		return ret.first->second;
	}
}

// this function can only load concrete objects. it can't load references to
// objects. path is relative to the root of m_ioInterface
ObjectPtr Object::LoadContext::loadObject( const IndexedIO *container )
{
	ObjectPtr result = 0;
	string type = "";
	container->read( g_typeEntry, type );
	ConstIndexedIOPtr dataIO = container->subdirectory( g_dataEntry );
	result = create( type );
	LoadContextPtr context = new LoadContext( dataIO, m_loadedObjects );
	result->load( context );
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// memory accumulator stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::MemoryAccumulator::MemoryAccumulator()
	:	m_total( 0 )
{
}

void Object::MemoryAccumulator::accumulate( size_t bytes )
{
	m_total += bytes;
}

void Object::MemoryAccumulator::accumulate( const Object *object )
{
	if( object->refCount() > 1 )
	{
		// object may occur multiple times in the data structure
		// being counted - ensure that we don't count it twice.
		if( m_accumulated.insert( object ).second )
		{
			object->memoryUsage( *this );
		}
	}
	else
	{
		// object can only occur once in the data structure being
		// counted - avoid unnecessary bookkeeping overhead.
		object->memoryUsage( *this );
	}
}

void Object::MemoryAccumulator::accumulate( const void *ptr, size_t bytes )
{
	if( m_accumulated.find( ptr )==m_accumulated.end() )
	{
		m_total += bytes;
		m_accumulated.insert( ptr );
	}
}

size_t Object::MemoryAccumulator::total() const
{
	return m_total;
}

//////////////////////////////////////////////////////////////////////////////////////////
// object interface stuff
//////////////////////////////////////////////////////////////////////////////////////////

ObjectPtr Object::copy() const
{
	CopyContext c;
	ObjectPtr result = c.copy( this );
	return result;
}

void Object::save( IndexedIOPtr ioInterface, const IndexedIO::EntryID &name ) const
{
	boost::shared_ptr<SaveContext> context( new SaveContext( ioInterface ) );
	context->save( this, ioInterface.get(), name );
}

void Object::copyFrom( const Object *toCopy )
{
	if ( !toCopy->isInstanceOf( typeId() ) )
	{
		throw InvalidArgumentException( ( boost::format( "\"%s\" is not an instance of \"%s\"" ) % toCopy->typeName() % typeName() ).str() );
	}
	
	CopyContext context;
	copyFrom( toCopy, &context );
}

void Object::copyFrom( const Object *toCopy, CopyContext *context )
{
}

bool Object::isEqualTo( const Object *other ) const
{
	if( typeId()!=other->typeId() )
	{
		return false;
	}
	// well, we ain't got no member data so as far as we're concerned
	// we always the same as the the t'other - it's up to the derived classes
	// to decide from here.
	return true;
}

bool Object::operator==( const Object &other ) const
{
	return isEqualTo( &other );
}

bool Object::isNotEqualTo( const Object *other ) const
{
	return !isEqualTo( other );
}

bool Object::operator!=( const Object &other ) const
{
	return isNotEqualTo( &other );
}

void Object::save( SaveContext *context ) const
{
}

void Object::load( LoadContextPtr context )
{
}

size_t Object::memoryUsage() const
{
	MemoryAccumulator m;
	m.accumulate( this );
	return m.total();
}

void Object::memoryUsage( MemoryAccumulator &accumulator ) const
{
	accumulator.accumulate( sizeof( RefCount ) );
}

MurmurHash Object::hash() const
{
	MurmurHash h;
	hash( h );
	return h;
}

void Object::hash( MurmurHash &h ) const
{
	h.append( (int)typeId() );
}

//////////////////////////////////////////////////////////////////////////////////////////
// object factory stuff
//////////////////////////////////////////////////////////////////////////////////////////

bool Object::isType( TypeId typeId )
{
	TypeInformation *i = typeInformation();
	return i->typeIdsToCreators.find( typeId )!=i->typeIdsToCreators.end();
}

bool Object::isType( const std::string &typeName )
{
	TypeInformation *i = typeInformation();
	return i->typeNamesToCreators.find( typeName )!=i->typeNamesToCreators.end();
}

bool Object::isAbstractType( TypeId typeId )
{
	TypeInformation *i = typeInformation();
	TypeInformation::TypeIdsToCreatorsMap::const_iterator it = i->typeIdsToCreators.find( typeId );
	if( it==i->typeIdsToCreators.end() )
	{
		return false;
	}
	return !it->second.first;
}

bool Object::isAbstractType( const std::string &typeName )
{
	TypeInformation *i = typeInformation();
	TypeInformation::TypeNamesToCreatorsMap::const_iterator it = i->typeNamesToCreators.find( typeName );
	if( it==i->typeNamesToCreators.end() )
	{
		return false;
	}
	return !it->second.first;
}

void Object::registerType( TypeId typeId, const std::string &typeName, CreatorFn creator, void *data )
{
	TypeInformation *i = typeInformation();
	i->typeIdsToCreators[typeId] = TypeInformation::CreatorAndData( creator, data );
	i->typeNamesToCreators[typeName] = TypeInformation::CreatorAndData( creator, data );
}

ObjectPtr Object::create( TypeId typeId )
{
	TypeInformation *i = typeInformation();
	TypeInformation::TypeIdsToCreatorsMap::const_iterator it = i->typeIdsToCreators.find( typeId );
	if( it==i->typeIdsToCreators.end() )
	{
		throw Exception( ( boost::format( "Type %d is not a registered Object type." ) % typeId ).str() );
	}
	const TypeInformation::CreatorAndData &creatorAndData = it->second;

	if( !creatorAndData.first )
	{
		throw Exception( ( boost::format( "Type %d is an abstract type." ) % typeId ).str() );
	}

	return creatorAndData.first( creatorAndData.second );
}

ObjectPtr Object::create( const std::string &typeName )
{
	TypeInformation *i = typeInformation();
	TypeInformation::TypeNamesToCreatorsMap::const_iterator it = i->typeNamesToCreators.find( typeName );
	if( it==i->typeNamesToCreators.end() )
	{
		throw Exception( ( boost::format( "Type \"%s\" is not a registered Object type." ) % typeName ).str() );
	}
	const TypeInformation::CreatorAndData &creatorAndData = it->second;

	if( !creatorAndData.first )
	{
		throw Exception( ( boost::format( "Type \"%s\" is an abstract type." ) % typeName ).str() );
	}

	return creatorAndData.first( creatorAndData.second );
}

ObjectPtr Object::load( ConstIndexedIOPtr ioInterface, const IndexedIO::EntryID &name )
{
	LoadContextPtr context( new LoadContext( ioInterface ) );
	ObjectPtr result = context->load<Object>( ioInterface.get(), name );
	return result;
}
