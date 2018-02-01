//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/SceneInterface.h"

#include "boost/filesystem/convenience.hpp"
#include "boost/tokenizer.hpp"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( SceneInterface )

const SceneInterface::Name &SceneInterface::rootName = IndexedIO::rootName;
const SceneInterface::Path &SceneInterface::rootPath = IndexedIO::rootPath;
const SceneInterface::Name &SceneInterface::visibilityName( "scene:visible" );

class SceneInterface::CreatorMap : public std::map< std::pair< std::string, IndexedIO::OpenModeFlags >, CreatorFn>
{
};

SceneInterface::CreatorMap &SceneInterface::fileCreators()
{
	static CreatorMap *g_createFns = new CreatorMap();
	return *g_createFns;
}

void SceneInterface::registerCreator( const std::string &extension, IndexedIO::OpenMode modes, CreatorFn f )
{
	CreatorMap &createFns = fileCreators();

	if ( modes & IndexedIO::Read )
	{
		std::pair< std::string, IndexedIO::OpenModeFlags > key( extension, IndexedIO::Read );
		assert( createFns.find(key) == createFns.end() );
		createFns.insert( CreatorMap::value_type(key, f) );
	}
	if ( modes & IndexedIO::Write )
	{
		std::pair< std::string, IndexedIO::OpenModeFlags > key( extension, IndexedIO::Write );
		assert( createFns.find(key) == createFns.end() );
		createFns.insert( CreatorMap::value_type(key, f) );
	}
	if ( modes & IndexedIO::Append )
	{
		std::pair< std::string, IndexedIO::OpenModeFlags > key( extension, IndexedIO::Append );
		assert( createFns.find(key) == createFns.end() );
		createFns.insert( CreatorMap::value_type(key, f) );
	}
}

std::vector<std::string> SceneInterface::supportedExtensions( IndexedIO::OpenMode modes )
{
	std::vector<std::string> extensions;
	CreatorMap &m = fileCreators();
	for( CreatorMap::const_iterator it=m.begin(); it!=m.end(); it++ )
	{
		if ( it->first.second & modes )
		{
			std::string ext = it->first.first.substr( 1 );
			if ( std::find(extensions.begin(), extensions.end(), ext) == extensions.end() )
			{
				extensions.push_back( ext );
			}
		}
	}
	return extensions;
}

SceneInterfacePtr SceneInterface::create( const std::string &path, IndexedIO::OpenMode mode )
{
	SceneInterfacePtr result = nullptr;

	std::string extension = boost::filesystem::extension(path);
	IndexedIO::OpenModeFlags openMode = IndexedIO::OpenModeFlags( mode & (IndexedIO::Read|IndexedIO::Write|IndexedIO::Append) );
	std::pair< std::string, IndexedIO::OpenModeFlags > key( extension, openMode );

	const CreatorMap &createFns = fileCreators();

	CreatorMap::const_iterator it = createFns.find(key);
	if (it == createFns.end())
	{
		throw IOException(path);
	}

	return (it->second)(path, mode);
}

SceneInterface::~SceneInterface()
{
}

bool SceneInterface::hasBound() const
{
	return true;
}

void SceneInterface::hash( HashType hashType, double time, MurmurHash &h ) const
{
	h.append( typeId() );
}

void SceneInterface::pathToString( const SceneInterface::Path &p, std::string &path )
{
	if ( !p.size() )
	{
		path = "/";
		return;
	}
	size_t totalLength = 0;
	for ( SceneInterface::Path::const_iterator it = p.begin(); it != p.end(); it++ )
	{
		totalLength += it->value().size();
	}
	totalLength += p.size();

	path.resize( totalLength );
	std::string::iterator sit = path.begin();
	for ( SceneInterface::Path::const_iterator it = p.begin(); it != p.end(); it++ )
	{
		*sit++ = '/';
		const std::string &str = it->value();
		path.replace( sit, sit + str.size(), str );
		sit += str.size();
	}
}

void SceneInterface::stringToPath( const std::string &path, SceneInterface::Path &p )
{
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer tokens(path, boost::char_separator<char>("/"));
	Tokenizer::iterator t = tokens.begin();
	p.clear();
	for ( ; t != tokens.end(); t++ )
	{
		p.push_back( *t );
	}
}


