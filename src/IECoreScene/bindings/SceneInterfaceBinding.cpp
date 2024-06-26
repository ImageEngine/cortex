//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "SceneInterfaceBinding.h"

#include "IECoreScene/SceneInterface.h"
#include "IECoreScene/SharedSceneInterfaces.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "boost/python/suite/indexing/container_utils.hpp"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{


inline list arrayToList( std::vector<IndexedIO::EntryID> &ids )
{
	list result;
	for( SceneInterface::NameList::const_iterator it = ids.begin(); it != ids.end(); it++ )
	{
		result.append( ( *it ).value() );
	}
	return result;
}

static list childNames( const SceneInterface &m )
{
	SceneInterface::NameList n;
	m.childNames( n );
	return arrayToList( n );
}

static list path( const SceneInterface &m )
{
	SceneInterface::Path p;
	m.path( p );
	return arrayToList( p );
}

static std::string pathAsString( const SceneInterface &m )
{
	SceneInterface::Path p;
	m.path( p );
	std::string str;
	SceneInterface::pathToString( p, str );
	return str;
}

static SceneInterfacePtr nonConstScene( SceneInterface &m, list l, SceneInterface::MissingBehaviour b )
{
	SceneInterface::Path p;
	container_utils::extend_container( p, l );
	return m.scene( p, b );
}

static list attributeNames( const SceneInterface &m )
{
	SceneInterface::NameList a;
	m.attributeNames( a );
	return arrayToList( a );
}

static std::string pathToString( list l )
{
	SceneInterface::Path p;
	container_utils::extend_container( p, l );
	std::string str;
	SceneInterface::pathToString( p, str );
	return str;
}

static list stringToPath( std::string str )
{
	SceneInterface::Path p;
	SceneInterface::stringToPath( str, p );
	return arrayToList( p );
}

static list supportedExtensions( IndexedIO::OpenMode modes )
{
	std::vector<std::string> e = SceneInterface::supportedExtensions( modes );
	list result;
	for( unsigned int i = 0; i < e.size(); i++ )
	{
		result.append( e[i] );
	}
	return result;
}

static dict readObjectPrimitiveVariables( const SceneInterface &m, list varNameList, double time )
{
	SceneInterface::NameList v;
	container_utils::extend_container( v, varNameList );

	PrimitiveVariableMap varMap = m.readObjectPrimitiveVariables( v, time );
	dict result;
	for( PrimitiveVariableMap::const_iterator it = varMap.begin(); it != varMap.end(); it++ )
	{
		result[it->first] = it->second;
	}
	return result;
}

