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

#include "boost/python.hpp"

#include "maya/MFnDependencyNode.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/bindings/NodeBinding.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

///////////////////////////////////////////////////////////////////////
// Node implementation
///////////////////////////////////////////////////////////////////////

Node::Node( const MObject &object )
	:	MObjectWrapper( object )
{
	MFnDependencyNode fnNode( object );
	if( !fnNode.hasObj( object ) )
	{
		throw Exception( "Object is not a dependency node." );
	}
}

Node::Node( const char *name )
	:	MObjectWrapper( name )
{
}

std::string Node::name()
{
	MFnDependencyNode fnNode( object() );
	return fnNode.name().asChar();
}

std::string Node::typeName()
{
	MFnDependencyNode fnNode( object() );
	return fnNode.typeName().asChar();
}

Plug *Node::plug( const char *name )
{
	MFnDependencyNode fnNode( object() );
	MStatus s;
	MPlug p = fnNode.findPlug( name, false, &s );
	return new Plug( p );
}
		
///////////////////////////////////////////////////////////////////////
// Node binding
///////////////////////////////////////////////////////////////////////

void IECoreMaya::bindNode()
{
	class_<Node, boost::noncopyable, bases<MObjectWrapper> >( "Node", init<const char *>() )
		.def( "name", &Node::name )
		.def( "__str__", &Node::name )
		.def( "typeName", &Node::typeName )
		.def( "plug", &Node::plug, return_value_policy<manage_new_object>() )
	;
}
