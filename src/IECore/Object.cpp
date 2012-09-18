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

#include "IECore/Object.h"
#include "IECore/IndexedIOPath.h"
#include "IECore/MurmurHash.h"

#include "boost/format.hpp"

#include <iostream>

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Object );

const Object::AbstractTypeDescription<Object> Object::m_typeDescription;
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
// save context stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::SaveContext::SaveContext( IndexedIOInterfacePtr ioInterface )
	:	m_ioInterface( ioInterface ), m_root( ioInterface->pwd() ), m_savedObjects( new SavedObjectMap ), m_containerRoots( new ContainerRootsMap )
{
}

Object::SaveContext::SaveContext( IndexedIOInterfacePtr ioInterface, const IndexedIO::EntryID &root,
	boost::shared_ptr<SavedObjectMap> savedObjects, boost::shared_ptr<ContainerRootsMap> containerRoots )
	:	m_ioInterface( ioInterface ), m_root( root ), m_savedObjects( savedObjects ), m_containerRoots( containerRoots )
{
}

IndexedIOInterfacePtr Object::SaveContext::container( const std::string &typeName, unsigned int ioVersion )
{

	m_ioInterface->chdir( m_root );

		m_ioInterface->mkdir( typeName );
		m_ioInterface->chdir( typeName );
			m_ioInterface->write( "ioVersion", ioVersion );
			m_ioInterface->mkdir( "data" );
			m_ioInterface->chdir( "data" );
				IndexedIOInterfacePtr container = m_ioInterface->resetRoot();
				(*m_containerRoots)[container] = m_ioInterface->pwd();

	return container;
}

IndexedIOInterfacePtr Object::SaveContext::rawContainer()
{
	m_ioInterface->chdir( m_root );
	return m_ioInterface->resetRoot();
}

void Object::SaveContext::save( const Object *toSave, IndexedIOInterfacePtr container, const IndexedIO::EntryID &name )
{
	SavedObjectMap::const_iterator it = m_savedObjects->find( toSave );
	if( it!=m_savedObjects->end() )
	{
		container->write( name, it->second );
	}
	else
	{

		IndexedIO::EntryID d = container->pwd();

		container->mkdir( name );
		container->chdir( name );

			(*m_savedObjects)[toSave] = (*m_containerRoots)[container] + container->pwd();

			container->write( "type", toSave->typeName() );
			container->mkdir( "data" );
			container->chdir( "data" );

				IndexedIO::EntryID newRoot = (*m_containerRoots)[container] + container->pwd();
				SaveContext context( m_ioInterface, newRoot, m_savedObjects, m_containerRoots );
				toSave->save( &context );

		container->chdir( d );

		assert( container->pwd()==d );

	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// load context stuff
//////////////////////////////////////////////////////////////////////////////////////////

Object::LoadContext::LoadContext( IndexedIOInterfacePtr ioInterface )
	:	m_ioInterface( ioInterface ), m_root( "/" ), m_loadedObjects( new LoadedObjectMap ), m_containerRoots( new ContainerRootsMap )
{
}

Object::LoadContext::LoadContext( IndexedIOInterfacePtr ioInterface, const IndexedIO::EntryID &root, boost::shared_ptr<LoadedObjectMap> loadedObjects, boost::shared_ptr<ContainerRootsMap> containerRoots )
	:	m_ioInterface( ioInterface ), m_root( root ), m_loadedObjects( loadedObjects ), m_containerRoots( containerRoots )
{
}

IndexedIOInterfacePtr Object::LoadContext::container( const std::string &typeName, unsigned int &ioVersion )
{
	m_ioInterface->chdir( m_root );

		m_ioInterface->chdir( typeName );
			unsigned int v;
			m_ioInterface->read( "ioVersion", v );
			if( v > ioVersion )
			{
				throw( IOException( "File version greater than library version." ) );
			}
			ioVersion = v;
			m_ioInterface->chdir( "data" );
				IndexedIOInterfacePtr container = m_ioInterface->resetRoot();
				(*m_containerRoots)[container] = m_ioInterface->pwd();

	return container;
}

IndexedIOInterfacePtr Object::LoadContext::rawContainer()
{
	m_ioInterface->chdir( m_root );
	return m_ioInterface->resetRoot();
}

ObjectPtr Object::LoadContext::loadObjectOrReference( IndexedIOInterfacePtr container, const IndexedIO::EntryID &name )
{
	IndexedIO::Entry e = container->ls( name );
	if( e.entryType()==IndexedIO::File )
	{
		string path;
		container->read( name, path );
		return loadObject( path );
	}
	else
	{
		IndexedIOPath path( (*m_containerRoots)[container] );
		path.append( container->pwd() );
		path.append( name );
		return loadObject( path.fullPath() );
	}
}

// this function can only load concrete objects. it can't load references to
// objects. path is relative to the root of m_ioInterface
ObjectPtr Object::LoadContext::loadObject( const IndexedIO::EntryID &path )
{
	LoadedObjectMap::iterator it = m_loadedObjects->find( path );
	if( it!=m_loadedObjects->end() )
	{
		return it->second;
	}

	ObjectPtr result = 0;

	m_ioInterface->chdir( m_root );

		m_ioInterface->chdir( path );
			string type = "";
			m_ioInterface->read( "type", type );
			m_ioInterface->chdir( "data" );

				result = create( type );
				LoadContextPtr context = new LoadContext( m_ioInterface, m_ioInterface->pwd(), m_loadedObjects, m_containerRoots );
				result->load( context );

			(*m_loadedObjects)[path] = result;

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
	if( m_accumulated.find( object )==m_accumulated.end() )
	{
		m_accumulated.insert( object );
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
	boost::shared_ptr<CopyContext> c( new CopyContext );
	ObjectPtr result = c->copy( this );
	return result;
}

void Object::save( IndexedIOInterfacePtr ioInterface, const IndexedIO::EntryID &name ) const
{
	// we get a copy of the ioInterface here so the SaveContext can be freed
	// from always having to balance chdirs() to return to the original
	// directory after an operation. this results in fewer chdir calls and faster
	// saving.
	IndexedIOInterfacePtr i = ioInterface->resetRoot();
	boost::shared_ptr<SaveContext> context( new SaveContext( i ) );
	context->save( this, i, name );
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
	return isEqualTo( ConstObjectPtr( &other ) );
}

bool Object::isNotEqualTo( const Object *other ) const
{
	return !isEqualTo( other );
}

bool Object::operator!=( const Object &other ) const
{
	return isNotEqualTo( ConstObjectPtr( &other ) );
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

ObjectPtr Object::load( IndexedIOInterfacePtr ioInterface, const IndexedIO::EntryID &name )
{
	// we get a copy of the ioInterface here so the LoadContext can be freed
	// from always having to balance chdirs() to return to the original
	// directory after an operation. this results in fewer chdir calls and faster
	// loading.

	IndexedIOInterfacePtr i = ioInterface->resetRoot();
	LoadContextPtr context( new LoadContext( i ) );
	ObjectPtr result = context->load<Object>( i, name );
	return result;
}
