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

#ifndef IECOREGL_SHADERLOADER_H
#define IECOREGL_SHADERLOADER_H

#include "IECore/RefCounted.h"
#include "IECore/SearchPath.h"

#include <map>
#include <string>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( ShaderLoader );

/// This simple class creates shaders based on files
/// found on disk. Asked to load a shader named "name"
/// it will attempt to locate and load source from
/// the files "name.vert" and "name.frag", and return
/// a Shader object compiled from them. If either file
/// is not found then the standard OpenGL fixed functionality
/// is used for that component of the Shader. The ShaderLoader
/// keeps a cache of loaded shaders, so repeatedly asking
/// for the same name will always return the same Shader
/// instance.
class ShaderLoader : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( ShaderLoader );

		/// Creates a ShaderLoader which will search for
		/// source files on the given search paths. If preprocessorSearchPaths is
		/// specified, then source preprocessing will be performed using boost::wave.
		ShaderLoader( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths=0 );

		/// Loads the Shader of the specified name. Returns 0
		/// if no such Shader can be found.
		ShaderPtr load( const std::string &name );

		/// Removes any cached shaders.
		void clear();

		/// Returns a static ShaderLoader instance that everyone
		/// can use. This has searchpaths set using the
		/// IECOREGL_SHADER_PATHS environment variable,
		/// and preprocessor searchpaths set using the
		/// IECOREGL_SHADER_INCLUDE_PATHS environment
		/// variable.
		static ShaderLoaderPtr defaultShaderLoader();

	private :

		typedef std::map<std::string, ShaderPtr> ShaderMap;
		ShaderMap m_loadedShaders;

		IECore::SearchPath m_searchPaths;
		bool m_preprocess;
		IECore::SearchPath m_preprocessorSearchPaths;

		std::string readFile( const std::string &fileName );

};

} // namespace IECoreGL

#endif // IECOREGL_SHADERLOADER_H
