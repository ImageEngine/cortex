//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "OP/OP_Director.h"
#include "boost/shared_ptr.hpp"

#include "IECoreHoudini/NodeHandle.h"

using namespace IECoreHoudini;

NodeHandle::NodeHandle()
{
}

NodeHandle::NodeHandle( const OP_Node *node )
{
	UT_String path = "";
	node->getFullPath( path );
	m_homNode = boost::shared_ptr<HOM_Node>( dynamic_cast<HOM_Node*>( HOM().node( path ) ) );
}

NodeHandle::~NodeHandle()
{
}

bool NodeHandle::alive() const
{
	return (bool)node();
}

OP_Node *NodeHandle::node() const
{
	if ( !m_homNode )
	{
		return 0;
	}

	// get the hom path and use opdirector to get a regular OP_Node* to our node
	try
	{
		OP_Node *node = OPgetDirector()->findNode( m_homNode->path().c_str() );
		if ( node )
		{
			return node;
		}
	}
	catch( HOM_ObjectWasDeleted )
	{
	}

	return 0;
}
