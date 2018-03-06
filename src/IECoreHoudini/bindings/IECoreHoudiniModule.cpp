//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "UT/UT_Version.h"

#if UT_MAJOR_VERSION_INT >= 16

#include "RE/RE_Visual.h"

#elif UT_MAJOR_VERSION_INT >= 14

#include "RE/RE_QtVisual.h"

#endif

#include "OP/OP_Director.h"
#include "OP/OP_Node.h"
#include "SOP/SOP_Node.h"
#include "HOM/HOM_Node.h"
#include "HOM/HOM_Geometry.h"
#include "RE/RE_Render.h"
#include "RE/RE_Window.h"
#include "RE/RE_Server.h"

#include "IECore/Object.h"
#include "IECore/Parameterised.h"
#include "IECorePython/PointerFromSWIG.h"

#include "IECoreHoudini/CoreHoudiniVersion.h"
#include "IECoreHoudini/CoreHoudini.h"
#include "IECoreHoudini/bindings/TypeIdBinding.h"
#include "IECoreHoudini/bindings/FnParameterisedHolderBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniGeometryConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniPointsConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniPolygonsConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniGeometryConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniPointsConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniPolygonsConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniCurvesConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniCurvesConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniGroupConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniGroupConverterBinding.h"
#include "IECoreHoudini/bindings/SceneCacheNodeBinding.h"
#include "IECoreHoudini/bindings/LiveSceneBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniCortexObjectConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniCortexObjectConverterBinding.h"
#include "IECoreHoudini/bindings/FromHoudiniCompoundObjectConverterBinding.h"
#include "IECoreHoudini/bindings/ToHoudiniCompoundObjectConverterBinding.h"

using namespace IECoreHoudini;
using namespace boost::python;

// returns a OP_Node from a hou node instance
static void *extractNodeFromHOM( PyObject *o )
{
	if( !PyObject_HasAttrString( o, "this" ) )
	{
		return 0;
	}

	PyObject *thisAttr = PyObject_GetAttrString( o, "this" );
	if( !thisAttr )
	{
		return 0;
	}

	/// \todo: here we 'assume' we have a HOM_Node object, when it really could be anything...
	HOM_Node *homNode = static_cast<HOM_Node*>(((IECorePython::Detail::PySwigObject*)thisAttr)->ptr);

	return OPgetDirector()->findNode( homNode->path().c_str() );
}

// This little function makes the OpenGL context for Houdini's main window
// current. This can be necessary when wanting to create your own OpenGL context
// which shares resources (textures, vertex buffers etc) with Houdini's contexts.
// See GafferUI/GLWidget.py for an example of the hideous abuse this allows.
static void makeMainGLContextCurrent()
{
	RE_Window *window = RE_OGLRender::getMainContext();
	RE_Render *render = window->getRender();

#if defined( IECOREHOUDINI_WITH_QT5 )

	render->makeCurrent();

#else

	render->makeCurrentQt();

#endif

}

#if UT_MAJOR_VERSION_INT >= 14 && UT_MAJOR_VERSION_INT < 16
// This little function returns the address of Houdini's shared QGLWidget
// This can be necessary when wanting to create your own OpenGL context
// which shares resources (textures, vertex buffers etc) with Houdini's contexts.
// See GafferUI/GLWidget.py for an example of the hideous abuse this allows.
static uint64_t sharedGLWidget()
{
	return (uint64_t)RE_QtVisual::getSharedGLWidget();
}

#endif

BOOST_PYTHON_MODULE(_IECoreHoudini)
{
	// setup our global python context
	CoreHoudini::initPython();

	// register a converter for a vector of strings
	typedef std::vector<std::string> StringVector;
    class_<StringVector>("StringVector")
    	.def(boost::python::vector_indexing_suite<StringVector>());

	// bind our IECoreHoudini classes & types
	bindTypeId();
	bindFnParameterisedHolder();
	bindFromHoudiniConverter();
	bindFromHoudiniGeometryConverter();
	bindFromHoudiniPointsConverter();
	bindFromHoudiniPolygonsConverter();
	bindToHoudiniConverter();
	bindToHoudiniGeometryConverter();
	bindToHoudiniPointsConverter();
	bindToHoudiniPolygonsConverter();
	bindFromHoudiniCurvesConverter();
	bindToHoudiniCurvesConverter();
	bindFromHoudiniGroupConverter();
	bindToHoudiniGroupConverter();
	bindSceneCacheNode();
	bindLiveScene();
	bindFromHoudiniCortexObjectConverter();
	bindToHoudiniCortexObjectConverter();
	bindFromHoudiniCompoundObjectConverter();
	bindToHoudiniCompoundObjectConverter();

	// register our node converter functions
	boost::python::converter::registry::insert( &extractNodeFromHOM, boost::python::type_id<OP_Node>() );
	boost::python::converter::registry::insert( &extractNodeFromHOM, boost::python::type_id<SOP_Node>() );

	IECorePython::PointerFromSWIG<HOM_Geometry>();

	def( "makeMainGLContextCurrent", &makeMainGLContextCurrent );

#if UT_MAJOR_VERSION_INT >= 14 && UT_MAJOR_VERSION_INT < 16

	def( "sharedGLWidget", &sharedGLWidget );

#endif

}
