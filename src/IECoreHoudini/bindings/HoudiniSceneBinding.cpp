//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/bindings/HoudiniSceneBinding.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

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

static HoudiniScenePtr constructor( const std::string n, const list &c, const list &r, double defaultTime )
{
	UT_String nodePath( n );
	IECore::SceneInterface::Path contentPath, rootPath;
	listToPath( c, contentPath );
	listToPath( r, rootPath );
	
	return new HoudiniScene( nodePath, contentPath, rootPath, defaultTime );
}

/// \todo: return a PyObject* directly if SideFx provides a swig-free method for creating one from a HOM_Node*
static std::string getNodePath( HoudiniScene *scene )
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

void IECoreHoudini::bindHoudiniScene()
{
	IECorePython::RunTimeTypedClass<HoudiniScene>()
		.def( init<>() )
		.def( "__init__", make_constructor( &constructor, default_call_policies(), ( arg( "nodePath" ), arg( "contentPath" ) = list(), arg( "rootPath" ) = list(), arg( "defaultTime" ) = std::numeric_limits<double>::infinity() ) ) )
		.def( "getDefaultTime", &HoudiniScene::getDefaultTime )
		.def( "setDefaultTime", &HoudiniScene::setDefaultTime )
		.def( "_getNodePath", &getNodePath )
	;
}
