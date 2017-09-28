//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_SCENESHAPE_H
#define IE_COREMAYA_SCENESHAPE_H

#include "IECore/SceneInterface.h"
#include "IECoreMaya/SceneShapeInterface.h"

namespace IECore
{
IE_CORE_FORWARDDECLARE( Object );
}

namespace IECoreMaya
{

/// A shape derived from a SceneShapeInterface which implements
/// a shape which can read an IECore::SceneInterface using a file (.scc) and a root path
/// It also registers itself in the LiveScene class so that the node is seen as a link to
/// an external file through the LinkedScene mechanism.
class SceneShape : public SceneShapeInterface
{
	public :

		SceneShape();
		virtual ~SceneShape();

		/*
		 * For Maya
		 */

		virtual void postConstructor();
		static void *creator();
		static MStatus initialize();
		MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );

		static MTypeId id;

		/*
		 * Custom
		 */

		virtual IECore::ConstSceneInterfacePtr getSceneInterface();

	private :

		static MObject aSceneFilePlug;
		static MObject aSceneRootPlug;

		bool m_sceneDirty;
		IECore::ConstSceneInterfacePtr m_scene;


		static SceneShape *findScene( const MDagPath &p, bool noIntermediate, MDagPath *dagPath = 0 );

		/// functions registered in LiveScene as custom object and custom attributes
		struct LiveSceneAddOn
		{
			LiveSceneAddOn();
		};
		static LiveSceneAddOn g_liveSceneAddon;

		static bool hasSceneShapeLink( const MDagPath &p );
		static IECore::ConstObjectPtr readSceneShapeLink( const MDagPath &p );
		static bool hasSceneShapeObject( const MDagPath &p );
		static IECore::ConstObjectPtr readSceneShapeObject( const MDagPath &p );
		static void sceneShapeAttributeNames( const MDagPath &p, IECore::SceneInterface::NameList &attributeNames );
		static IECore::ConstObjectPtr readSceneShapeAttribute( const MDagPath &p, IECore::SceneInterface::Name attributeName );
		static bool hasTag( const MDagPath &p, const IECore::SceneInterface::Name &tag, int filter );
		static void readTags( const MDagPath &p, IECore::SceneInterface::NameList &tags, int filter );

};

}

#endif // IE_COREMAYA_SCENESHAPE_H
