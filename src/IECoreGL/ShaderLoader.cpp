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

#include "IECoreGL/ShaderLoader.h"

#include "IECoreGL/Shader.h"

#include "IECore/MessageHandler.h"

#include "boost/format.hpp"
#include "boost/wave.hpp"
#include "boost/wave/cpplexer/cpp_lex_iterator.hpp"
#include "boost/wave/cpplexer/cpp_lex_token.hpp"

#include <fstream>
#include <iostream>

using namespace IECoreGL;
using namespace IECore;
using namespace boost::filesystem;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// GLSLPreprocessingHooks implementation
//////////////////////////////////////////////////////////////////////////

namespace IECoreGL
{

namespace Detail
{

// The GLSLPreprocessingHooks class is used to modify the boost::wave preprocessor to work
// with GLSL source code. Although GLSL has its own preprocessor, it is limited in functionality
// and in particular doesn't allow #include directives, which is why we want to use boost::wave.
// However, GLSL also defines additional directives (version and extension) which would trip up
// the wave preprocessor if we didn't give a helping hand, which is what this class is all about.
class GLSLPreprocessingHooks : public boost::wave::context_policies::default_preprocessing_hooks
{

	public :

		// Pass through "#version" and "#extension" directives unmodified so that they can
		// be processed by the GL preprocessor itself.
		template<typename ContextT, typename ContainerT>
		bool found_unknown_directive( const ContextT &ctx, const ContainerT &line, ContainerT &pending )
		{
			typename ContainerT::const_iterator it = line.begin();
			if( *it != boost::wave::T_POUND )
			{
				return false;
			}

			it++;
			if( it->get_value() == "version" || it->get_value() == "extension" )
			{
				std::copy( line.begin(), line.end(), std::back_inserter( pending ) );
				return true;
			}

			return false;
		}

};

} // namespace Detail

} // namespace IECoreGL

//////////////////////////////////////////////////////////////////////////
// ShaderLoader::Implementation
//////////////////////////////////////////////////////////////////////////

class ShaderLoader::Implementation : public IECore::RefCounted
{

	public :

		Implementation( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths )
			:	m_searchPaths( searchPaths ),
				m_preprocess( preprocessorSearchPaths ),
				m_preprocessorSearchPaths( preprocessorSearchPaths ? *preprocessorSearchPaths : SearchPath() )
		{
		}

		void loadSource( const std::string &name, std::string &vertexSource, std::string &geometrySource, std::string &fragmentSource )
		{
			SourceMap::const_iterator it = m_loadedSource.find( name );
			if( it == m_loadedSource.end() )
			{
				Source source;

				path vertexPath = m_searchPaths.find( name + ".vert" );
				path geometryPath = m_searchPaths.find( name + ".geom" );
				path fragmentPath = m_searchPaths.find( name + ".frag" );
				if( vertexPath.empty() && geometryPath.empty() && fragmentPath.empty() )
				{
					IECore::msg( IECore::Msg::Error, "IECoreGL::ShaderLoader::loadSource", boost::format( "Couldn't find \"%s\"." ) % name );
				}

				if( !vertexPath.empty() )
				{
					source.vertex = readFile( vertexPath.string() );
				}

				if( !geometryPath.empty() )
				{
					source.geometry = readFile( geometryPath.string() );
				}

				if( !fragmentPath.empty() )
				{
					source.fragment = readFile( fragmentPath.string() );
				}

				it = m_loadedSource.insert( SourceMap::value_type( name, source ) ).first;
			}

			vertexSource = it->second.vertex;
			geometrySource = it->second.geometry;
			fragmentSource = it->second.fragment;
		}

