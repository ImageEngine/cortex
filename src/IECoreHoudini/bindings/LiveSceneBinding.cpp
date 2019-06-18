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

#include "IECoreHoudini/bindings/LiveSceneBinding.h"

#include "IECoreHoudini/CoreHoudini.h"
#include "IECoreHoudini/LiveScene.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "boost/python/suite/indexing/container_utils.hpp"

using namespace IECoreHoudini;
using namespace boost::python;

namespace
{

void listToPath( const list &l, IECoreScene::SceneInterface::Path &p )
{
	int listLen = IECorePython::len( l );
	for (int i = 0; i < listLen; i++ )
	{
		extract< std::string > ex( l[i] );
		if ( !ex.check() )
		{
			throw IECore::InvalidArgumentException( std::string( "Invalid path! Should be a list of strings!" ) );
		}
		p.push_back( ex() );
	}
}


LiveScenePtr constructor( const std::string n, const list &c, const list &r, double defaultTime )
{
	UT_String nodePath( n );
	IECoreScene::SceneInterface::Path contentPath, rootPath;
	listToPath( c, contentPath );
	listToPath( r, rootPath );

	// constructing a LiveScene can cause SOPs to cook (via the DetailSplitter),
	// which can cause further python evaluations (eg parm expressions), so we
	// must release the GIL to avoid deadlocks.
	IECorePython::ScopedGILRelease gilRelease;
	return new LiveScene( nodePath, contentPath, rootPath, defaultTime );
}

/// \todo: return a PyObject* directly if SideFx provides a swig-free method for creating one from a HOM_Node*
std::string getNodePath( LiveScene *scene )
{
	const OP_Node *node = scene->node();
	if ( !node )
	{
		return 0;
	}

	UT_String path;
	node->getFullPath( path );

	return path.toStdString();
}

//! utility function to acquire a houdini node by it's path in the node graph
boost::python::object getNodeAsPython( const UT_String &path )
{
	static boost::python::object houModule = boost::python::import( "hou" );
	static boost::python::object nodeFn = houModule.attr( "node" );

	return nodeFn( path.c_str() );
}

} // namespace


IECore::DataPtr readWorldTransform( LiveScene &scene, double time )
{
	if ( IECore::ConstDataPtr t = scene.readWorldTransform( time ) )
	{
		return t->copy();
	}

	return 0;
}

class CustomTagReader
{
	public :
		CustomTagReader( object hasFn, object readFn ) : m_has( hasFn ), m_read( readFn )
		{
		}

		bool operator() ( const OP_Node *node, const IECoreScene::SceneInterface::Name &tag, int filter )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;

			return m_has( getNodeAsPython( path ), tag, filter );
		}

		void operator() ( const OP_Node *node, IECoreScene::SceneInterface::NameList &tags, int filter )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			object o = m_read( getNodeAsPython ( path ), filter );
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			container_utils::extend_container( tags, l() );
		}

		object m_has;
		object m_read;
};

void registerCustomTags( object hasFn, object readFn )
{
	CustomTagReader reader( hasFn, readFn );
	LiveScene::registerCustomTags( reader, reader, false );
}

class CustomAttributeReader
{
	public :
		CustomAttributeReader( object namesFn, object readFn ) : m_names( namesFn ), m_read( readFn )
		{
		}

		IECore::ConstObjectPtr operator() ( const OP_Node *node, const IECoreScene::SceneInterface::Name &attr, double time )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			return extract<IECore::ConstObjectPtr>( m_read( getNodeAsPython( path ), attr, time ) );
		}

		void operator() ( const OP_Node *node, IECoreScene::SceneInterface::NameList &attributes )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			object o = m_names( getNodeAsPython( path ) );
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			container_utils::extend_container( attributes, l() );
		}

		object m_names;
		object m_read;
};

void registerCustomAttributes( object namesFn, object readFn )
{
	CustomAttributeReader reader( namesFn, readFn );
	LiveScene::registerCustomAttributes( reader, reader, false );
}

void IECoreHoudini::bindLiveScene()
{
	IECorePython::RunTimeTypedClass<LiveScene>()
		.def( init<>() )
		.def( "__init__", make_constructor( &constructor, default_call_policies(), ( arg( "nodePath" ), arg( "contentPath" ) = list(), arg( "rootPath" ) = list(), arg( "defaultTime" ) = std::numeric_limits<double>::infinity() ) ) )
		.def( "getDefaultTime", &LiveScene::getDefaultTime )
		.def( "setDefaultTime", &LiveScene::setDefaultTime )
		.def( "embedded", &LiveScene::embedded )
		.def( "_getNodePath", &getNodePath )
		.def( "readWorldTransform", &readWorldTransform )
		.def( "readWorldTransformAsMatrix", &LiveScene::readWorldTransformAsMatrix )
		.def( "registerCustomTags", registerCustomTags ).staticmethod( "registerCustomTags" )
		.def( "registerCustomAttributes", registerCustomAttributes ).staticmethod( "registerCustomAttributes" )
	;
}
