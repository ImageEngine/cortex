//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECoreScene/Group.h"
#include "IECoreScene/Renderer.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

#include "GroupBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

/// \todo: this should return a tuple rather than a list
static list children( Group &g )
{
	list result;
	for( Group::ChildContainer::const_iterator it=g.children().begin(); it!=g.children().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

/// \todo: this should return a tuple rather than a list
static list state( Group &g )
{
	list result;
	for( Group::StateContainer::const_iterator it=g.state().begin(); it!=g.state().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

static void render( const Group &group, Renderer *renderer )
{
	IECorePython::ScopedGILRelease gilRelease;
	group.render( renderer );
}

static void render2( const Group &group, Renderer *renderer, bool inAttributeBlock )
{
	IECorePython::ScopedGILRelease gilRelease;
	group.render( renderer, inAttributeBlock );
}

static void renderState( const Group &group, Renderer *renderer )
{
	IECorePython::ScopedGILRelease gilRelease;
	group.renderState( renderer );
}

static void renderChildren( const Group &group, Renderer *renderer )
{
	IECorePython::ScopedGILRelease gilRelease;
	group.renderChildren( renderer );
}

static DataPtr getAttribute( Group &g, const std::string &name )
{
	ConstDataPtr d = g.getAttribute( name );
	if( d )
	{
		return d->copy();
	}
	return nullptr;
}



BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( transformMatrixOverloads, transformMatrix, 0, 1 );
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( globalTransformMatrixOverloads, globalTransformMatrix, 0, 1 );

void bindGroup()
{
	RunTimeTypedClass<Group>()
		.def( init<>() )
		.def( "children", &children, "Returns all the children in a list - note that modifying the list will not add or remove children." )
		.def( "addChild", &Group::addChild )
		.def( "removeChild", &Group::removeChild )
		.def( "clearChildren", &Group::clearChildren )
		.def( "state", &state, "Returns all the state in a list - note that modifying the list will not add or remove state." )
		.def( "addState", &Group::addState )
		.def( "removeState", &Group::removeState )
		.def( "clearState", &Group::clearState )
		.def( "setAttribute", &Group::setAttribute )
		.def( "getAttribute", &getAttribute, "Returns a copy of the internal attribute data." )
		.def( "getTransform", (Transform *(Group::*)())&Group::getTransform, return_value_policy<CastToIntrusivePtr>() )
		.def( "setTransform", &Group::setTransform )
		.def( "transformMatrix", &Group::transformMatrix, transformMatrixOverloads() )
		.def( "globalTransformMatrix", &Group::globalTransformMatrix, globalTransformMatrixOverloads() )
		.def( "parent", (Group *(Group::*)())&Group::parent, return_value_policy<CastToIntrusivePtr>() )
		.def( "render", &render )
		.def( "render", &render2 )
		.def( "renderState", &renderState )
		.def( "renderChildren", &renderChildren )
	;
}

}
