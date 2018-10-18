//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
#ifndef IE_COREMAYA_SCENESHAPESUBSCENEOVERRIDE_H
#define IE_COREMAYA_SCENESHAPESUBSCENEOVERRIDE_H

#include "IECoreMaya/Export.h"
#include "IECoreMaya/SceneShape.h"

#include "IECoreScene/SceneInterface.h"

#include "IECore/InternedString.h"

#include "maya/MPxSubSceneOverride.h"

#include "boost/signals2.hpp"
#include "boost/variant.hpp"

#include <bitset>
#include <map>

namespace IECoreMaya
{

struct GeometryData
{
public :
	GeometryData()
	{
	}

	~GeometryData()
	{
	}

	IECore::ConstV3fVectorDataPtr positionData;
	IECore::ConstV3fVectorDataPtr normalData;
	IECore::ConstV2fVectorDataPtr uvData;

	IECore::ConstIntVectorDataPtr indexData;
	IECore::ConstIntVectorDataPtr wireframeIndexData;
};

using GeometryDataPtr = std::shared_ptr<GeometryData>;

using VertexBufferPtr = std::shared_ptr<MHWRender::MVertexBuffer>;
using IndexBufferPtr = std::shared_ptr<MHWRender::MIndexBuffer>;

using Buffer = boost::variant<VertexBufferPtr, IndexBufferPtr>;
using BufferPtr = std::shared_ptr<Buffer>;

class IECOREMAYA_API SceneShapeSubSceneOverride : public MHWRender::MPxSubSceneOverride
{
	public :

		static MString& drawDbClassification();
		static MString& drawDbId();
		static MHWRender::MPxSubSceneOverride* Creator( const MObject& obj );

		~SceneShapeSubSceneOverride() override;

		// Maya calls this to determine if `update` needs to be called at all for this refresh. Gets called a lot.
		bool requiresUpdate(const MHWRender::MSubSceneContainer& container, const MHWRender::MFrameContext& frameContext) const override;

		// Performing the actual updating. Needs to fill given container with MRenderItem objects for drawing.
		void update(MHWRender::MSubSceneContainer&  container, const MHWRender::MFrameContext& frameContext) override;

		// We are responsible for drawing all instances. Maya therefore refers to
		// us for figuring out which instance was selected when the user clicks
		// on one of our MRenderItems.
		#if MAYA_API_VERSION > 201650
		bool getInstancedSelectionPath( const MHWRender::MRenderItem &renderItem, const MHWRender::MIntersection &intersection, MDagPath &dagPath ) const override;
		#endif

		// Maya allows us to switch between object and component selection by changing the MSelectionContext.
		void updateSelectionGranularity( const MDagPath &path, MHWRender::MSelectionContext &selectionContext ) override;

		MHWRender::DrawAPI supportedDrawAPIs() const override;

	protected :

		SceneShapeSubSceneOverride( const MObject& obj );

	private :

		class ComponentConverter;

		using IndexMap = std::map<const std::string, std::set<int> >;
		using RenderItemMap = std::map<const std::string, std::pair<MHWRender::MRenderItem*, MMatrixArray> >;
		class RenderItemUserData;
		using RenderItemUserDataPtr = std::shared_ptr<RenderItemUserData>;
		using StyleMask = std::bitset<3>;

		struct Instance
		{
			Instance( Imath::M44d transformation, bool selected, bool componentMode, MDagPath path, bool visible )
				: transformation( transformation ), selected( selected ), componentMode( componentMode ), path( path ), visible( visible )
			{
			}

			bool operator==( const Instance &rhs ) const
			{
				return transformation == rhs.transformation && selected == rhs.selected && path == rhs.path && componentMode == rhs.componentMode && visible == rhs.visible;
			}

			Imath::M44d transformation;
			bool selected;
			bool componentMode;
			MDagPath path;
			bool visible;
		};

		using Instances = std::vector<Instance>;

		// Traverse the scene and create MRenderItems as necessary while collecting all matrices to be associated with them.
		void visitSceneLocations( const IECoreScene::SceneInterface *sceneInterface, RenderItemMap &renderItems, MHWRender::MSubSceneContainer &container, const Imath::M44d &matrix, bool isRoot = false );

		// Provide information about the instances that need drawing as
		// SubSceneOverrides are responsible for drawing all instances of the
		// shape, which is different to how things were handled in Maya's VP1.
		void collectInstances( Instances &instances ) const;

		// Retrieve global display settings (can be locally overridden by instances)
		void checkDisplayOverrides( unsigned int displayStyle, StyleMask &mask ) const;

		RenderItemUserDataPtr acquireUserData( int componentIndex );
		void selectedComponentIndices( IndexMap &indexMap ) const;
		void setBuffersForRenderItem( GeometryDataPtr geometryData, MHWRender::MRenderItem *renderItem, bool useWireframeIndex, const MBoundingBox &boundingBox );

		void bufferEvictedCallback( const BufferPtr buffer ); // \todo

		SceneShape *m_sceneShape;

		std::string m_drawTagsFilter;
		double m_time;

		StyleMask m_styleMask; // \todo: now that things are simpler, consider replacing this with three bools.
		Instances m_instances;

		bool m_drawRootBounds;
		bool m_drawChildBounds;
		MPlug m_shaderOutPlug;
		bool m_instancedRendering;
		IECoreScene::ConstSceneInterfacePtr m_sceneInterface;
		bool m_geometryVisible;

		std::map<const std::string, MDagPath> m_renderItemNameToDagPath;
		IndexMap m_selectedComponents;
		std::map<int, RenderItemUserDataPtr> m_userDataMap;
		std::vector<BufferPtr> m_markedForDeletion;
		using RenderItemNameSet = std::set<IECore::InternedString>;
		std::map<Buffer*, RenderItemNameSet> m_bufferToRenderItems;
		std::set<MHWRender::MRenderItem*> m_renderItemsToEnable;

		struct AllShaders;
		using AllShadersPtr = std::shared_ptr<AllShaders>;

		AllShadersPtr m_allShaders;
		boost::signals2::scoped_connection m_evictionConnection;

};

} // namespace IECoreMaya

#endif // IE_COREMAYA_SCENESHAPESUBSCENEOVERRIDE_H
