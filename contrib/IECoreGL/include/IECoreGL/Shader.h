//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/GL.h"
#include "IECoreGL/Bindable.h"
#include "IECoreGL/TypeIds.h"

#include "IECore/CompoundData.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( Primitive );

/// A class to represent GLSL shaders.
class Shader : public Bindable
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Shader, ShaderTypeId, Bindable );

		/// Either vertexSource or fragmentSource may be empty to use the fixed
		/// functionality for that shader component.
		/// Throws an Exception if the shader fails to compile, or if the OpenGL
		/// version isn't sufficient to support shaders.
		Shader( const std::string &vertexSource, const std::string &fragmentSource );
		virtual ~Shader();

		bool operator==( const Shader &other ) const;

		virtual void bind() const;
		/// \bug This returns 0, as I haven't yet found a way of pushing/popping
		/// the current shader in GL. This is worked around by pushing and popping
		/// the current program by hand in Group::render().
		virtual GLbitfield mask() const;

		/// Fills the passed vector with the names of all shader parameters.
		/// Structures will use the struct.component convention used in GLSL.
		/// Arrays will be returned as a single name, rather than the list array[0],
		/// array[n] names used internally in OpenGL.
		void parameterNames( std::vector<std::string> &names ) const;
		/// Returns a numeric index for the named parameter. This can be
		/// used in the calls below to avoid more expensive lookups by name.
		/// Throws an Exception if parameter does not exist.
		GLint parameterIndex( const std::string &parameterName ) const;
		/// Returns true if the Shader has a parameter of the given name.
		bool hasParameter( const std::string &parameterName ) const;

		/// Returns the type of a named parameter, described in terms of the
		/// most closely related IECore datatype. The type here is the type of
		/// data returned by the getParameter() function below, except in the case
		/// of 2d samplers, where TextureTypeId will be returned.
		IECore::TypeId parameterType( GLint parameterIndex ) const;
		/// As above but by specifying the parameter by name.
		IECore::TypeId parameterType( const std::string &parameterName ) const;

		//! @name Parameter getting
		/// These calls return the current values of shader parameters. Unlike
		/// the calls to set values (see below) the shader does not have to be
		/// bound at the time of calling.
		//////////////////////////////////////////////////////////////////////
		//@{
		IECore::DataPtr getParameter( GLint parameterIndex ) const;
		IECore::DataPtr getParameter( const std::string &parameterName ) const;
		//@}

		//! @name Parameter setting
		/// These calls set shader parameters. They can only be called while
		/// the Shader is bound (using bind()) as the current shader - Exceptions
		/// will result if this is not the case.
		/// \todo Might it be possible just to store the values being set if the
		/// shader isn't currently bound? And then flush those values on bind().
		//////////////////////////////////////////////////////////////////////
		//@{
		/// Returns true if the specified value is valid for setting the
		/// specified parameter, and false if not.
		bool valueValid( GLint parameterIndex, IECore::ConstDataPtr value ) const;
		/// As above, but specifying the parameter by name.
		bool valueValid( const std::string &parameterName, IECore::ConstDataPtr value ) const;
		/// Sets the specified parameter to the value specified. value must
		/// be of an appropriate type for the parameter - an Exception is thrown
		/// if this is not the case.
		void setParameter( GLint parameterIndex, IECore::ConstDataPtr value );
		/// Sets the specified parameter to the value specified. value must
		/// be of an appropriate type for the parameter - an Exception is thrown
		/// if this is not the case. This call may be slower than the overload based
		/// on parameter indexes.
		void setParameter( const std::string &parameterName, IECore::ConstDataPtr value );
		/// Sets the specified sampler parameter to use the texture unit indicated.
		void setParameter( GLint parameterIndex, unsigned int textureUnit );
		/// Sets the specified sampler parameter to use the texture unit indicated.
		/// This call may be slower than the overload based
		/// on parameter indexes.
		void setParameter( const std::string &parameterName, unsigned int textureUnit );
		void setParameter( GLint parameterIndex, int value );
		void setParameter( const std::string &parameterName, int value );
		//@}

		//! @name Built in shaders
		/// These functions provide access to static instances of
		/// various simple but useful shaders.
		///////////////////////////////////////////////////////////
		//@{
		/// Returns a shader which shades as a constant flat color
		/// using the current gl color.
		static ShaderPtr constant();
		/// Returns a shader which shades as a facing ratio.
		static ShaderPtr facingRatio();
		//@}

	private :

		// The Primitive class needs access to the internals of the Shader so it
		// can set vertex attributes for shader attributes. This functionality
		// isn't provided as a public method because there's no way that the Shader
		// can validate that the right amount of data is being passed - whereas
		// the Primitive is in a position to do this.
		friend class Primitive;

		void compile( const std::string &source, GLenum type, GLuint &shader );
		void release();

		GLuint m_vertexShader;
		GLuint m_fragmentShader;
		GLuint m_program;

		struct ParameterDescription
		{
			std::string name;
			GLenum type;
			GLint size;
		};
		/// Maps from the uniform location to the parameter details.
		typedef std::map<GLint, ParameterDescription> ParameterMap;
		ParameterMap m_parameters;
		/// Throws an Exception if the parameter doesn't exist.
		const ParameterDescription &parameterDescription( GLint parameterIndex ) const;
};

} // namespace IECoreGL

#endif // IECOREGL_SHADER_H
