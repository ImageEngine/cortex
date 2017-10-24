//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IECOREGL_SHADER_H
#define IECOREGL_SHADER_H

#include <boost/utility.hpp>

#include "IECoreGL/Export.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/Bindable.h"
#include "IECoreGL/TypeIds.h"

#include "IECore/CompoundData.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( Texture );

/// A class to represent GLSL shaders.
class IECOREGL_API Shader : public IECore::RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Shader, ShaderTypeId, IECore::RunTimeTyped );

		/// Either vertexSource or fragmentSource may be empty to use a simple default
		/// shader for that shader component. Throws a descriptive Exception if the shader
		/// fails to compile.
		Shader( const std::string &vertexSource, const std::string &fragmentSource );
		/// Either vertexSource or fragmentSource may be empty to use a simple default
		/// shader for that shader component. If geometrySource is empty then no geometry shader
		/// will be used. Throws a descriptive Exception if the shader fails to compile.
		Shader( const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource );
		~Shader() override;

		/// Returns the GL program this shader represents. Note that this is owned by the Shader,
		/// and will be destroyed upon destruction of the Shader - you must not call glDeleteProgram() yourself.
		GLuint program() const;

		//! @name Source accessors.
		/// These functions return the shader source as passed to
		/// the constructor. In the case of an empty string being
		/// returned, you can determine the effective source by
		/// calling defaultVertexSource() or defaultFragmentSource().
		///////////////////////////////////////////////////////////
		//@{
		const std::string &vertexSource() const;
		const std::string &geometrySource() const;
		const std::string &fragmentSource() const;
		//@}

		struct IECOREGL_API Parameter
		{

			/// Type as reported by glGetActiveUnifom() or
			/// glGetActiveAttrib().
			GLenum type;
			/// Size as reported by glGetActiveUniform() or
			/// glGetActiveAttrib().
			GLint size;
			/// Location as reported by glGetUniformLocation()
			/// or glGetAttribLocation().
			GLint location;
			/// Texture unit assigned for this parameter - only
			/// valid for uniform parameters of sampler type.
			/// This does not store an enum but instead an index
			/// (so 0 represents GL_TEXTURE0).
			GLuint textureUnit;

			bool operator == ( const Parameter &other ) const;

		};

		/// Fills the passed vector with the names of all uniform shader parameters.
		/// Structures will use the struct.component convention used in GLSL.
		/// Arrays will be returned as a single name, rather than the list array[0],
		/// array[n] names used internally in OpenGL.
		void uniformParameterNames( std::vector<std::string> &names ) const;
		/// Returns the details of the named uniform parameter, or 0 if no such
		/// parameter exists. The return value directly references data held within
		/// the Shader, and will die when the Shader dies.
		const Parameter *uniformParameter( const std::string &name ) const;

		/// Fills the passed vector with the names of all vertex shader parameters.
		void vertexAttributeNames( std::vector<std::string> &names ) const;
		/// Returns the details of the named vertex attribute, or 0 if no such
		/// parameter exists. The return value directly references data held within
		/// the Shader, and will die when the Shader dies.
		const Parameter *vertexAttribute( const std::string &name ) const;

		//! @name Standard parameters
		/// Cortex defines a set of standard shader parameters which
		/// are used to pass state to the shaders. These functions
		/// provide rapid access to the standard parameters if
		/// they exist.
		/// \todo Pass the matrices and projections via standard
		/// parameters to avoid use of deprecated gl_ModelViewMatrix etc.
		///////////////////////////////////////////////////////////
		//@{
		/// Returns the "uniform vec3 Cs" parameter used to specify
		/// constant colours to the shader.
		const Parameter *csParameter() const;
		//@}

		/// Shaders are only useful when associated with a set of values for
		/// their uniform parameters and vertex attributes, and to render
		/// different objects in different forms different sets of values
		/// will be a necessary. The Setup class encapsulates a set of such
		/// values and provides a means of cleanly binding and unbinding the
		/// Shader using them.
		class IECOREGL_API Setup : public IECore::RefCounted
		{

			public :

				IE_CORE_DECLAREMEMBERPTR( Setup )

				Setup( ConstShaderPtr shader );
				~Setup() override;

				const Shader *shader() const;

				void addUniformParameter( const std::string &name, ConstTexturePtr value );
				void addUniformParameter( const std::string &name, IECore::ConstDataPtr value );
				/// Binds the specified value to the named vertex attribute. The divisor will be passed to
				/// glVertexAttribDivisor().
				void addVertexAttribute( const std::string &name, IECore::ConstDataPtr value, GLuint divisor = 0 );

				/// Returns true if this setup specifies a value for the standard "Cs" parameter.
				bool hasCsValue() const;

				/// The ScopedBinding class cleanly binds and unbinds the shader
				/// Setup, making the shader current and setting the uniform
				/// and vertex values as necessary.
				class IECOREGL_API ScopedBinding
				{

					public :

						/// Binds the setup. It is the responsibility of the
						/// caller to ensure the setup remains alive for
						/// the lifetime of the ScopedBinding. The setup will
						/// also be registered with the current Selector if
						/// necessary.
						ScopedBinding( const Setup &setup );
						/// Unbinds the setup, reverting to the previous state.
						~ScopedBinding();

					private :

						GLint m_previousProgram;
						const Setup &m_setup;

				};

			private :

				IE_CORE_FORWARDDECLARE( MemberData );

				MemberDataPtr m_memberData;

		};

		IE_CORE_DECLAREPTR( Setup );

		//! @name Common shader sources.
		/// These functions return some common shader sources which
		/// may be useful in many situations.
		///////////////////////////////////////////////////////////
		//@{
		/// Default vertex shader source. This takes vertexP, vertexN,
		/// vertexuv, vertexCs and Cs inputs and sets fragmentI, fragmentP, fragmentN,
		/// fragmentuv and fragmentCs outputs. It also sets equivalent geometry*
		/// outputs which may be used by geometry shaders in calculating new
		/// values for the corresponding fragment* outputs.
		static const std::string &defaultVertexSource();
		/// Default geometry shader source. This is provided to make a common
		/// interface, but is actually just a no-op.
		static const std::string &defaultGeometrySource();
		/// Default fragment shader source. This uses fragmentI, fragmentN
		/// and fragmentCs to compute a simple facing ratio.
		static const std::string &defaultFragmentSource();
		/// This uses fragmentCs as a constant flat color.
		static const std::string &constantFragmentSource();
		/// This passes fragmentP and fragmentN to ieDiffuse.
		static const std::string &lambertFragmentSource();
		//@}

		//! @name Built in shaders
		/// These functions provide access to static instances of
		/// various simple but useful shaders.
		///////////////////////////////////////////////////////////
		//@{
		/// Returns a shader which shades as a constant flat color.
		static Shader *constant();
		/// Returns a shader which shades as a facing ratio.
		static Shader *facingRatio();
		//@}

	private :

		IE_CORE_FORWARDDECLARE( Implementation )
		ImplementationPtr m_implementation;

};

} // namespace IECoreGL

#endif // IECOREGL_SHADER_H
