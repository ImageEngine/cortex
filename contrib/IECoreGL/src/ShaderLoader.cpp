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

#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/Shader.h"

#include "IECore/MessageHandler.h"

#include <fstream>
#include <iostream>

using namespace IECoreGL;
using namespace boost::filesystem;
using namespace std;

ShaderLoader::ShaderLoader( const IECore::SearchPath &searchPaths )
	:	m_searchPaths( searchPaths )
{
}

ShaderPtr ShaderLoader::load( const std::string &name )
{
	ShaderMap::iterator it = m_loadedShaders.find( name );
	if( it!=m_loadedShaders.end() )
	{
		return it->second;
	}
	
	path vertexPath = m_searchPaths.find( name + ".vert" );
	path fragmentPath = m_searchPaths.find( name + ".frag" );
	
	if( vertexPath.empty() && fragmentPath.empty() )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::ShaderLoader::load", boost::format( "Couldn't find \"%s\"." ) % name );
		return 0;
	}
	
	string vertexSrc = "";
	if( !vertexPath.empty() )
	{
		vertexSrc = readFile( vertexPath.string() );
	}
	
	string fragmentSrc = "";
	if( !fragmentPath.empty() )
	{
		fragmentSrc = readFile( fragmentPath.string() );
	}
	
	if( vertexSrc=="" && fragmentSrc=="" )
	{
		return 0;
	}
			
	ShaderPtr s = new Shader( vertexSrc, fragmentSrc );
	m_loadedShaders[name] = s;
	
	return s;
}

void ShaderLoader::clear()
{
	m_loadedShaders.clear();
}

std::string ShaderLoader::readFile( const std::string &fileName )
{
	ifstream f( fileName.c_str() );

	if( !f.is_open() )
	{
		return "";
	}

	string result = "";
	while( f.good() )
	{
		string line;
		getline( f, line );
		result += line + "\n";
	}
	
	return result;
}


ShaderLoaderPtr ShaderLoader::defaultShaderLoader()
{
	static ShaderLoaderPtr t = 0;
	if( !t )
	{
		const char *e = getenv( "IECOREGL_SHADER_PATHS" );
		t = new ShaderLoader( IECore::SearchPath( e ? e : "", ":" ) );
	}
	return t;
}
