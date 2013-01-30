//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/tokenizer.hpp"
#include "IECore/SceneInterface.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( SceneInterface )

const SceneInterface::Name &SceneInterface::rootName = IndexedIO::rootName;
const SceneInterface::Path &SceneInterface::rootPath = IndexedIO::rootPath;

SceneInterface::~SceneInterface()
{
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
