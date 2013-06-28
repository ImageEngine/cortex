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

#ifndef IECOREHOUDINI_OBJSCENECACHETRANSFORM_H
#define IECOREHOUDINI_OBJSCENECACHETRANSFORM_H

#include "OBJ/OBJ_SubNet.h"

#include "IECore/LinkedScene.h"

#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/OBJ_SceneCacheNode.h"

namespace IECoreHoudini
{

/// OBJ for loading a transform or expanding a hierarchy from an IECore::SceneCache
class OBJ_SceneCacheTransform : public OBJ_SceneCacheNode<OBJ_SubNet>
{
	public :
		
		OBJ_SceneCacheTransform( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~OBJ_SceneCacheTransform();
		
		static const char *typeName;
		
		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );
		static OP_TemplatePair *buildParameters();
		
		static PRM_Name pHierarchy;
		static PRM_Name pDepth;
		static PRM_Name pTagFilter;
		
		static PRM_Default hierarchyDefault;
		static PRM_Default depthDefault;
		static PRM_Default filterDefault;
		
		static PRM_ChoiceList hierarchyList;
		static PRM_ChoiceList depthList;
		static PRM_ChoiceList tagFilterMenu;
		
		static void buildTagFilterMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * );
		
		enum Hierarchy
		{
			SubNetworks,
			Parenting,
			FlatGeometry
		};
		
		enum Depth
		{
			AllDescendants,
			Children
		};
		
		/// Implemented to expand the SceneCache using a combination of OBJ_SceneCacheTransform
		/// and/or OBJ_SceneCacheGeometry nodes depending on the settings for pHierarchy and pDepth.
		/// Derived classes should re-implement doExpandObject() and doExpandChild() if specialized
		/// behaviour is necessary.
		virtual void expandHierarchy( const IECore::SceneInterface *scene );
	
	protected :
		
		/// Called by expandHierarchy() and doExpandChildren() when the SceneCache contains an object.
		/// Implemented to expand the specific object using an OBJ_SceneCacheGeometry node.
		virtual OBJ_Node *doExpandObject( const IECore::SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth, const UT_StringMMPattern &tagFilter );
		
		/// Called by doExpandChildren() when the SceneCache contains a child.
		/// Implemented to expand the current cache path using an OBJ_SceneCacheTransform or
		/// OBJ_SceneCacheGeometry node depending on the settings for hierarchy and depth.
		virtual OBJ_Node *doExpandChild( const IECore::SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth, const UT_StringMMPattern &tagFilter );
		
		/// Called by expandHierarchy() to expand the children of the SceneCache.
		/// This will be called recursively for each child when Depth is AllDescenants.
		virtual void doExpandChildren( const IECore::SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth, const UT_StringMMPattern &tagFilter );
		
		static OP_TemplatePair *buildExtraParameters();
	
	private :
		
		bool tagged( const IECore::SceneInterface *scene, const UT_StringMMPattern &filter );
		
		/// functions registered in HoudiniScene as custom attributes
		struct HoudiniSceneAddOn
		{
			HoudiniSceneAddOn();
		};
		static HoudiniSceneAddOn g_houdiniSceneAddOn;
		
		static bool hasLink( const OP_Node *node );
		static IECore::ObjectPtr readLink( const OP_Node *node );
		static bool hasTag( const OP_Node *node, const IECore::SceneInterface::Name &tag );
		static void readTags( const OP_Node *node, IECore::SceneInterface::NameList &tags, bool includeChildren );

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_OBJSCENECACHETRANSFORM_H
