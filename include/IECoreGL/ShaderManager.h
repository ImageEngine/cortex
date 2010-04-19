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

#ifndef IECOREGL_SHADERMANAGER_H
#define IECOREGL_SHADERMANAGER_H

#include "IECore/RefCounted.h"
#include "IECore/SearchPath.h"

#include <map>
#include <string>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Shader );
IE_CORE_FORWARDDECLARE( ShaderManager );

/// This class manages shaders by keeping track of their
/// reference counters and also provides read methods,
/// preprocessing and shader creation. 
/// The ShaderManager keeps a cache of created shaders based on 
/// their source code, so repeatedly asking for the same code 
/// will always return the same Shader instance.
class ShaderManager : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( ShaderManager );

		/// Creates a ShaderManager which will search for
		/// source files on the given search paths. If preprocessorSearchPaths is
		/// specified, then source preprocessing will be performed using boost::wave.
		ShaderManager( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths=0 );

		/// Loads the Shader code of the specified name.
		/// It will attempt to locate and load source from the files 
		/// "name.vert" and "name.frag". If either file
		/// is not found then return empty strings representing that
		/// the standard OpenGL fixed functionality should be used instead.
		void loadShaderCode( const std::string &name, std::string &vertexShader, std::string &fragmentShader ) const;

		/// Creates a new Shader if necessary or returns a previously compiled shader from the cache.
		/// In case the given shader was not in the cache it will do preprocessing (adding include files)
		/// and then return a new instance os Shader object.
		/// This function will also eliminate any shader from the cache that is not being used.
		ShaderPtr create( const std::string &vertexShader, const std::string &fragmentShader );

		/// Loads the shader code and creates the Shader object.
		/// This function can only be called when the OpenGL context is defined.
		ShaderPtr load( const std::string &name );

		/// Frees unused shaders.
		/// Automatically called by create() function.
		void clearUnused();

		/// Returns a static ShaderManager instance that everyone
		/// can use. This has searchpaths set using the
		/// IECOREGL_SHADER_PATHS environment variable,
		/// and preprocessor searchpaths set using the
		/// IECOREGL_SHADER_INCLUDE_PATHS environment
		/// variable.
		static ShaderManagerPtr defaultShaderManager();

	private :

		typedef std::map<std::string, ShaderPtr> ShaderMap;
		ShaderMap m_loadedShaders;

		IECore::SearchPath m_searchPaths;
		bool m_preprocess;
		IECore::SearchPath m_preprocessorSearchPaths;

		std::string readFile( const std::string &fileName ) const;
		std::string preprocessShader( const std::string &fileName, const std::string &source ) const;

};

} // namespace IECoreGL

#endif // IECOREGL_SHADERMANAGER_H
