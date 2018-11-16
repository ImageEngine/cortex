//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/ShaderNetworkAlgo.h"

#include "IECoreAppleseed/ParameterAlgo.h"

#include "IECoreScene/Shader.h"
#include "IECoreScene/ShaderNetworkAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/lexical_cast.hpp"

#include <sstream>
#include <string>

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAppleseed;

namespace asf = foundation;
namespace asr = renderer;

namespace IECoreAppleseed
{

namespace ShaderNetworkAlgo
{

renderer::ShaderGroup *convert( const IECoreScene::ShaderNetwork *shaderNetwork )
{
	asf::auto_release_ptr<asr::ShaderGroup> shaderGroup;
	shaderGroup = asr::ShaderGroupFactory::create( "shader_group" );

	IECoreScene::ShaderNetworkAlgo::depthFirstTraverse(
		shaderNetwork,
		[&shaderGroup] ( const ShaderNetwork *shaderNetwork, const InternedString &handle ) {

			const Shader *shader = shaderNetwork->getShader( handle );

			const char *shaderType = shader->getType().c_str();
			if( strncmp( shaderType, "osl:", 4) == 0 )
			{
				// Strip the osl: prefix.
				shaderType += 4;
			}

			asr::ParamArray params( ParameterAlgo::convertShaderParameters( shader->parameters() ) );
			shaderGroup->add_shader( shaderType, shader->getName().c_str(), handle.c_str(), params );

			for( const auto &c : shaderNetwork->inputConnections( handle ) )
			{
				shaderGroup->add_connection(
					c.source.shader.c_str(), c.source.name.c_str(),
					c.destination.shader.c_str(), c.destination.name.c_str()
				);
			}
		}
	);

	return shaderGroup.release();
}

} // namespace ShaderNetworkAlgo

} // namespace IECoreAppleseed
