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

#include <boost/python.hpp>

#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

#include <exception>

using std::string;
using namespace boost;
using namespace boost::python;

namespace IECore {

static unsigned int len( const CompoundObject &o )
{
	return o.members().size();
}

static ObjectPtr getItem( const CompoundObject &o, const std::string &n )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	return it->second;
}

static void setItem( CompoundObject &o, const std::string &n, Object &v )
{
	o.members()[n] = &v;
}

static void delItem( CompoundObject &o, const std::string &n )
{
	CompoundObject::ObjectMap::iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	o.members().erase( it );
}

static bool contains( const CompoundObject &o, const std::string &n )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		return false;
	}
	return true;
}

static bool has_key( const CompoundObject &o, const std::string n)
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	return ( it != o.members().end() );
}

static boost::python::list keys( CompoundObject &o )
{
	boost::python::list result;
	CompoundObject::ObjectMap::const_iterator it;
	for( it = o.members().begin(); it!=o.members().end(); it++ )
	{
		result.append( it->first );
	}
	return result;
}

static boost::python::list values( CompoundObject &o )
{
	boost::python::list result;
	CompoundObject::ObjectMap::const_iterator it;
	for( it = o.members().begin(); it!=o.members().end(); it++ )
	{
		result.append( it->second );
	}
	return result;
}

static CompoundObjectPtr compoundObjectFromDict( dict v )
{
	CompoundObjectPtr x = new CompoundObject;
	list values = v.values();
	list keys = v.keys();
		
	for (int i = 0; i < keys.attr("__len__")(); i++)
	{
		object key(keys[i]);
		object value(values[i]);
		extract< const std::string > keyElem(key);
		if (!keyElem.check()) 
		{
			PyErr_SetString(PyExc_TypeError, "Incompatible key type. Only strings accepted.");
			throw_error_already_set();
		}
		
		extract< Object& > valueElem(value);
		if (valueElem.check())
		{
			setItem( *x, keyElem(), valueElem() );
			continue;
		}
		extract<dict> dictValueE( value );
		if( dictValueE.check() )
		{
			ObjectPtr co = compoundObjectFromDict( dictValueE() );
			setItem( *x, keyElem(), *co );
			continue;
		}
		else 
		{
			PyErr_SetString(PyExc_TypeError, "Incompatible value type - must be Object or dict.");
			throw_error_already_set();
		}
	}
	return x;
}

/// binding for update method
static void
update1(CompoundObject &x, CompoundObject &y)
{
	CompoundObject::ObjectMap::const_iterator it = y.members().begin();

	for (; it != y.members().end(); it++) 
	{
		setItem( x, it->first, *it->second );
	}
}
	
/// binding for update method
static void
update2(CompoundObject &x, dict v)
{
	CompoundObjectPtr vv = compoundObjectFromDict( v );
	update1( x, *vv );
}

/// constructor that receives a python map object
static CompoundObjectPtr 
compoundObjectConstructor(dict v) 
{
	return compoundObjectFromDict( v );
}

void bindCompoundObject()
{
	typedef class_< CompoundObject , CompoundObjectPtr, boost::noncopyable, bases<Object> > CompoundObjectPyClass;

	CompoundObjectPyClass ( "CompoundObject" )
		.def("__init__", make_constructor(&compoundObjectConstructor), "Copy constructor that accepts a python dict containing Object instances.")
		.def( "__len__", &len )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		.def( "__getattr__", &getItem )
		.def( "__setattr__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "has_key", &has_key )
		.def( "keys", &keys )
		.def( "values", &values )
		.def( "update", &update1 )
		.def( "update", &update2 )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( CompoundObject )
	;

	INTRUSIVE_PTR_PATCH( CompoundObject, CompoundObjectPyClass );

	implicitly_convertible<CompoundObjectPtr, ObjectPtr>();
}

} // namespace IECore
