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

#ifndef IECOREHOUDINI_OBJSCENECACHEGEOMETRY_H
#define IECOREHOUDINI_OBJSCENECACHEGEOMETRY_H

#include "OBJ/OBJ_Geometry.h"

#include "IECoreHoudini/OBJ_SceneCacheNode.h"

namespace IECoreHoudini
{

/// OBJ for loading a single transform and leaf Objects from an IECore::SceneCache
class OBJ_SceneCacheGeometry : public OBJ_SceneCacheNode<OBJ_Geometry>
{
	public :

		OBJ_SceneCacheGeometry( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~OBJ_SceneCacheGeometry();

		static const char *typeName;

		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );
		static OP_TemplatePair *buildParameters();

		virtual bool runCreateScript();

		/// Implemented to expand the SceneCache using a SOP_SceneCacheSource. Derived classes
		/// should re-implement doExpandGeometry() if specialized behaviour is necessary.
		/// \todo: do we need this extra abstraction?
		virtual void expandHierarchy( const IECoreScene::SceneInterface *scene );
		/// Implemented to push the GeometryType and attribute filter values to the sop below.
		virtual void pushToHierarchy();

	protected :

		/// Called by expandHierarchy() to load the SceneCache. The Space parameter will
		/// determine what settings are used. World and Path will load all descedants,
		/// while Local and Object will load the immediate child object only.
		virtual void doExpandGeometry( const IECoreScene::SceneInterface *scene );

		virtual int *getIndirect() const;

	private :

		static int *g_indirection;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_OBJSCENECACHEGEOMETRY_H
