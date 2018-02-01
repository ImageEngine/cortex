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

#ifndef IECOREMAYA_SCENESHAPEUI_H
#define IECOREMAYA_SCENESHAPEUI_H

#include "maya/MPxSurfaceShapeUI.h"
#include "maya/MTypes.h"

#include "IECore/Object.h"

#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/DisplayStyle.h"

namespace IECoreGL
{
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( StateComponent );
}


namespace IECoreMaya
{

class IECOREMAYA_API SceneShapeUI : public MPxSurfaceShapeUI
{

	public :

		SceneShapeUI();
		virtual ~SceneShapeUI();

		virtual void getDrawRequests( const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests );
		virtual void draw( const MDrawRequest &request, M3dView &view ) const;
		virtual bool select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const;

		/// If the maya version is greater or equal to 2013 then add support for snapping geometry to the SceneShape.
		#if MAYA_API_VERSION >= 201300
		virtual bool snap( MSelectInfo &snapInfo ) const;
		#endif

		static void *creator();

	private :

		typedef std::map< IECoreGL::GroupPtr, IECoreGL::StatePtr > StateMap;

		void hiliteGroups( IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr hilite, IECoreGL::StateComponentPtr base ) const;
		void unhiliteGroupChildren( const std::string &name, IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr base ) const;
		void resetHilites() const;

		/// A useful method that calculates the world space position of the selection ray when given a camera and depth. The result is returned in worldIntersectionPoint.
		void selectionRayToWorldSpacePoint( const MDagPath &camera, const MSelectInfo &selectInfo, float depth, MPoint &worldIntersectionPoint ) const;

		/// Returns the concatenated object transforms of the SceneInterface.
		Imath::M44d worldTransform( const IECoreScene::SceneInterface *scene, double time ) const;

		mutable StateMap m_stateMap;
		mutable DisplayStyle m_displayStyle;

		enum DrawMode
		{
			SceneDrawMode,
			BoundDrawMode,
		};

		static void setWireFrameColors( MDrawRequest &request, M3dView::DisplayStatus status );

		struct LightingState {
			unsigned int numMayaLights;
			unsigned int numGlLights;
			std::vector<Imath::Color4f> diffuses;
			std::vector<Imath::Color4f> specs;
			std::vector<Imath::Color4f> ambients;
		};

		bool cleanupLights( const MDrawRequest &request, M3dView &view, LightingState *s ) const;
		void restoreLights( LightingState *s ) const;
};

} // namespace IECoreMaya

#endif // IECOREMAYA_SCENESHAPEUI_H
