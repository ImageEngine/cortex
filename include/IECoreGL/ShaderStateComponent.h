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

#ifndef IECOREGL_SHADERSTATECOMPONENT_H
#define IECOREGL_SHADERSTATECOMPONENT_H

#include "IECore/CompoundObject.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/Shader.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( ShaderLoader )
IE_CORE_FORWARDDECLARE( TextureLoader )

/// The ShaderStateComponent class represents a Shader
/// object and a set of associated parameter values. It derives
/// from StateComponent and therefore can be used to apply Shaders
/// to Primitives within a Group or Scene.
/// \todo Allow this to specify texture filtering and wrap modes.
class IECOREGL_API ShaderStateComponent : public StateComponent
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::ShaderStateComponent, ShaderStateComponentTypeId, StateComponent );

		/// Default constructor creates a facing ratio shader.
		ShaderStateComponent();
		/// Creates a ShaderStateComponent with the given source and parameters. The loaders are passed to
		/// allow the creation of GL resources to be deferred until shaderSetup() is called - this makes
		/// it possible to create ShaderStateComponents concurrently in multiple threads, with the actual
		/// GL resource creation deferred until the drawing thread uses shaderSetup().
		ShaderStateComponent( ShaderLoaderPtr shaderLoader, TextureLoaderPtr textureLoader, const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource, IECore::ConstCompoundObjectPtr parameterValues );
		/// Destructor
		~ShaderStateComponent() override;

		/// Implemented to do nothing - it is the responsibility of the Primitive
		/// to actually bind the shaderSetup() at an appropriate time.
		void bind() const override;

		ShaderLoader *shaderLoader();
		TextureLoader *textureLoader();

		/// Returns a hash to uniquely identify this shader state.
		IECore::MurmurHash hash() const;

		/// Returns a Shader::Setup object for binding the shader. This function can
		/// only be called from a thread with a valid GL context.
		Shader::Setup *shaderSetup();
		const Shader::Setup *shaderSetup() const;

		/// Adds the parameters this StateComponent holds to the specified
		/// setup - this can be of use when Primitives wish to use a modified
		/// shader to take advantage of custom vertex or geometry shaders.
		/// There is no need to call this for setups retrieved using the shaderSetup()
		/// method above - that will have been done automatically.
		void addParametersToShaderSetup( Shader::Setup *shaderSetup ) const;

	private :

		IE_CORE_FORWARDDECLARE( Implementation );
		ImplementationPtr m_implementation;

		static Description<ShaderStateComponent> g_description;

};

IE_CORE_DECLAREPTR( ShaderStateComponent );

} // namespace IECoreGL

#endif // IECOREGL_SHADERSTATECOMPONENT_H