list readTags( const SceneInterface &m, int filter )
{
	SceneInterface::NameList tags;
	m.readTags( tags, filter );
	list result;
	for( SceneInterface::NameList::const_iterator it = tags.begin(); it != tags.end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

void writeTags( SceneInterface &m, list tagList )
{
	SceneInterface::NameList v;
	container_utils::extend_container( v, tagList );
	m.writeTags( v );
}

DataPtr readTransform( SceneInterface &m, double time, bool copy )
{
	ConstDataPtr t = m.readTransform( time );
	if( t )
	{
		return copy ? t->copy() : boost::const_pointer_cast<Data>( t );
	}
	return nullptr;
}

ObjectPtr readAttribute( SceneInterface &m, const SceneInterface::Name &name, double time, bool copy )
{
	ConstObjectPtr o = m.readAttribute( name, time );
	if( o )
	{
		return copy ? o->copy() : boost::const_pointer_cast<Object>( o );
	}
	return nullptr;
}

ObjectPtr readObject( SceneInterface &m, double time, const IECore::Canceller *canceller, bool copy )
{
	ScopedGILRelease gilRelease;
	ConstObjectPtr o = m.readObject( time, canceller );
	if( o )
	{
		return copy ? o->copy() : boost::const_pointer_cast<Object>( o );
	}
	return nullptr;
}

static MurmurHash sceneHash( SceneInterface &m, SceneInterface::HashType hashType, double time )
{
	MurmurHash h;
	m.hash( hashType, time, h );
	return h;
}

static  list setNames( const SceneInterface &m, bool includeDescendantSets = true   )
{
	SceneInterface::NameList a = m.setNames( includeDescendantSets );
	return arrayToList( a );
}

static MurmurHash hashSet( SceneInterface &m, const SceneInterface::Name &name)
{
	MurmurHash h;
	m.hashSet( name,  h );
	return h;
}

void bindSceneInterface()
{
	SceneInterfacePtr (SceneInterface::*nonConstChild)(const SceneInterface::Name &, SceneInterface::MissingBehaviour) = &SceneInterface::child;

	// make the SceneInterface class first
	IECorePython::RunTimeTypedClass<SceneInterface> sceneInterfaceClass;

	{
		// then define all the nested types
		scope s( sceneInterfaceClass );

		enum_< SceneInterface::MissingBehaviour > ("MissingBehaviour")
			.value("ThrowIfMissing", SceneInterface::ThrowIfMissing)
			.value("NullIfMissing", SceneInterface::NullIfMissing)
			.value("CreateIfMissing", SceneInterface::CreateIfMissing)
			.export_values()
		;

		enum_< SceneInterface::TagFilter > ("TagFilter")
			.value("DescendantTag", SceneInterface::DescendantTag)
			.value("LocalTag", SceneInterface::LocalTag)
			.value("AncestorTag", SceneInterface::AncestorTag)
			.value("EveryTag", SceneInterface::EveryTag)
			.export_values()
		;

		enum_< SceneInterface::HashType > ("HashType")
			.value("TransformHash", SceneInterface::TransformHash)
			.value("AttributesHash", SceneInterface::AttributesHash)
			.value("BoundHash", SceneInterface::BoundHash)
			.value("ObjectHash", SceneInterface::ObjectHash)
			.value("ChildNamesHash", SceneInterface::ChildNamesHash)
			.value("HierarchyHash", SceneInterface::HierarchyHash)
			.export_values()
		;

	}

	// now we've defined the nested types, we're able to define the methods for
	// the IndexedIO class itself (we need the definitions for the nested types
	// to exist for defining default values).

	sceneInterfaceClass.def( "path", path )
		.def( "fileName", &SceneInterface::fileName )
		.def( "pathAsString", pathAsString )
		.def( "name", &SceneInterface::name )
		.def( "hasBound", &SceneInterface::hasBound )
		.def( "readBound", &SceneInterface::readBound )
		.def( "writeBound", &SceneInterface::writeBound )
		.def( "readTransform", &readTransform, ( arg( "time" ), arg( "_copy" ) = true ) )
		.def( "readTransformAsMatrix", &SceneInterface::readTransformAsMatrix )
		.def( "writeTransform", &SceneInterface::writeTransform )
		.def( "hasAttribute", &SceneInterface::hasAttribute )
		.def( "attributeNames", attributeNames )
		.def( "readAttribute", &readAttribute, ( arg( "name" ), arg( "time" ), arg( "_copy" ) = true ) )
		.def( "writeAttribute", &SceneInterface::writeAttribute )
		.def( "hasTag", &SceneInterface::hasTag, ( arg( "name" ), arg( "filter" ) = SceneInterface::LocalTag ) )
		.def( "readTags", readTags, ( arg( "filter" ) = SceneInterface::LocalTag ) )
		.def( "writeTags", writeTags )
		.def( "setNames", &setNames, ( arg_( "includeDescendantSets" ) = true ) )
		.def( "writeSet", &SceneInterface::writeSet )
		.def( "hashSet", &hashSet )
		.def( "readSet", &SceneInterface::readSet, ( arg_("name"), arg_( "includeDescendantSets" ) = true, arg_( "canceller" ) = object() ) )
		.def( "readObject", &readObject, ( arg_( "time" ), arg_( "canceller" ) = object(), arg( "_copy" ) = true ) )
		.def( "readObjectPrimitiveVariables", &readObjectPrimitiveVariables )
		.def( "writeObject", &SceneInterface::writeObject )
		.def( "hasObject", &SceneInterface::hasObject )
		.def( "hasChild", &SceneInterface::hasChild )
		.def( "childNames", &childNames )
		.def( "child", nonConstChild, ( arg( "name" ), arg( "missingBehaviour" ) = SceneInterface::ThrowIfMissing ) )
		.def( "createChild", &SceneInterface::createChild )
		.def( "scene", &nonConstScene, ( arg( "path" ), arg( "missingBehaviour" ) = SceneInterface::ThrowIfMissing ) )
		.def( "hash", &sceneHash )

		.def( "pathToString", pathToString ).staticmethod("pathToString")
		.def( "stringToPath", stringToPath ).staticmethod("stringToPath")
		.def( "create", SceneInterface::create ).staticmethod( "create" )
		.def( "supportedExtensions", supportedExtensions, ( arg("modes") = IndexedIO::Read|IndexedIO::Write|IndexedIO::Append ) ).staticmethod( "supportedExtensions" )

		.def_readonly("visibilityName", &SceneInterface::visibilityName )
	;
}

} // namespace IECoreSceneModule