		ShaderPtr create( const std::string &vertexShader, const std::string &geometryShader, const std::string &fragmentShader )
		{
			std::string uniqueName = vertexShader + "\n## Geometry ##\n" + geometryShader + "## Fragment ##\n" + fragmentShader;

			ShaderMap::iterator it = m_loadedShaders.find( uniqueName );
			if( it!=m_loadedShaders.end() )
			{
				return it->second;
			}

			clearUnused();

			ShaderPtr s = new Shader(
				preprocessShader( "<Vertex Shader>", vertexShader ),
				preprocessShader( "<Geometry Shader>", geometryShader ),
				preprocessShader( "<Fragment Shader>", fragmentShader )
			);
			m_loadedShaders[uniqueName] = s;

			return s;
		}

		ShaderPtr load( const std::string &name )
		{
			std::string vertexSource, geometrySource, fragmentSource;
			loadSource( name, vertexSource, geometrySource, fragmentSource );
			return create( vertexSource, geometrySource, fragmentSource );
		}

		void clearUnused()
		{
			ShaderMap::iterator it = m_loadedShaders.begin();
			while( it != m_loadedShaders.end() )
			{
				if ( it->second->refCount() == 1 )
				{
					m_loadedShaders.erase( it++ );
				}
				else
				{
					++it;
				}
			}
		}

		void clear()
		{
			m_loadedSource.clear();
			m_loadedShaders.clear();
		}

	private :

		struct Source
		{
			std::string vertex;
			std::string geometry;
			std::string fragment;
		};
		typedef std::map<std::string, Source> SourceMap;
		SourceMap m_loadedSource; // maps from shader name to shader source

		typedef std::map<std::string, ShaderPtr> ShaderMap;
		ShaderMap m_loadedShaders;

		IECore::SearchPath m_searchPaths;
		bool m_preprocess;
		IECore::SearchPath m_preprocessorSearchPaths;

		std::string readFile( const std::string &fileName ) const
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

		std::string preprocessShader( const std::string &fileName, const std::string &source ) const
		{
			if ( source == "" )
			{
				return source;
			}

			string result = source + "\n";

			if( m_preprocess )
			{
				try
				{
					typedef boost::wave::cpplexer::lex_token<> Token;
					typedef boost::wave::cpplexer::lex_iterator<Token> LexIterator;
					typedef boost::wave::context<
						std::string::iterator,
						LexIterator,
						boost::wave::iteration_context_policies::load_file_to_string,
						Detail::GLSLPreprocessingHooks
					> Context;

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
					throw IECore::Exception( boost::str( boost::format( "Error during preprocessing : %s line %d : %s" ) % fileName % e.line_no() % e.description() ) );
				}
			}
			return result;
		}

};

//////////////////////////////////////////////////////////////////////////
// ShaderLoader
//////////////////////////////////////////////////////////////////////////

ShaderLoader::ShaderLoader( const IECore::SearchPath &searchPaths, const IECore::SearchPath *preprocessorSearchPaths )
	:	m_implementation( new Implementation( searchPaths, preprocessorSearchPaths ) )
{
}

ShaderLoader::~ShaderLoader()
{
}

void ShaderLoader::loadSource( const std::string &name, std::string &vertexShader, std::string &geometryShader, std::string &fragmentShader )
{
	m_implementation->loadSource( name, vertexShader, geometryShader, fragmentShader );
}

ShaderPtr ShaderLoader::create( const std::string &vertexShader, const std::string &geometryShader, const std::string &fragmentShader )
{
	return m_implementation->create( vertexShader, geometryShader, fragmentShader );
}

void ShaderLoader::clearUnused( )
{
	m_implementation->clearUnused();
}

void ShaderLoader::clear( )
{
	m_implementation->clear();
}

ShaderPtr ShaderLoader::load( const std::string &name )
{
	return m_implementation->load( name );
}

ShaderLoader *ShaderLoader::defaultShaderLoader()
{
	static ShaderLoaderPtr t = nullptr;
	if( !t )
	{
		const char *e = getenv( "IECOREGL_SHADER_PATHS" );
		const char *p = getenv( "IECOREGL_SHADER_INCLUDE_PATHS" );
		IECore::SearchPath pp( p ? p : "" );
		t = new ShaderLoader( IECore::SearchPath( e ? e : "" ), p ? &pp : nullptr );
	}
	return t.get();
}
