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

#include "IECoreAppleseed/ShaderAlgo.h"

#include "IECoreAppleseed/ParameterAlgo.h"

#include "IECoreScene/Shader.h"

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

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

void addConnections( const CompoundDataMap &parameters, const std::string &handle, asr::ShaderGroup *shaderGroup )
{
	for( CompoundDataMap::const_iterator it = parameters.begin(), eIt = parameters.end(); it != eIt; ++it )
	{
		if( it->second->typeId() != StringDataTypeId )
		{
			continue;
		}

		const std::string &value = static_cast<const StringData *>( it->second.get() )->readable();
		if( boost::starts_with( value, "link:" ) )
		{
			size_t dot_pos = value.find_first_of( '.' );

			if( dot_pos == std::string::npos )
			{
				IECore::msg( Msg::Warning, "AppleseedRenderer", boost::format( "Shader parameter \"%s\" has unexpected value \"%s\" - expected value of the form \"link:sourceShader.sourceParameter" ) % it->first.string() % value );
				continue;
			}

			shaderGroup->add_connection( std::string( value, 5, dot_pos - 5 ).c_str(), std::string( value, dot_pos + 1).c_str(), handle.c_str(), it->first.c_str() );
		}
	}
}

}

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreAppleseed
{

namespace ShaderAlgo
{

renderer::ShaderGroup *convert( const IECore::ObjectVector *shaderNetwork )
{
	asf::auto_release_ptr<asr::ShaderGroup> shaderGroup;
	shaderGroup = asr::ShaderGroupFactory::create( "shader_group" );

	for( ObjectVector::MemberContainer::const_iterator it = shaderNetwork->members().begin(), eIt = shaderNetwork->members().end(); it != eIt; ++it )
	{
		const char *shaderName = NULL;
		const char *shaderType = NULL;
		const CompoundDataMap *parameters = NULL;
		if( const Shader *shader = runTimeCast<const Shader>( it->get() ) )
		{
			shaderName = shader->getName().c_str();
			shaderType = shader->getType().c_str();
			parameters = &shader->parameters();
		}

		if( !shaderName || !shaderType )
		{
			continue;
		}

		// Strip the osl: prefix.
		if( strncmp( shaderType, "osl:", 4) == 0 )
		{
			shaderType += 4;
		}

		std::string name = boost::lexical_cast<std::string>( shaderGroup->shaders().size() );
		asr::ParamArray params( ParameterAlgo::convertShaderParameters( *parameters, name ) );
		shaderGroup->add_shader( shaderType, shaderName, name.c_str(), params );
		addConnections( *parameters, name, shaderGroup.get() );
	}

	return shaderGroup.release();
}

} // namespace ShaderAlgo

} // namespace IECoreAppleseed
