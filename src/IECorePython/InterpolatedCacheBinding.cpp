//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/InterpolatedCache.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/InterpolatedCacheBinding.h"
#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct InterpolatedCacheHelper
{
	typedef std::vector<InterpolatedCache::HeaderHandle> HeaderHandleVector;
	typedef std::vector<InterpolatedCache::ObjectHandle> ObjectHandleVector;
	typedef std::vector<InterpolatedCache::AttributeHandle> AttributeHandleVector;

	static list objects( ConstInterpolatedCachePtr cache, float frame )
	{
		ObjectHandleVector o;
		{
			ScopedGILRelease gilRelease;
			cache->objects( frame, o );
		}
		
		list objects;
		for (ObjectHandleVector::const_iterator it = o.begin(); it != o.end(); ++it)
		{
			objects.append<std::string>(*it);
		}
		return objects;
	}
	
	static list headers(ConstInterpolatedCachePtr cache, float frame )
	{
		HeaderHandleVector o;
		{
			ScopedGILRelease gilRelease;
			cache->headers( frame, o );
		}
		
		list headers;
		for (HeaderHandleVector::const_iterator it = o.begin(); it != o.end(); ++it)
		{
			headers.append<std::string>(*it);
		}

		return headers;
	}
	
	static list attributes( ConstInterpolatedCachePtr cache, float frame, const InterpolatedCache::ObjectHandle &obj, object regex )
	{
		list attributes;

		AttributeHandleVector a;
		if ( regex != object() )
		{
			extract< std::string >elem( regex );
			if ( elem.check() )
			{
				cache->attributes( frame, obj, elem(), a );
			}
			else
			{
			   	PyErr_SetString(PyExc_TypeError, "Regex parameter must be a string or None.");
			   	throw_error_already_set();
			}
		}
		else
		{
			cache->attributes( frame, obj, a );
		}
		for (AttributeHandleVector::const_iterator it = a.begin(); it != a.end(); ++it)
		{
			attributes.append<std::string>(*it);
		}

		return attributes;
	}
		
	static ObjectPtr read( InterpolatedCachePtr cache, float frame, const InterpolatedCache::ObjectHandle &obj, const InterpolatedCache::AttributeHandle &attr )
	{
		ScopedGILRelease gilRelease;
		return cache->read( frame, obj, attr );
	}
	
	static ObjectPtr read2( InterpolatedCachePtr cache, float frame, const InterpolatedCache::ObjectHandle &obj )
	{
		ScopedGILRelease gilRelease;
		return cache->read( frame, obj );
	}
		
	static ObjectPtr readHeader( InterpolatedCachePtr cache, float frame, const InterpolatedCache::HeaderHandle &hdr )
	{
		ScopedGILRelease gilRelease;
		return cache->readHeader( frame, hdr );
	}
	
	static CompoundObjectPtr readHeader2( InterpolatedCachePtr cache, float frame )
	{
		ScopedGILRelease gilRelease;
		return cache->readHeader( frame );
	}
		
	static bool contains( InterpolatedCachePtr cache, float frame, const InterpolatedCache::ObjectHandle &obj )
	{
		ScopedGILRelease gilRelease;
		return cache->contains( frame, obj );
	}
	
	static bool contains2( InterpolatedCachePtr cache, float frame, const InterpolatedCache::ObjectHandle &obj, const InterpolatedCache::AttributeHandle &attr )
	{
		ScopedGILRelease gilRelease;
		return cache->contains( frame, obj, attr );
	}

};

void bindInterpolatedCache()
{
	RefCountedClass<InterpolatedCache, RefCounted> interpolatedCacheClass( "InterpolatedCache" );
	{
		// define enum before functions.
		scope varScope = interpolatedCacheClass;
		enum_<InterpolatedCache::Interpolation>( "Interpolation" )
			.value( "None", InterpolatedCache::None )
			.value( "Linear", InterpolatedCache::Linear )
			.value( "Cubic", InterpolatedCache::Cubic )
		;
	}
	interpolatedCacheClass
		.def(
			init<const std::string &, InterpolatedCache::Interpolation, const OversamplesCalculator &, size_t>
			(
				(
					arg( "pathTemplate" ) = std::string(""),
					arg( "interpolation" ) = InterpolatedCache::None,
					arg( "oversamplesCalculator" ) = OversamplesCalculator(),
					arg( "maxOpenFiles" ) = 10
				)
			)
		)
		.def("setPathTemplate", &InterpolatedCache::setPathTemplate )
		.def("getPathTemplate", &InterpolatedCache::getPathTemplate, return_value_policy<copy_const_reference>() )
		.def("setMaxOpenFiles", &InterpolatedCache::setMaxOpenFiles )
		.def("getMaxOpenFiles", &InterpolatedCache::getMaxOpenFiles )
		.def("setInterpolation", &InterpolatedCache::setInterpolation )
		.def("getInterpolation", &InterpolatedCache::getInterpolation )
		.def("setOversamplesCalculator", &InterpolatedCache::setOversamplesCalculator )
		.def("getOversamplesCalculator", &InterpolatedCache::getOversamplesCalculator, return_value_policy<copy_const_reference>() )
		.def("read", &InterpolatedCacheHelper::read )
		.def("read", &InterpolatedCacheHelper::read2 )
		.def("readHeader", &InterpolatedCacheHelper::readHeader )
		.def("readHeader", &InterpolatedCacheHelper::readHeader2 )
		.def("contains", &InterpolatedCacheHelper::contains )
		.def("contains", &InterpolatedCacheHelper::contains2 )
		.def("objects", &InterpolatedCacheHelper::objects)
		.def("headers", &InterpolatedCacheHelper::headers)
		.def("attributes", make_function( &InterpolatedCacheHelper::attributes  , default_call_policies(), ( boost::python::arg_( "obj" ), boost::python::arg_( "regex" ) = object() ) ) )
	;
}

} // namespace IECorePython
