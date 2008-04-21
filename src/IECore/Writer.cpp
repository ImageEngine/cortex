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

#include <cassert>

#include "IECore/Writer.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/filesystem/convenience.hpp"

using namespace std;
using namespace IECore;
using namespace boost;
using namespace boost::filesystem;

Writer::Writer( const std::string &name, const std::string &description, TypeId writableType )
	:	Op( name, description, new ObjectParameter( "output", "A pass through of the input object", new NullObject, writableType ) )
{
	constructParameters();
}

Writer::Writer( const std::string &name, const std::string &description, const ObjectParameter::TypeIdSet &writableTypes )
	:	Op( name, description, new ObjectParameter( "output", "A pass through of the input object", new NullObject, writableTypes ) )
{
	constructParameters();
}

void Writer::constructParameters( void )
{
	m_objectParameter = new ObjectParameter( "object", "The object to be written", new NullObject, static_pointer_cast<const ObjectParameter>(resultParameter())->validTypes() );
	parameters()->addParameter( m_objectParameter );
	m_fileNameParameter = new FileNameParameter( "fileName", "The filename to be written to.", "", "", false );
	parameters()->addParameter( m_fileNameParameter );
}

const std::string &Writer::fileName() const
{
	return m_fileNameParameter->getTypedValue();
}

ConstObjectPtr Writer::object() const
{
	return m_objectParameter->getValue();
}

void Writer::write()
{
	/// \todo Perhaps we should append the fileName() to any exceptions thrown by operate() before re-raising them?
	operate();
}

ObjectPtr Writer::doOperation( ConstCompoundObjectPtr operands )
{
	doWrite();
	return m_objectParameter->getValue();
}

void Writer::registerWriter( const std::string &extensions, CanWriteFn canWrite, CreatorFn creator )
{
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );
	vector<string> splitExt;
	split( splitExt, extensions, is_any_of( " " ) );
	WriterFns w; 
	w.creator = creator; 
	w.canWrite = canWrite;
	for( vector<string>::const_iterator it=splitExt.begin(); it!=splitExt.end(); it++ )
	{
		m->insert( ExtensionsToFnsMap::value_type( "." + *it, w ) );
	}
}

WriterPtr Writer::create( ObjectPtr object, const std::string &fileName )
{
	string ext = extension(boost::filesystem::path(fileName));
	
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );
	ExtensionsToFnsMap::const_iterator it = m->find( ext );

	if ( it == m->end() )
	{
		throw Exception( string( "Unrecognized output file format '") + ext + "'!" );
	}

	for( it=m->begin(); it!=m->end(); it++ )
	{
		if( it->first==ext )
		{
			if( it->second.canWrite( object, fileName ) )
			{
				return it->second.creator( object, fileName );
			}				
		}
	}

	throw Exception( string( "Unable to find writer able to write given object to file of type '") + ext + "'!" );
}

void Writer::supportedExtensions( std::vector<std::string> &extensions )
{
	ExtensionsToFnsMap *m = extensionsToFns();
	assert( m );	
	for( ExtensionsToFnsMap::const_iterator it=m->begin(); it!=m->end(); it++ )
	{
		extensions.push_back( it->first.substr( 1 ) );
	}
}

Writer::ExtensionsToFnsMap *Writer::extensionsToFns()
{
	static ExtensionsToFnsMap *m = new ExtensionsToFnsMap;
	return m;
}
