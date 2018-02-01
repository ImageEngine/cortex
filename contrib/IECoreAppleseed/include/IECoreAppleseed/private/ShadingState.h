//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#ifndef IECOREAPPLESEED_SHADINGSTATE_H
#define IECOREAPPLESEED_SHADINGSTATE_H

#include "IECoreScene/Shader.h"

#include "IECore/MurmurHash.h"

#include "foundation/utility/searchpaths.h"
#include "renderer/api/scene.h"
#include "renderer/api/shadergroup.h"

namespace IECoreAppleseed
{

class ShadingState
{
	public :

		ShadingState();

		void setShadingSamples( int samples );

		void addOSLShader( IECoreScene::ConstShaderPtr shader );
		void setOSLSurface( IECoreScene::ConstShaderPtr surface );

		void shaderGroupHash( IECore::MurmurHash &hash ) const;
		std::string createShaderGroup( renderer::Assembly &assembly, const std::string &name );
		void editShaderGroup( renderer::Assembly &assembly, const std::string &name );

		void materialHash( IECore::MurmurHash &hash ) const;
		std::string createMaterial( renderer::Assembly &assembly, const std::string &name, const std::string &shaderGroupName );

		bool valid() const;

	private :

		renderer::ParamArray convertParameters( const IECore::CompoundDataMap &parameters );
		void addConnections( const std::string &shaderHandle, const IECore::CompoundDataMap &parameters, renderer::ShaderGroup *shaderGroup );

		std::vector<IECoreScene::ConstShaderPtr> m_shaders;
		IECoreScene::ConstShaderPtr m_surfaceShader;
		int m_shadingSamples;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_RENDERER_SHADINGSTATE_H
