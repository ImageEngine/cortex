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

#ifndef IE_COREMAYA_SCENESHAPEINTERFACE_H
#define IE_COREMAYA_SCENESHAPEINTERFACE_H

#include "IECore/SceneInterface.h"

#include <map>
#include "OpenEXR/ImathMatrix.h"
#include "IECoreGL/IECoreGL.h"
#include "maya/MPxComponentShape.h"


namespace IECoreGL
{
IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( BoxPrimitive );
IE_CORE_FORWARDDECLARE( Primitive );
IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( NameStateComponent );
IE_CORE_FORWARDDECLARE( Renderer );
}

namespace IECoreMaya
{

class SceneShapeInterface: public MPxComponentShape
{
	public:
		
		SceneShapeInterface();
		virtual ~SceneShapeInterface();
		
		virtual void postConstructor();
		
		static void *creator();
		static MStatus initialize();
		static MTypeId id;
		
		virtual bool isBounded() const;
		virtual MBoundingBox boundingBox() const;
		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );
		virtual MStatus compute( const MPlug &plug, MDataBlock &dataBlock );
		
		virtual void componentToPlugs( MObject &component, MSelectionList &selectionList ) const;
		virtual MatchResult matchComponent( const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list );

		IECoreGL::ConstScenePtr scene();
		void setDirty();
		
		void buildScene( IECoreGL::RendererPtr renderer, IECore::ConstSceneInterfacePtr subSceneInterface );

		virtual IECore::SceneInterfacePtr getSceneInterface();
		virtual IECore::SceneInterface::Path getSceneRoot();
		
		void getOutputPlugsArray( MPlugArray &plugArray );
		
		IECoreGL::GroupPtr getGroup( std::string name );
		int getIndex( std::string name );
		std::string getName( int index );
		
		static MObject aDrawGeometry;
		static MObject aDrawRootBound;
		static MObject aDrawChildBounds;
		static MObject aTime;
		static MObject aPreviewSpace;
		static MObject aChildrenNames;
		static MObject aQuerySpace;
		static MObject aSceneQueries;
		// todo: attributeQueries
		static MObject aOutputObjects;
		
		static MObject aTransform;
		static MObject aTranslate;
		static MObject aTranslateX;
		static MObject aTranslateY;
		static MObject aTranslateZ;
		static MObject aRotate;
		static MObject aRotateX;
		static MObject aRotateY;
		static MObject aRotateZ;
		static MObject aScale;
		static MObject aScaleX;
		static MObject aScaleY;
		static MObject aScaleZ;
		
		static MObject aBound;
		static MObject aBoundMin;
		static MObject aBoundMinX;
		static MObject aBoundMinY;
		static MObject aBoundMinZ;
		static MObject aBoundMax;
		static MObject aBoundMaxX;
		static MObject aBoundMaxY;
		static MObject aBoundMaxZ;
		static MObject aBoundCenter;
		static MObject aBoundCenterX;
		static MObject aBoundCenterY;
		static MObject aBoundCenterZ;
	
	private :
		
		enum Space
		{
			World,
			Path,
			Local,
			Object
		};

		bool m_sceneInterfaceDirty;
		bool m_previewSceneDirty;

		IECoreGL::ScenePtr m_scene;
		IECore::SceneInterface::Path m_sceneRoot;
		
		void buildSceneMaps();
		void buildSceneMaps( IECore::ConstSceneInterfacePtr subSceneInterface );
		
		void buildGroups( IECoreGL::ConstNameStateComponentPtr nameState, IECoreGL::GroupPtr subScene );
		
		std::string getRelativePathName( IECore::SceneInterface::Path path );
		Imath::M44d worldTransform( IECore::ConstSceneInterfacePtr scene, IECore::SceneInterface::Path root, double time );

		typedef std::map< IECore::InternedString,  std::pair< unsigned int, IECore::ConstSceneInterfacePtr> > NameToIndexSceneMap;
		typedef std::vector< IECore::InternedString > IndexToNameMap;
		typedef std::map< IECore::InternedString, IECoreGL::GroupPtr > NameToGroupMap;
		
		NameToIndexSceneMap m_nameToIndexSceneMap;
		IndexToNameMap m_indexToNameMap;
		NameToGroupMap m_nameToGroupMap;
};

}

#endif // IE_COREMAYA_SCENESHAPEINTERFACE_H

