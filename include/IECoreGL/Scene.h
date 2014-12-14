//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SCENE_H
#define IECOREGL_SCENE_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Renderable.h"
#include "IECoreGL/HitRecord.h"
#include "IECoreGL/Selector.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( Camera );

class IECOREGL_API Scene : public Renderable
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Scene, SceneTypeId, Renderable );

		Scene();
		virtual ~Scene();

		/// Renders the scene, using the passed state as
		/// the root state - it is not necessary for this
		/// state to already be bound.
		virtual void render( State *state ) const;
		/// Convenience function to bind a default state
		/// and then call render() with it.
		void render() const;
		virtual Imath::Box3f bound() const;

		/// Fills hits with HitRecords for all primitives which are visible within the specified
		/// region. The region is specified in NDC space (0,0 at top left) in the same
		/// way a crop window would be. As with the render() method, if the Scene has a camera
		/// then that will be used to specify the framing - otherwise you may frame the Scene
		/// using raw gl calls before calling Scene::select. In either case the region applies.
		/// \todo Have an overload which takes a Box2i specifying a raster space region
		/// instead.
		size_t select( Selector::Mode mode, const Imath::Box2f &region, std::vector<HitRecord> &hits ) const;

		/// Sets the camera used to view the scene. If unspecified then
		/// you may position the scene using raw gl calls before
		/// calling Scene::render(). If a camera is specified however, then
		/// the Scene::render() method will set up the camera for you.
		void setCamera( CameraPtr camera );
		CameraPtr getCamera();
		ConstCameraPtr getCamera() const;

		/// Returns the root node for the scene. The
		/// scene can be edited by editing the root node.
		GroupPtr root();
		/// Returns the root node for the scene.
		ConstGroupPtr root() const;

	private :

		GroupPtr m_root;
		CameraPtr m_camera;

};

IE_CORE_DECLAREPTR( Scene );

} // namespace IECoreGL

#endif // IECOREGL_SCENE_H
