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

#include "IECore/CompoundParameter.h"
#include "IECoreScene/SceneInterface.h"
#include "IECoreMaya/Export.h"

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

/// A base class for a maya shape that can read an IECore::SceneInterface.
/// getSceneInterface should be implemented by any derived class
/// Builds a glScene for preview, which can draw all geometry in the hierarchy and the child bounds
/// Can be used as objectOnly, in which case the glScene will only represent the current path of the sceneInterface
/// or not objectOnly in which case the entire hierarchy starting from the current path is represented.
/// Computes queries on paths to get transforms, bounds and objects as outputs, as well as attributes.
/// The query paths are relative to the current sceneInterface path.
/// Queries can be accessed in local space or world space (starting from the current path).
///
class IECOREMAYA_API SceneShapeInterface: public MPxComponentShape
{
	friend class SceneShapeInterfaceComponentBoundIterator;

	public:

		SceneShapeInterface();
		virtual ~SceneShapeInterface();

		/*
		 * For Maya
		 */

		virtual void postConstructor();
		static void *creator();
		static MStatus initialize();

		virtual bool isBounded() const;
		virtual MBoundingBox boundingBox() const;
		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );
		virtual MStatus compute( const MPlug &plug, MDataBlock &dataBlock );

		MatchResult matchComponent( const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list ) override;
		MSelectionMask getShapeSelectionMask() const override;
		MSelectionMask getComponentSelectionMask() const override;

#if MAYA_API_VERSION >= 201600

		virtual MStatus preEvaluation( const MDGContext &context, const MEvaluationNode &evaluationNode );

#endif

		/// This method is overridden to supply a geometry iterator, which maya uses to work out
		/// the bounding boxes of the components you've selected in the viewport
		virtual MPxGeometryIterator* geometryIteratorSetup( MObjectArray&, MObject&, bool );

		/// This is a blank override, to stop maya offering you a rotation manipulator for the
		/// procedural components, then crashing when you try and use it (maya 2013)
		virtual void transformUsing( const MMatrix &mat, const MObjectArray &componentList, MPxSurfaceShape::MVertexCachingMode cachingMode, MPointArray *pointCache );

		static MTypeId id;

		// Public variables because plugs need to be accessed by the UI creator which implements the drawing/selection
		static MObject aObjectOnly;
		static MObject aDrawGeometry;
		static MObject aDrawRootBound;
		static MObject aDrawChildBounds;
		static MObject aDrawTagsFilter;

		/*
		 * Custom
		 */

		/// Returns the sceneInterface for this node. Needs to be implemented by derived classes.
		virtual IECoreScene::ConstSceneInterfacePtr getSceneInterface();

		/// Returns the GL Scene representing the sceneInterface for the preview plug values ( objectOnly, drawGeometry, drawLocators, drawChildBounds )
		IECoreGL::ConstScenePtr glScene();

		/// Returns GL Group matching the given path name.
		IECoreGL::GroupPtr glGroup( const IECore::InternedString &name );
		/// Returns the internal index stored for the given path
		int selectionIndex( const IECore::InternedString &name );
		/// Returns the path name for the given index
		IECore::InternedString selectionName( int index );
		/// Returns all component names currently existing in the shape
		const std::vector< IECore::InternedString > & componentNames() const;
		/// Return the value of the time plug for the SceneShape.
		double time() const;
		/// Build data-structure to map from group names to component indices. If
		/// rendering is done through a different mechanism than glScene(), for
		/// example by using VP2, this needs to be called whenever the scene updates
		/// to keep the map in sync. Return value indicates success.
		bool buildComponentIndexMap();

	protected :

		// protected variables, used by derived classes to set attribute dependencies
		static MObject aTime;
		static MObject aOutTime;
		static MObject aOutputObjects;
		static MObject aObjectDependency;
		static MObject aAttributes;
		static MObject aTransform;
		static MObject aBound;

		/// Flags the GL scene as dirty, for use by derived classes
		void setDirty();

	private :

		static MObject aQuerySpace;
		static MObject aSceneQueries;
		static MObject aAttributeQueries;
		/// Read convert parameters.
		/**
		Some ToMaya*Converters have parameters to be used when converting objects.
		In case of ToMayaCurveConverter, it takes an int parameter "index" that controls which one of curves it should convert to Maya nurbs curve.
		queryConvertParameters lets you specify some of these paramters.
		Converter parameters are updated with a string value held in aConvertParamQueries with IECore.ParameterParser.
		**/
		static MObject aConvertParamQueries;

		static MObject aAttributeValues;

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

		/// Available modes for querySpace: Local space or World space (starts at sceneInterface path)
		enum Space
		{
			World,
			Local
		};

		bool m_sceneInterfaceDirty;
		bool m_previewSceneDirty;

		IECoreGL::ScenePtr m_scene;

		/// Uses the sceneInterface hierarchy to build a GL Scene matching the preview plug values
		void buildScene( IECoreGL::RendererPtr renderer, IECoreScene::ConstSceneInterfacePtr subSceneInterface );

		/// Recursively parses the sceneInterface hierarchy to build a GL Scene matching the preview plug values
		void recurseBuildScene( IECoreGL::Renderer * renderer, const IECoreScene::SceneInterface *subSceneInterface, double time, bool drawBounds, bool drawGeometry, bool objectOnly, const IECoreScene::SceneInterface::NameList &drawTags );

		void createInstances();

		/// Recursively parses glScene to store GL Groups matching path names
		void buildGroups( IECoreGL::ConstNameStateComponentPtr nameState, IECoreGL::GroupPtr subScene );

		void registerGroup( const std::string &name, IECoreGL::GroupPtr &group );

		std::string relativePathName( IECoreScene::SceneInterface::Path path );
		IECoreScene::SceneInterface::Path fullPathName( std::string relativeName );
		/// Returns concatenated matrix from current sceneInterface path to given scene
		Imath::M44d worldTransform( IECoreScene::ConstSceneInterfacePtr scene, double time );
		/// Returns bound for the component matching the given index
		Imath::Box3d componentBound( int idx );
		bool animatedScene();

		void recurseCopyGroup( const IECoreGL::Group *srcGroup, IECoreGL::Group *trgGroup, const std::string &namePrefix );

		bool readConvertParam( IECore::CompoundParameterPtr parameters, int attrIndex ) const;

		MStatus computeOutputPlug( const MPlug &plug, const MPlug &topLevelPlug, MDataBlock &dataBlock, const IECoreScene::SceneInterface *scene, int topLevelIndex, int querySpace, MTime &time );

		typedef std::map< IECore::InternedString,  std::pair< unsigned int, IECoreGL::GroupPtr> > NameToGroupMap;
		typedef std::vector< IECore::InternedString > IndexToNameMap;
		typedef std::map< IECore::MurmurHash, IECore::InternedString > HashToName;
		typedef std::pair< IECore::InternedString, IECore::InternedString > InstanceInfo;
		typedef std::vector< InstanceInfo > InstanceArray;

		IndexToNameMap m_indexToNameMap;
		NameToGroupMap m_nameToGroupMap;
		HashToName m_hashToName;
		InstanceArray m_instances;

		IE_CORE_FORWARDDECLARE( PostLoadCallback );
		PostLoadCallbackPtr m_postLoadCallback;

};

}

#endif // IE_COREMAYA_SCENESHAPEINTERFACE_H

