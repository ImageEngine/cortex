//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
#include <boost/python.hpp>

#include <string>

#include <IECore/IndexedIO.h>
#include <IECore/HierarchicalCache.h>
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"

using namespace boost::python;
using namespace IECore;

namespace IECore
{

struct HierarchicalCacheHelper
{
	typedef std::vector<HierarchicalCache::HeaderHandle> HeaderHandleVector;
	typedef std::vector<HierarchicalCache::ObjectHandle> ObjectHandleVector;
	typedef std::vector<HierarchicalCache::AttributeHandle> AttributeHandleVector;	
	
	static list objects(HierarchicalCachePtr cache)
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

	static list headers(HierarchicalCachePtr cache)
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
	
	static list attributes(HierarchicalCachePtr cache, const HierarchicalCache::ObjectHandle &obj, object regex)
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
	
	static list children(HierarchicalCachePtr cache, const HierarchicalCache::ObjectHandle &obj)
	{
		list children;

		ObjectHandleVector a;
		cache->children(obj, a);
		for (ObjectHandleVector::const_iterator it = a.begin(); it != a.end(); ++it)
		{
			children.append<std::string>(*it);
		}
		
		return children;
	}
};

void bindHierarchicalCache()
{
	const char *bindName = "HierarchicalCache";
	
	bool (HierarchicalCache::*containsObj)(const HierarchicalCache::ObjectHandle &) = &HierarchicalCache::contains;
	bool (HierarchicalCache::*containsObjAttr)(const HierarchicalCache::ObjectHandle &, const HierarchicalCache::AttributeHandle &) = &HierarchicalCache::contains;
	
	typedef class_< HierarchicalCache, HierarchicalCachePtr > HierarchicalCachePyClass;
	HierarchicalCachePyClass ( bindName, no_init )
		.def( init<const std::string &, IndexedIO::OpenMode>() )
		.def("write", (void (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle &, const HierarchicalCache::AttributeHandle &, ObjectPtr ))&HierarchicalCache::write)
		.def("writeHeader", &HierarchicalCache::writeHeader)
		.def("read", (ObjectPtr (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle &, const HierarchicalCache::AttributeHandle & ) )&HierarchicalCache::read)
		.def("read", (CompoundObjectPtr (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle & ))&HierarchicalCache::read)
		.def("readHeader", (ObjectPtr (HierarchicalCache::*) ( const HierarchicalCache::HeaderHandle & ))&HierarchicalCache::readHeader)
		.def("readHeader", (CompoundObjectPtr (HierarchicalCache::*)())&HierarchicalCache::readHeader)
		.def("contains", containsObj)
		.def("contains", containsObjAttr)		
		.def("objects", &HierarchicalCacheHelper::objects)
		.def("headers", &HierarchicalCacheHelper::headers)
		.def("attributes", make_function( &HierarchicalCacheHelper::attributes, default_call_policies(), ( boost::python::arg_( "obj" ), boost::python::arg_( "regex" ) = object() ) ) )
		.def("remove", (void (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle &, const HierarchicalCache::AttributeHandle & ) )&HierarchicalCache::remove)
		.def("remove", (void (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle & ))&HierarchicalCache::remove)
		.def("removeHeader", &HierarchicalCache::removeHeader )

		.def("write", (void (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle &, const Imath::M44f &))&HierarchicalCache::write)
		.def("write", (void (HierarchicalCache::*)( const HierarchicalCache::ObjectHandle &, ConstVisibleRenderablePtr ))&HierarchicalCache::write)
		.def("isShape", &HierarchicalCache::isShape )
		.def("isTransform", &HierarchicalCache::isTransform )
		.def("transformMatrix", &HierarchicalCache::transformMatrix )
		.def("shape", &HierarchicalCache::shape )
		.def("globalTransformMatrix", &HierarchicalCache::globalTransformMatrix )
		.def("bound", &HierarchicalCache::bound )
		.def("children", &HierarchicalCacheHelper::children )
		.def("absoluteName", make_function( &HierarchicalCache::absoluteName, default_call_policies(), ( boost::python::arg_( "relativeName" ), boost::python::arg_( "parent" ) = str( HierarchicalCache::rootName() ) ) ) )
		.staticmethod( "absoluteName" )
		.def("relativeName", &HierarchicalCache::relativeName )
		.staticmethod( "relativeName" )
		.def("parentName", &HierarchicalCache::parentName )
		.staticmethod( "parentName" )
		.def("rootName", &HierarchicalCache::rootName )
		.staticmethod( "rootName" )
	;

	INTRUSIVE_PTR_PATCH( HierarchicalCache, HierarchicalCachePyClass );
}
}
