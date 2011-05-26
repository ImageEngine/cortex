//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/LRUCache.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

typedef LRUCache<object, object> PythonLRUCache;

struct LRUCacheGetter
{
	LRUCacheGetter( object g )
		:	getter( g )
	{
	}
	
	object operator() ( object key, PythonLRUCache::Cost &cost )
	{
		tuple t = extract<tuple>( getter( key ) );
		cost = extract<PythonLRUCache::Cost>( t[1] );
		return t[0];
	}
	
	object getter;
};

static PythonLRUCache *construct( object getter, PythonLRUCache::Cost maxCost )
{
	return new PythonLRUCache( LRUCacheGetter( getter ), maxCost );
} 

void bindLRUCache()
{
	
	class_<PythonLRUCache, boost::noncopyable>( "LRUCache", no_init )
		.def( "__init__", make_constructor( &construct, default_call_policies(), ( boost::python::arg_( "getter" ), boost::python::arg_( "maxCost" )=500  ) ) )
		.def( "clear", &PythonLRUCache::clear )
		.def( "erase", &PythonLRUCache::erase )
		.def( "setMaxCost", &PythonLRUCache::setMaxCost )
		.def( "getMaxCost", &PythonLRUCache::getMaxCost )
		.def( "currentCost", &PythonLRUCache::currentCost )
		.def( "get", &PythonLRUCache::get )
		.def( "set", &PythonLRUCache::set )
		.def( "cached", &PythonLRUCache::cached )
	;
}

} // namespace IECorePython
