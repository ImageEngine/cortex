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

#include "boost/wave.hpp"
#include "boost/wave/cpplexer/cpp_lex_token.hpp"
#include "boost/wave/cpplexer/cpp_lex_iterator.hpp"
#include "boost/format.hpp"

#include <fstream>
#include <iostream>

using namespace IECoreGL;
using namespace IECore;
using namespace boost::filesystem;
using namespace std;

ShaderLoader::ShaderLoader( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths )
	:	m_searchPaths( searchPaths ), m_preprocess( preprocessorSearchPaths ), m_preprocessorSearchPaths( preprocessorSearchPaths ? *preprocessorSearchPaths : SearchPath() )
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

	if( m_preprocess )
	{
		try
		{
			typedef boost::wave::cpplexer::lex_token<> Token;
			typedef boost::wave::cpplexer::lex_iterator<Token> LexIterator;
			typedef boost::wave::context<std::string::iterator, LexIterator> Context;

			Context ctx( result.begin(), result.end(), fileName.c_str() );
			// set the language so that #line directives aren't inserted (they make the ati shader compiler barf)
			ctx.set_language( boost::wave::support_normal );

			for( list<path>::const_iterator it=m_preprocessorSearchPaths.paths.begin(); it!=m_preprocessorSearchPaths.paths.end(); it++ )
			{
				string p = (*it).string();
				ctx.add_include_path( p.c_str() );
			}

			Context::iterator_type b = ctx.begin();
			Context::iterator_type e = ctx.end();

			string processed = "";
			while( b != e )
			{
				processed += (*b).get_value().c_str();
				b++;
			}

			result = processed;
		}
		catch( boost::wave::cpp_exception &e )
		{
			// rethrow but in a nicely formatted form
			throw Exception( boost::str( boost::format( "Error during preprocessing : %s line %d : %s" ) % e.file_name() % e.line_no() % e.description() ) );
		}
	}
	return result;
}


ShaderLoaderPtr ShaderLoader::defaultShaderLoader()
{
	static ShaderLoaderPtr t = 0;
	if( !t )
	{
		const char *e = getenv( "IECOREGL_SHADER_PATHS" );
		const char *p = getenv( "IECOREGL_SHADER_INCLUDE_PATHS" );
		IECore::SearchPath pp( p ? p : "", ":" );
		t = new ShaderLoader( IECore::SearchPath( e ? e : "", ":" ), p ? &pp : 0 );
	}
	return t;
}
