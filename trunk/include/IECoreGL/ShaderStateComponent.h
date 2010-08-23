//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/StateComponent.h"
#include "IECoreGL/ShaderManager.h"
#include "IECoreGL/TextureLoader.h"
#include "IECore/CompoundObject.h"

#include <map>
#include <set>
#include <string>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( Texture );

/// The ShaderStateComponent class represents a Shader
/// object and a set of associated parameter values. It derives
/// from StateComponent and therefore can be used to apply Shaders
/// to Primitives within a Group or Scene.
/// \todo Allow this to specify texture filtering and wrap modes.
class ShaderStateComponent : public StateComponent
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::ShaderStateComponent, ShaderStateComponentTypeId, StateComponent );

		// default constructor uses no shader.
		ShaderStateComponent();
		ShaderStateComponent( const ShaderStateComponent &other );
		// Creates a ShaderStateComponent with the given parameters.
		ShaderStateComponent( ShaderManagerPtr shaderManager, TextureLoaderPtr textureLoader, const std::string vertexShader, const std::string fragmentShader, IECore::ConstCompoundObjectPtr parameterValues = 0 );

		//! @name Bindable interface
		////////////////////////////////////////////////////
		//@{
		virtual void bind() const;
		//@}

		//! @name shader
		// Returns the shader object.
		// This function can only be called from a thread 
		// with the valid GL context loaded.
		////////////////////////////////////////////////////
		//@{
		ShaderPtr shader();
		ShaderPtr shader() const;
		//@}

		// Adds or replaces a shader parameter in the ShaderStateComponent.
		// this function can be called even if there's no GL context.
		void addShaderParameterValue( const std::string &paramName, IECore::ConstObjectPtr paramValue );

	protected :

		typedef std::map<std::string, ConstTexturePtr> TexturesMap;

		ShaderManagerPtr m_shaderManager;
		TextureLoaderPtr m_textureLoader;
		std::string m_fragmentShader;
		std::string m_vertexShader;
		IECore::CompoundObjectPtr m_parameterMap;
		ShaderPtr m_shader;

		mutable std::set< std::string > m_dirtyTextures;
		mutable TexturesMap m_textureParameters;

		static Description<ShaderStateComponent> g_description;
};

IE_CORE_DECLAREPTR( ShaderStateComponent );

} // namespace IECoreGL

#endif // IECOREGL_SHADERSTATECOMPONENT_H
