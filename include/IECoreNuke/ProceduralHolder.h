//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_PROCEDURALHOLDER_H
#define IECORENUKE_PROCEDURALHOLDER_H

#include "DDImage/Op.h"

#include "IECoreGL/Scene.h"

#include "IECoreNuke/ParameterisedHolder.h"

namespace IECoreNuke
{

/// This class allows IECore::ParameterisedProcedural objects
/// to be displayed on nodes in Nuke.
class ProceduralHolder : public ParameterisedHolderOp
{

	public :

		ProceduralHolder( Node *node );
		virtual ~ProceduralHolder();

		//! @name Reimplementation of Nuke methods.
		/////////////////////////////////////////////////////////////////////
		//@{
		virtual void knobs( DD::Image::Knob_Callback f );
		virtual const char *Class() const;
		virtual const char *node_help() const;
		//@}

		/// Returns the procedural which is being held.
		IECoreScene::ConstParameterisedProceduralPtr procedural();
		/// Returns the scene created by the procedural in a form
		/// suitable for OpenGL rendering.
		IECoreGL::ConstScenePtr scene();
		/// Returns the bounding box for the procedural in local space.
		/// Prefer this over calling procedural()->bound() directly because it
		/// only recomputes when necessary (when input parameters have changed).
		Imath::Box3f bound();
		/// Returns the transform for the procedural.
		Imath::M44f transform();

	protected :

		/// Implemented to draw the procedural
#if kDDImageVersionInteger >= 70000
		virtual DD::Image::Op::HandlesMode doAnyHandles( DD::Image::ViewerContext *ctx );
#elif kDDImageVersionInteger >= 62000
		virtual bool doAnyHandles( DD::Image::ViewerContext *ctx );
#endif
		virtual void build_handles( DD::Image::ViewerContext *ctx );
		virtual void draw_handle( DD::Image::ViewerContext *ctx );

		/// Implemented to distinguish these nodes from others.
		const char *node_shape() const;

	private :

		static const Description g_description;
		static DD::Image::Op *build( Node *node );

		bool m_drawContents;
		bool m_drawBound;
		bool m_drawCoordinateSystems;

		IECoreGL::ScenePtr m_scene;
		DD::Image::Hash m_sceneHash;

		Imath::Box3f m_bound;
		DD::Image::Hash m_boundHash;

		DD::Image::Matrix4 m_transform;
		DD::Image::Knob *m_transformKnob;

};

} // namespace IECoreNuke

#endif // IECORENUKE_PROCEDURALHOLDER_H
