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

#ifndef IECOREHOUDINI_OBJSCENECACHENODE_H
#define IECOREHOUDINI_OBJSCENECACHENODE_H

#include "OBJ/OBJ_Node.h"

#include "IECore/SceneCache.h"

#include "IECoreHoudini/SceneCacheNode.h"

namespace IECoreHoudini
{

/// Abstract base class for all OBJ SceneCacheNodes.
/// See OBJ_SceneCacheGeometry or OBJ_SceneCacheTransform for specific implementations.
template<typename BaseType>
class OBJ_SceneCacheNode : public SceneCacheNode<BaseType>
{
	public :
		
		OBJ_SceneCacheNode( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~OBJ_SceneCacheNode();
		
		static PRM_Template *buildParameters( OP_TemplatePair *extraParameters = 0 );
		
		static PRM_Name pMainSwitcher;
		static PRM_Name pExpand;
		static PRM_Name pPush;
		static PRM_Name pCollapse;
		static PRM_Name pExpanded;
		static PRM_Name pOverrideTransform;
		static PRM_Name pOutTranslate;
		static PRM_Name pOutRotate;
		static PRM_Name pOutScale;
		
		static int expandButtonCallback( void *data, int index, float time, const PRM_Template *tplate );
		static int pushButtonCallback( void *data, int index, float time, const PRM_Template *tplate );
		static int collapseButtonCallback( void *data, int index, float time, const PRM_Template *tplate );
		
		/// Derived classes should define this function to expand the hierarchy contained in the SceneCache.
		virtual void expandHierarchy( const IECore::SceneInterface *scene ) = 0;
		// Derived classes should define this function to update the hierarchy based on relevant parameter values.
		virtual void pushToHierarchy() = 0;
		/// Implemented to destroy all child nodes
		virtual void collapseHierarchy();
	
	protected :
		
		virtual void sceneChanged();
		
		virtual OP_ERROR cookMyObj( OP_Context &context );
		virtual bool getParmTransform( OP_Context &context, UT_DMatrix4 &xform );
		virtual bool updateParmsFlags();
		void updateState();
		
		UT_Matrix4D m_xform;
	
	private :
		
		static OP_TemplatePair *buildExpansionParameters();
		static OP_TemplatePair *buildOutputParameters();

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_OBJSCENECACHENODE_H
