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

#ifndef IECOREGL_SHADERSTATECOMPONENT_H
#define IECOREGL_SHADERSTATECOMPONENT_H

#include "IECoreGL/StateComponent.h"

#include "IECore/CompoundData.h"

#include <map>
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

		typedef std::map<std::string, ConstTexturePtr> TexturesMap;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ShaderStateComponent, ShaderStateComponentTypeId, StateComponent );
		
		ShaderStateComponent();
		ShaderStateComponent( ShaderPtr shader, IECore::ConstCompoundDataPtr parameterValues = 0,
			const TexturesMap *textureParameterValues = 0 );
		
		//! @name Bindable interface
		////////////////////////////////////////////////////
		//@{
		virtual void bind() const;
		virtual GLbitfield mask() const;
		//@}

		ShaderPtr shader();
		ConstShaderPtr shader() const;
		IECore::CompoundDataPtr parameterValues();
		IECore::ConstCompoundDataPtr parameterValues() const;
		TexturesMap &textureValues();
		const TexturesMap &textureValues() const;

	protected :

		ShaderPtr m_shader;
		IECore::CompoundDataPtr m_parameterData;
		TexturesMap m_textureParameters;

		static Description<ShaderStateComponent> g_description;

};

IE_CORE_DECLAREPTR( ShaderStateComponent );

} // namespace IECoreGL

#endif // IECOREGL_SHADERSTATECOMPONENT_H
