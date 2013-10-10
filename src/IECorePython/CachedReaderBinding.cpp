//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/CachedReader.h"
#include "IECore/Object.h"
#include "IECore/ModifyOp.h"
#include "IECorePython/CachedReaderBinding.h"
#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

static ObjectPtr read( CachedReader &r, const std::string &f )
{
	ScopedGILRelease gilRelease;
	ConstObjectPtr o = r.read( f );
	if( o )
	{
		return o->copy();
	}
	else
	{
		return 0;
	}
}

static ObjectPoolPtr objectPool( CachedReader &r )
{
	return r.objectPool();
}

void bindCachedReader()
{
	RefCountedClass<CachedReader, RefCounted>( "CachedReader" )
		.def( init<const SearchPath &, optional<ObjectPoolPtr> >() )
		.def( init<const SearchPath &, ConstModifyOpPtr, optional<ObjectPoolPtr> >() )
		.def( "read", &read )
		.def( "clear", (void (CachedReader::*)( const std::string &) )&CachedReader::clear )
		.def( "clear", (void (CachedReader::*)( void ) )&CachedReader::clear )
		.def( "insert", &CachedReader::insert )
		.def( "cached", &CachedReader::cached )
		.add_property( "searchPath", make_function( &CachedReader::getSearchPath, return_value_policy<copy_const_reference>() ), &CachedReader::setSearchPath )
		.def( "defaultCachedReader", &CachedReader::defaultCachedReader ).staticmethod( "defaultCachedReader" )
		.def( "objectPool", &objectPool )
	;
}

}
