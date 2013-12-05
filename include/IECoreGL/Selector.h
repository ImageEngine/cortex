//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SELECTOR_H
#define IECOREGL_SELECTOR_H

#include <vector>

#include "OpenEXR/ImathBox.h"

#include "IECore/RefCounted.h"

#include "IECoreGL/HitRecord.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Shader );

/// The Selector class simplifies the process of selecting objects
/// rendered with OpenGL.
class Selector : boost::noncopyable
{

	public :

		/// The Mode defines the method used to perform selection.
		/// Each mode has pros and cons.
		enum Mode
		{
			Invalid,
			/// Uses glRenderMode( GL_SELECT ). This can select multiple
			/// overlapping objects at once and provides accurate depth
			/// information for each. However it is officially deprecated
			/// in modern OpenGL and has terrible performance on many
			/// modern drivers due to using a software fallback path.
			GLSelect,
			/// Uses OpenGL occlusion queries. This can also select
			/// multiple overlapping objects at once but does not provide
			/// depth information at all.
			OcclusionQuery,
			/// Renders each object to an offscreen framebuffer using a
			/// unique colour per object. Can therefore only select the
			/// frontmost objects, but does provide accurate depth information.
			/// Note that this mode is currently only supported for GLSL
			/// versions 330 and up - lesser versions will fall back to using
			/// the GLSelect mode.
			IDRender
		};

		/// Starts an operation to select objects in the specified
		/// region of NDC space (0,0-1,1 top left to bottom right).
		/// Set up the GL camera, construct a Selector, then render
		/// the objects with appropriate loadName() calls with names
		/// generated using NameStateComponent. It is the caller's
		/// responsibility to keep the hits vector alive for the lifetime
		/// of the Selector.
		Selector( const Imath::Box2f &region, Mode mode, std::vector<HitRecord> &hits );
		/// Completes the selection operation, filling in the vector
		/// of hits that was passed to the constructor.
		virtual ~Selector();
		
		/// Returns the mode this Selector is operating in.
		Mode mode() const;
		
		/// Call this to set the name attached to subsequently rendered objects.
		/// If rendering a Scene, this will be called automatically
		/// by the NameStateComponents within the Scene.
		void loadName( GLuint name );
		
		/// A State that should be used as the base state for
		/// selection drawing.
		State *baseState();
		
		/// The IDRender mode requires a shader which takes a name
		/// via a "uniform uint ieCoreGLName" parameter and outputs it
		/// via an "out uint ieCoreGLNameOut" fragment output.
		/// Typically one is set up automatically in baseState(), but
		/// if rendering must be performed with an alternative shader
		/// then it may be passed via this function.
		void pushIDShader( const IECoreGL::Shader *idShader );
		
		/// Revert to previous ID shader:
		void popIDShader();
		
		/// Deprecated: calls pushIDShader
		void loadIDShader( const IECoreGL::Shader *idShader );
		
		/// Returns the currently active Selector - this may be used
		/// in drawing code to retrieve a selector to call loadName()
		/// on. The NameStateComponent is an example of a class which
		/// does this.
		static Selector *currentSelector();
		
	private :
		
		IE_CORE_FORWARDDECLARE( Implementation )
		ImplementationPtr m_implementation;
						
};

} // namespace IECoreGL

#endif // IECOREGL_SELECTOR_H
