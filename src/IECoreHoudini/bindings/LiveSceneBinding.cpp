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

#include "IECoreHoudini/CoreHoudini.h"
#include "IECoreHoudini/LiveScene.h"
#include "IECoreHoudini/bindings/LiveSceneBinding.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/SceneInterfaceBinding.h"

using namespace IECoreHoudini;
using namespace boost::python;

static void listToPath( const list &l, IECore::SceneInterface::Path &p )
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

static LiveScenePtr constructor( const std::string n, const list &c, const list &r, double defaultTime )
{
	UT_String nodePath( n );
	IECore::SceneInterface::Path contentPath, rootPath;
	listToPath( c, contentPath );
	listToPath( r, rootPath );

	return new LiveScene( nodePath, contentPath, rootPath, defaultTime );
}

/// \todo: return a PyObject* directly if SideFx provides a swig-free method for creating one from a HOM_Node*
static std::string getNodePath( LiveScene *scene )
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

		bool operator() ( const OP_Node *node, const IECore::SceneInterface::Name &tag, int filter )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			return m_has( CoreHoudini::evalPython( "hou.node( \"" + path.toStdString() + "\" )" ), tag, filter );
		}

		void operator() ( const OP_Node *node, IECore::SceneInterface::NameList &tags, int filter )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			object o = m_read( CoreHoudini::evalPython( "hou.node( \"" + path.toStdString() + "\" )" ), filter );
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			IECorePython::listToSceneInterfaceNameList( l(), tags );
		}

		object m_has;
		object m_read;
};

void registerCustomTags( object hasFn, object readFn )
{
	CustomTagReader reader( hasFn, readFn );
	LiveScene::registerCustomTags( reader, reader );
}

class CustomAttributeReader
{
	public :
		CustomAttributeReader( object namesFn, object readFn ) : m_names( namesFn ), m_read( readFn )
		{
		}

		IECore::ConstObjectPtr operator() ( const OP_Node *node, const IECore::SceneInterface::Name &attr, double time )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			return extract<IECore::ConstObjectPtr>( m_read( CoreHoudini::evalPython( "hou.node( \"" + path.toStdString() + "\" )" ), attr, time ) );
		}

		void operator() ( const OP_Node *node, IECore::SceneInterface::NameList &attributes )
		{
			UT_String path;
			node->getFullPath( path );
			IECorePython::ScopedGILLock gilLock;
			object o = m_names( CoreHoudini::evalPython( "hou.node( \"" + path.toStdString() + "\" )" ) );
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			IECorePython::listToSceneInterfaceNameList( l(), attributes );
		}

		object m_names;
		object m_read;
};

void registerCustomAttributes( object namesFn, object readFn )
{
	CustomAttributeReader reader( namesFn, readFn );
	LiveScene::registerCustomAttributes( reader, reader );
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
