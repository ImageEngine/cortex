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

#include "OP/OP_Node.h"

#include "IECore/MessageHandler.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreHoudini/SceneCacheNode.h"
#include "IECoreHoudini/OBJ_SceneCacheTransform.h"
#include "IECoreHoudini/NodeHandle.h"

#include "IECoreHoudini/bindings/SceneCacheNodeBinding.h"

using namespace boost::python;
using namespace IECoreHoudini;

class SceneCacheNodeHelper
{
	public :

		SceneCacheNodeHelper( OP_Node *node = 0 )
		{
			if ( !node )
			{
				return;
			}

			if ( sceneNode( node ) )
			{
				m_handle = node;
			}
			else
			{
				UT_String path;
				node->getFullPath( path );
				IECore::msg( IECore::MessageHandler::Error, "SceneCacheNode", path.toStdString() + " was not a valid SceneCacheNode" );
			}
		}

		~SceneCacheNodeHelper()
		{
		}

		bool hasNode() const
		{
			return m_handle.alive();
		}

		SceneCacheNode<OP_Node> *sceneNode( OP_Node *node ) const
		{
			// make sure its a SceneCacheNode
			if ( !node || !node->hasParm( SceneCacheNode<OP_Node>::pFile.getToken() ) || !node->hasParm( SceneCacheNode<OP_Node>::pRoot.getToken() ) )
			{
				return 0;
			}

			return reinterpret_cast<SceneCacheNode<OP_Node>* >( node );
		}

		IECore::SceneInterfacePtr scene() const
		{
			if ( !hasNode() )
			{
				return 0;
			}

			if ( SceneCacheNode<OP_Node> *node = sceneNode( m_handle.node() ) )
			{
				if ( IECore::ConstSceneInterfacePtr s = node->scene() )
				{
					return const_cast<IECore::SceneInterface*>( s.get() );
				}
			}

			return 0;
		}

	private :

		NodeHandle m_handle;

};

void IECoreHoudini::bindSceneCacheNode()
{
	scope modeCacheNodeScope = class_<SceneCacheNodeHelper>( "SceneCacheNode" )
		.def( init<OP_Node*>() )
		.def( "scene", &SceneCacheNodeHelper::scene )
	;

	enum_<SceneCacheNode<OP_Node>::Space>( "Space" )
		.value( "World", SceneCacheNode<OP_Node>::World )
		.value( "Path", SceneCacheNode<OP_Node>::Path )
		.value( "Local", SceneCacheNode<OP_Node>::Local )
		.value( "Object", SceneCacheNode<OP_Node>::Object )
	;

	enum_<SceneCacheNode<OP_Node>::GeometryType>( "GeometryType" )
		.value( "Cortex", SceneCacheNode<OP_Node>::Cortex )
		.value( "Houdini", SceneCacheNode<OP_Node>::Houdini )
		.value( "BoundingBox", SceneCacheNode<OP_Node>::BoundingBox )
		.value( "PointCloud", SceneCacheNode<OP_Node>::PointCloud )
	;

	enum_<OBJ_SceneCacheTransform::Hierarchy>( "Hierarchy" )
		.value( "SubNetworks", OBJ_SceneCacheTransform::SubNetworks )
		.value( "Parenting", OBJ_SceneCacheTransform::Parenting )
		.value( "FlatGeometry", OBJ_SceneCacheTransform::FlatGeometry )
	;

	enum_<OBJ_SceneCacheTransform::Depth>( "Depth" )
		.value( "AllDescendants", OBJ_SceneCacheTransform::AllDescendants )
		.value( "Children", OBJ_SceneCacheTransform::Children )
	;

}
