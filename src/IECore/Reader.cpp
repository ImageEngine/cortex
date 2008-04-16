//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Reader.h"
#include "IECore/FileNameParameter.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/filesystem/convenience.hpp"

#include <iostream>

using namespace std;
using namespace IECore;
using namespace boost;
using namespace boost::filesystem;

Reader::Reader(  const std::string &name, const std::string &description, ParameterPtr resultParameter )
	: 	Op( name, description, resultParameter ? resultParameter : new Parameter( "result", "The loaded object.", new NullObject ) )
{
	m_fileNameParameter = new FileNameParameter( "fileName", "The name of the file to be loaded", "", "", false, PathParameter::MustExist );
	parameters()->addParameter( m_fileNameParameter );
}

CompoundObjectPtr Reader::readHeader()
{
	CompoundObjectPtr header = new CompoundObject();
	
	return header;
}

ObjectPtr Reader::read()
{
	/// \todo Perhaps we should append the fileName() to any exceptions thrown by operate() before re-raising them?
	return operate();
}

const std::string &Reader::fileName() const
{
	return m_fileNameParameter->getTypedValue();
}

ReaderPtr Reader::create( const std::string &fileName )
{
	bool knownExtension = false;
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );
	string ext = extension(boost::filesystem::path(fileName, boost::filesystem::native));
	if( ext!="" )
	{
		ExtensionsToFnsMap::const_iterator it = m->find( ext );
		if( it!=m->end() )
		{
			knownExtension = true;
			if( it->second.canRead( fileName ) )
			{
				return it->second.creator( fileName );
			}
		}
	}

	// failed to find a reader based on extension. try all canRead functions
	// as a last ditch attempt.
	for( ExtensionsToFnsMap::const_iterator it=m->begin(); it!=m->end(); it++ )
	{
		if( it->second.canRead( fileName ) )
		{
			return( it->second.creator( fileName ) );
		}
	}
	if ( knownExtension )
	{
		throw Exception( string( "Unable to load file '" ) + fileName + "'!" );
	}
	else
	{
		throw Exception( string( "Unrecognized input file format '") + ext + "'!" );
	}
}

void Reader::supportedExtensions( std::vector<std::string> &extensions )
{
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );	
	for( ExtensionsToFnsMap::const_iterator it=m->begin(); it!=m->end(); it++ )
	{
		extensions.push_back( it->first.substr( 1 ) );
	}
}

void Reader::registerReader( const std::string &extensions, CanReadFn canRead, CreatorFn creator )
{
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );	
	vector<string> splitExt;
	split( splitExt, extensions, is_any_of( " " ) );
	ReaderFns r;
	r.creator = creator;
	r.canRead = canRead;
	for( vector<string>::const_iterator it=splitExt.begin(); it!=splitExt.end(); it++ )
	{
		m->insert( ExtensionsToFnsMap::value_type( "." + *it, r ) );
	}
}

Reader::ExtensionsToFnsMap *Reader::extensionsToFns()
{
	static ExtensionsToFnsMap *m = new ExtensionsToFnsMap;
	return m;
}
