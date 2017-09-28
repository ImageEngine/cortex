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

#include "boost/python.hpp"

#include "IECore/Reader.h"
#include "IECore/Object.h"
#include "IECore/FileNameParameter.h"
#include "IECorePython/ReaderBinding.h"
#include "IECorePython/OpBinding.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

using std::string;
using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

namespace
{

static list supportedExtensions()
{
	list result;
	std::vector<std::string> e;
	Reader::supportedExtensions( e );
	for( unsigned int i=0; i<e.size(); i++ )
	{
		result.append( e[i] );
	}
	return result;
}

static list supportedExtensions( TypeId typeId )
{
	list result;
	std::vector<std::string> e;
	Reader::supportedExtensions( typeId, e );
	for( unsigned int i=0; i<e.size(); i++ )
	{
		result.append( e[i] );
	}
	return result;
}

struct ReaderCreator
{

	ReaderCreator( object fn )
		:	m_fn( fn )
	{
	}

	ReaderPtr operator()( const std::string &fileName )
	{
		ScopedGILLock gilLock;
		ReaderPtr result = extract<ReaderPtr>( m_fn( fileName ) );
		return result;
	}

	private :

		object m_fn;

};

struct ReaderCanRead
{

	ReaderCanRead( object fn )
		:	m_fn( fn )
	{
	}

	bool operator()( const std::string &fileName )
	{
		ScopedGILLock gilLock;
		bool result = m_fn( fileName );
		return result;
	}

	private :

		object m_fn;

};

static void registerReader( const std::string &extensions, object &canRead, object &creator, TypeId typeId )
{
	Reader::registerReader( extensions, ReaderCanRead( canRead ), ReaderCreator( creator ), typeId );
}

} // namespace

namespace IECorePython
{

void bindReader()
{
	using boost::python::arg;

	ReaderClass<Reader, ReaderWrapper<Reader> >()
		.def( init< const std::string &>( ( arg( "description" ) ) ) )
		.def( "create", &Reader::create ).staticmethod( "create" )
		.def( "supportedExtensions", ( list(*)( ) ) &supportedExtensions )
		.def( "supportedExtensions", ( list(*)( IECore::TypeId ) ) &supportedExtensions )
		.staticmethod( "supportedExtensions" )
		.def( "registerReader", &registerReader )
		.staticmethod( "registerReader" )
	;
}

} // namespace IECorePython
