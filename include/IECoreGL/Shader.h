//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

		/////////////////////////////////////////////////////////////
		///
		/// Uniform Parameters
		///
		/////////////////////////////////////////////////////////////
	
		/// Fills the passed vector with the names of all uniform shader parameters.
		/// Structures will use the struct.component convention used in GLSL.
		/// Arrays will be returned as a single name, rather than the list array[0],
		/// array[n] names used internally in OpenGL.
		void uniformParameterNames( std::vector<std::string> &names ) const;
		/// Returns a numeric index for the named uniform parameter. This can be
		/// used in the calls below to avoid more expensive lookups by name.
		/// Throws an Exception if parameter does not exist.
		GLint uniformParameterIndex( const std::string &parameterName ) const;
		/// Returns true if the Shader has a uniform parameter of the given name.
		bool hasUniformParameter( const std::string &parameterName ) const;

		/// Returns the type of a named uniform parameter, described in terms of the
		/// most closely related IECore datatype. The type here is the type of
		/// data returned by the getUniformParameter() function below, except in the case
		/// of 2d samplers, where TextureTypeId will be returned.
		IECore::TypeId uniformParameterType( GLint parameterIndex ) const;
		/// As above but by specifying the parameter by name.
		IECore::TypeId uniformParameterType( const std::string &parameterName ) const;

		//! @name getUniformParameterDefault
		/// Returns a new data object containing the suggested default value (zero)
		/// to be set on the given parameter.
		/// The default values are not supported by OpenGL. So the shader does not
		/// have to be bound at the time of calling.
		//////////////////////////////////////////////////////////////////////
		//@{
		IECore::DataPtr getUniformParameterDefault( GLint parameterIndex ) const;
		IECore::DataPtr getUniformParameterDefault( const std::string &parameterName ) const;
		//@}

		//! @name Uniform Parameter getting
		/// These calls return the current values of shader uniform parameters. Unlike
		/// the calls to set values (see below) the shader does not have to be
		/// bound at the time of calling.
		//////////////////////////////////////////////////////////////////////
		//@{
		IECore::DataPtr getUniformParameter( GLint parameterIndex ) const;
		IECore::DataPtr getUniformParameter( const std::string &parameterName ) const;
		//@}

		//! @name Uniform Parameter setting
		/// These calls set shader parameters. They can only be called while
		/// the Shader is bound (using bind()) as the current shader - Exceptions
		/// will result if this is not the case.
		//////////////////////////////////////////////////////////////////////
		//@{
		/// Returns true if the specified data type is valid for setting the
		/// specified uniform parameter, and false if not.
		bool uniformValueValid( GLint parameterIndex, IECore::TypeId type ) const;
		/// Returns true if the specified value is valid for setting the
		/// specified uniform parameter, and false if not.
		bool uniformValueValid( GLint parameterIndex, const IECore::Data *value ) const;
		/// As above, but specifying the uniform parameter by name.
		bool uniformValueValid( const std::string &parameterName, const IECore::Data *value ) const;
		/// Sets the specified uniform parameter to the value specified. value must
		/// be of an appropriate type for the parameter - an Exception is thrown
		/// if this is not the case.
		void setUniformParameter( GLint parameterIndex, const IECore::Data *value );
		/// Sets the specified parameter to the value provided. value must be
		/// of an appropriate simple type or a Imath type.
		template< typename T >
		typename boost::disable_if_c< boost::is_convertible< T, IECore::Data * >::value >::type 
		setUniformParameter( GLint parameterIndex, const T &value )
		{
			IECore::TypeId t = IECore::TypedData< T >::staticTypeId();
			if ( !uniformValueValid( parameterIndex, t ) )
			{
				throw IECore::Exception( "Can't set uniform parameter value. Type mismatch." );
			}
			setUniformParameter( parameterIndex, t, &value );
		}
		/// Sets the specified parameter to the value specified. value must
		/// be of an appropriate type for the parameter - an Exception is thrown
		/// if this is not the case. This call may be slower than the overload based
		/// on parameter indexes.
		void setUniformParameter( const std::string &parameterName, const IECore::Data *value );
		/// Sets the specified sampler parameter to use the texture unit indicated.
		void setUniformParameter( GLint parameterIndex, unsigned int textureUnit );
		/// Sets the specified sampler parameter to use the texture unit indicated.
		/// This call may be slower than the overload based
		/// on parameter indexes.
		void setUniformParameter( const std::string &parameterName, unsigned int textureUnit );
		void setUniformParameter( GLint parameterIndex, int value );
		void setUniformParameter( const std::string &parameterName, int value );
		/// Returns true if the specified vector data is valid for setting one of it's items to the
		/// specified uniform parameter, and false if not.
		bool uniformVectorValueValid( GLint parameterIndex, const IECore::Data *value ) const;
		/// As above, but specifying the uniform parameter by name.
		bool uniformVectorValueValid( const std::string &parameterName, const IECore::Data *value ) const;
		/// Sets the specified uniform parameter to a single item from a vector Data type.
		/// Raises an exception if the type is not compatible.
		void setUniformParameterFromVector( GLint parameterIndex, const IECore::Data *vector, unsigned int item );
		void setUniformParameterFromVector( const std::string &parameterName, const IECore::Data *vector, unsigned int item );

		struct VertexToUniform 
		{
			public:
				VertexToUniform();
				VertexToUniform( GLint p, unsigned char d, bool i, const void *a );
				void operator() ( int index ) const;

			private:
				GLint m_paramId;
				unsigned char m_dimensions;
				bool m_isInteger;
				const void *m_array;
		};
		/// Returns a callable object that sets a uniform shader parameter with a single item value from a given vector.
		VertexToUniform uniformParameterFromVectorSetup( GLint parameterIndex, const IECore::Data *vector ) const;
		//@}

		/////////////////////////////////////////////////////////////
		///
		/// Vertex Parameters
		///
		/////////////////////////////////////////////////////////////
		
		/// Fills the passed vector with the names of all vertex shader parameters.
		void vertexParameterNames( std::vector<std::string> &names ) const;
		/// Returns a numeric index for the named vertex parameter. This can be
		/// used in the calls below to avoid more expensive lookups by name.
		/// Throws an Exception if parameter does not exist.
		GLint vertexParameterIndex( const std::string &parameterName ) const;
		/// Returns true if the Shader has a vertex parameter of the given name.
		bool hasVertexParameter( const std::string &parameterName ) const;
		/// Returns true if the specified vertex data object is valid for setting the
		/// specified vertex parameter, and false if not.
		bool vertexValueValid( GLint parameterIndex, const IECore::Data *value ) const;
		/// As above, but specifying the vertex parameter by name.
		bool vertexValueValid( const std::string &parameterName, const IECore::Data *value ) const;
		/// Sets the specified vertex parameter to the value specified. value must
		/// be of an appropriate type for the parameter - an Exception is thrown
		/// if this is not the case.
		/// Derived classes can set normalize to true when they know a integer typed
		/// vector should be normalized to [-1,1] or [0,1] when passed to the shader.
		void setVertexParameter( GLint parameterIndex, const IECore::Data *value, bool normalize = false );
		/// As above, but specifying the vertex parameter by name.
		void setVertexParameter( const std::string &parameterName, const IECore::Data *value, bool normalize = false );
		/// Unsets all vertex parameters from the shader.
		void unsetVertexParameters();

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

		/// default constructor creates an empty Shader, which simply disables OpenGL shaders and
		/// only accepts old gl vertex and uniform parameters.
		Shader();

		struct VectorValueValid;
		struct VectorSetValue;
		struct VectorSetup;

		void compile( const std::string &source, GLenum type, GLuint &shader );
		void release();

		size_t getDataSize( const IECore::Data* data );

		void setUniformParameter( GLint parameterIndex, IECore::TypeId type, const void *p );

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
		ParameterMap m_uniformParameters;
		ParameterMap m_vertexParameters;
		/// Throws an Exception if the parameter doesn't exist.
		const ParameterDescription &uniformParameterDescription( GLint parameterIndex ) const;
		/// Throws an Exception if the parameter doesn't exist.
		const ParameterDescription &vertexParameterDescription( GLint parameterIndex ) const;
};

} // namespace IECoreGL

#endif // IECOREGL_SHADER_H
