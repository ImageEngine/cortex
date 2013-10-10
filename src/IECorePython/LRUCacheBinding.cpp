//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/ScopedGILRelease.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct LRUCacheGetter
{
	LRUCacheGetter( object g )
		:	getter( g )
	{
	}
	
	object operator() ( object key, LRUCache<object, object>::Cost &cost )
	{
		tuple t = extract<tuple>( getter( key ) );
		cost = extract<LRUCache<object, object>::Cost>( t[1] );
		return t[0];
	}
	
	object getter;
};

class PythonLRUCache : public LRUCache<object, object>
{
	
	public :
	
		PythonLRUCache( object getter, LRUCache<object, object>::Cost maxCost )
			:	LRUCache<object, object>( LRUCacheGetter( getter ), maxCost )
		{
		}
		
		PythonLRUCache( object getter, object removalCallback, LRUCache<object, object>::Cost maxCost )
			:	LRUCache<object, object>( LRUCacheGetter( getter ), removalCallback, maxCost )
		{
		}
		
		object get( const object &key )
		{
			// we must hold the GIL when entering LRUCache<object, object>::get()
			// or any other of our base class' methods, because they manipulate
			// boost::python::objects, which in turn use the python api. in addition,
			// LRUCacheGetter enters python, giving us another reason to need the
			// GIL. things are complicated slightly by the fact that the python code
			// executed by LRUCacheGetter may release the GIL, either explicitly or
			// because the interpreter does that from time to time anyway. this can
			// allow another thread to make a call to get(), potentially with the same
			// key as was passed to the current call. this would lead to deadlock -
			// the first call doing the caching waits for the GIL to continue,
			// and the second call holds the GIL and waits for the caching to be
			// complete.
			//
			// the code below avoids this, by ensuring only one thread can be in
			// LRUCache<object, object>::get(), at any given time, and by releasing
			// the GIL while it waits for m_getMutex. this allows the first thread
			// to finish caching (because it can reacquire the GIL), at which point
			// it will release m_getMutex, allowing the second thread to go about
			// its business.
			//
			// while this serialisation may seem inefficient, it's less of a big
			// deal because all python execution is serialised anyway.
			//
			// see test/IECore/LRUCache.py, in particular testYieldGILInGetter().
			
			Mutex::scoped_lock lock;
			{
				ScopedGILRelease gilRelease;
				lock.acquire( m_getMutex );
			}
			return LRUCache<object, object>::get( key );
		}

	private :
	
		Mutex m_getMutex;

};

void bindLRUCache()
{
	
	class_<PythonLRUCache, boost::noncopyable>( "LRUCache", no_init )
		.def( init<object, PythonLRUCache::Cost>( ( boost::python::arg_( "getter" ), boost::python::arg_( "maxCost" )=500  ) ) )
		.def( init<object, object, PythonLRUCache::Cost>( ( boost::python::arg_( "getter" ), boost::python::arg_( "removalCallback" ), boost::python::arg_( "maxCost" )  ) ) )
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
