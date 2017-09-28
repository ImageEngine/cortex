//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SHADERLOADER_H
#define IECOREGL_SHADERLOADER_H

#include <map>
#include <string>

#include "IECore/RefCounted.h"
#include "IECore/SearchPath.h"

#include "IECoreGL/Export.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( ShaderLoader );

/// This class provides loading and preprocessing of GLSL shaders, and manages
/// a cache of Shader objects compiled from that source.
class IECOREGL_API ShaderLoader : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( ShaderLoader );

		/// Creates a ShaderLoader which will search for
		/// source files on the given search paths. If preprocessorSearchPaths is
		/// specified, then source preprocessing will be performed using boost::wave.
		ShaderLoader( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths=0 );
		virtual ~ShaderLoader();

		/// Loads the GLSL source code for a shader of the specified name, by
		/// locating and loading files named "name.vert". "name.geom" and "name.frag". If any
		/// file is missing then the empty string will be returned, signifying that the default
		/// shader source for that component should be used.
		void loadSource( const std::string &name, std::string &vertexSource, std::string &geometrySource, std::string &fragmentSource );

		/// Creates a new Shader if necessary or returns a previously compiled shader from the cache.
		/// In case the given shader was not in the cache it will do preprocessing (adding include files)
		/// and then return a new instance of the Shader class. This function will also eliminate any shader
		/// from the cache that is not being used.
		ShaderPtr create( const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource );

		/// Loads the shader code and creates the Shader object.
		/// This function can only be called when the OpenGL context is defined.
		ShaderPtr load( const std::string &name );

		/// Frees unused shaders.
		/// Automatically called by create() function.
		void clearUnused();


		// Free all shaders - this allows us to reload shaders and pick up changes
		void clear();

		/// Returns a static ShaderLoader instance that everyone
		/// can use. This has searchpaths set using the
		/// IECOREGL_SHADER_PATHS environment variable,
		/// and preprocessor searchpaths set using the
		/// IECOREGL_SHADER_INCLUDE_PATHS environment
		/// variable.
		static ShaderLoader *defaultShaderLoader();

	private :

		IE_CORE_FORWARDDECLARE( Implementation );
		ImplementationPtr m_implementation;

};

} // namespace IECoreGL

#endif // IECOREGL_SHADERLOADER_H
