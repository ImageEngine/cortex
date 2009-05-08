//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include <string>

#include "IECore/IndexedIO.h"
#include "IECore/AttributeCache.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/RefCountedBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECore
{

struct AttributeCacheHelper
{
	typedef std::vector<AttributeCache::HeaderHandle> HeaderHandleVector;
	typedef std::vector<AttributeCache::ObjectHandle> ObjectHandleVector;
	typedef std::vector<AttributeCache::AttributeHandle> AttributeHandleVector;

	static list objects(AttributeCachePtr cache)
	{
		list objects;

		ObjectHandleVector o;
		cache->objects(o);
		for (ObjectHandleVector::const_iterator it = o.begin(); it != o.end(); ++it)
		{
			objects.append<std::string>(*it);
		}


		return objects;
	}

	static list headers(AttributeCachePtr cache)
	{
		list headers;

		HeaderHandleVector o;
		cache->headers(o);
		for (HeaderHandleVector::const_iterator it = o.begin(); it != o.end(); ++it)
		{
			headers.append<std::string>(*it);
		}

		return headers;
	}

	static list attributes(AttributeCachePtr cache, const AttributeCache::ObjectHandle &obj, object regex)
	{
		list attributes;

		AttributeHandleVector a;
		if ( regex != object() )
		{
			extract< std::string >elem( regex );
			if ( elem.check() )
			{
				cache->attributes(obj, elem(), a);
			}
			else
			{
			   	PyErr_SetString(PyExc_TypeError, "Regex parameter must be a string or None.");
			   	throw_error_already_set();
			}
		}
		else
		{
			cache->attributes(obj, a);
		}
		for (AttributeHandleVector::const_iterator it = a.begin(); it != a.end(); ++it)
		{
			attributes.append<std::string>(*it);
		}

		return attributes;
	}

};

void bindAttributeCache()
{
	bool (AttributeCache::*containsObj)(const AttributeCache::ObjectHandle &) = &AttributeCache::contains;
	bool (AttributeCache::*containsObjAttr)(const AttributeCache::ObjectHandle &, const AttributeCache::AttributeHandle &) = &AttributeCache::contains;

	RefCountedClass<AttributeCache, RefCounted>( "AttributeCache" )
		.def( init<const std::string &, IndexedIO::OpenMode>() )
		.def("write", &AttributeCache::write)
		.def("writeHeader", &AttributeCache::writeHeader)
		.def("read", (ObjectPtr (AttributeCache::*)( const AttributeCache::ObjectHandle &, const AttributeCache::AttributeHandle & ) )&AttributeCache::read)
		.def("read", (CompoundObjectPtr (AttributeCache::*)( const AttributeCache::ObjectHandle & ))&AttributeCache::read)
		.def("readHeader", (ObjectPtr (AttributeCache::*) ( const AttributeCache::HeaderHandle & ))&AttributeCache::readHeader)
		.def("readHeader", (CompoundObjectPtr (AttributeCache::*)())&AttributeCache::readHeader)
		.def("contains", containsObj)
		.def("contains", containsObjAttr)
		.def("objects", &AttributeCacheHelper::objects)
		.def("headers", &AttributeCacheHelper::headers)
		.def("attributes", make_function( &AttributeCacheHelper::attributes, default_call_policies(), ( boost::python::arg_( "obj" ), boost::python::arg_( "regex" ) = object() ) ) )
		.def("remove", (void (AttributeCache::*)( const AttributeCache::ObjectHandle &, const AttributeCache::AttributeHandle & ) )&AttributeCache::remove)
		.def("remove", (void (AttributeCache::*)( const AttributeCache::ObjectHandle & ))&AttributeCache::remove)
		.def("removeHeader", &AttributeCache::removeHeader )
	;
}
}
