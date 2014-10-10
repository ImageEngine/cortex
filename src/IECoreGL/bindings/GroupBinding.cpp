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

#include <boost/python.hpp>

#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"
#include "IECoreGL/bindings/SceneBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;

namespace IECoreGL
{

Imath::Box3f bound( Group &g )
{
	Group::Mutex::scoped_lock lock( g.mutex() );
	return g.bound();
}

void addChild( Group &g, RenderablePtr child )
{
	Group::Mutex::scoped_lock lock( g.mutex() );
	g.addChild( child );
}

void removeChild( Group &g, Renderable *child )
{
	Group::Mutex::scoped_lock lock( g.mutex() );
	g.removeChild( child );
}

void clearChildren( Group &g )
{
	Group::Mutex::scoped_lock lock( g.mutex() );
	g.clearChildren();
}

list children( const Group &g )
{
	list result;
	Group::Mutex::scoped_lock lock( g.mutex() );
	Group::ChildContainer::const_iterator it;
	for( it=g.children().begin(); it!=g.children().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

void bindGroup()
{
	IECorePython::RunTimeTypedClass<Group>()
		.def( init<>() )
		.def( "setTransform", &Group::setTransform )
		.def( "getTransform", &Group::getTransform, return_value_policy<copy_const_reference>() )
		.def( "setState", &Group::setState )
		.def( "getState", (State *(Group::*)())&Group::getState, return_value_policy<IECorePython::CastToIntrusivePtr>() )
		.def( "addChild", &addChild )
		.def( "removeChild", &removeChild )
		.def( "clearChildren", &clearChildren )
		.def( "bound", &bound )
		.def( "children", &children, "Returns a list referencing the children of the group - modifying the list has no effect on the Group." )
	;
}

}
