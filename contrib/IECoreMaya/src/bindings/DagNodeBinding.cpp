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

#include "maya/MFnDagNode.h"
#include "maya/MDagPath.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/bindings/DagNodeBinding.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

///////////////////////////////////////////////////////////////////////
// DagNode implementation
///////////////////////////////////////////////////////////////////////

DagNode::DagNode( const MObject &object )
	:	Node( object )
{
	MFnDagNode fnNode( object );
	if( !fnNode.hasObj( object ) )
	{
		throw Exception( "Object is not a DAG node." );
	}
}

DagNode::DagNode( const char *name )
	:	Node( name )
{
}

std::string DagNode::fullPathName()
{
	MFnDagNode fnNode( object() );
	return fnNode.fullPathName().asChar();
}

unsigned DagNode::numParents()
{
	MFnDagNode fnNode( object() );
	return fnNode.parentCount();
}

DagNode DagNode::parent( unsigned int index )
{
	
	MStatus s;
	MFnDagNode fnNode( object() );
	MObject p = fnNode.parent( index, &s );
	StatusException::throwIfError( s );
	return DagNode( p );
}
		
///////////////////////////////////////////////////////////////////////
// DagNode binding
///////////////////////////////////////////////////////////////////////

void IECoreMaya::bindDagNode()
{
	class_<DagNode, boost::noncopyable, bases<Node> >( "DagNode", init<const char *>() )
		.def( "fullPathName", &DagNode::fullPathName )
		.def( "__str__", &DagNode::fullPathName )
		.def( "numParents", &DagNode::numParents )
		.def( "parent", (DagNode (DagNode::*)())&DagNode::parent )
		.def( "parent", (DagNode (DagNode::*)( unsigned ))&DagNode::parent )
	;
}
