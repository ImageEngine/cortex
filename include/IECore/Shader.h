//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SHADER_H
#define IE_CORE_SHADER_H

#include "IECore/StateRenderable.h"

namespace IECore
{

/// A class to represent shaders.
class Shader : public StateRenderable
{
	public:
	
		Shader( const std::string &name="defaultsurface", const std::string &type="surface", const CompoundDataMap &parameters = CompoundDataMap() );
		virtual ~Shader();
				
		IE_CORE_DECLAREABSTRACTOBJECT( Shader, StateRenderable );
		
		const std::string &getName() const;
		void setName( const std::string &name );
		
		const std::string &getType() const;
		void setType( const std::string &type );
		
		CompoundDataMap &parameters();
		const CompoundDataMap &parameters() const;
		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		CompoundDataPtr parametersData();
		
		/// Sets this shader as the current shader in Renderer.
		virtual void render( RendererPtr renderer ) const;
	
	private:
	
		static TypeDescription<Shader> m_description;
		
		std::string m_name;
		std::string m_type;
		CompoundDataPtr m_parameters;
	
		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Shader );

} // namespace IECore


#endif // IE_CORE_SHADER_H
