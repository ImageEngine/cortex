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

#include <sstream>

#include "IECore/CompoundObject.h"
#include "IECorePython/CompoundObjectBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include <exception>

using std::string;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

static std::string repr( CompoundObject &o )
{
	std::stringstream s;

	s << "IECore." << o.typeName() << "(";

	bool added = false;
	for (
		CompoundObject::ObjectMap::const_iterator it = o.members().begin(); it != o.members().end(); ++it )
	{
		const std::string &key = it->first;

		object item( it->second );

		if ( item.attr( "__repr__" ) != object() )
		{
			std::string v = call_method< std::string >( item.ptr(), "__repr__" );

			if ( !added )
			{
				added = true;
				s << "{";
			}
			else
			{
				s << ",";
			}

			s << "'";
			s << key;
			s << "':";
			s << v;
		}
	}

	if ( added )
	{
		s << "}";
	}

	s << ")";

	return s.str();
}

static unsigned int len( const CompoundObject &o )
{
	return o.members().size();
}

static ObjectPtr getItem( const CompoundObject &o, const char *n )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		PyErr_SetString( PyExc_KeyError, n );
		throw_error_already_set();
	}
	return it->second;
}

static void setItem( CompoundObject &o, const char *n, Object &v )
{
	o.members()[n] = &v;
}

static void delItem( CompoundObject &o, const char *n )
{
	CompoundObject::ObjectMap::iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		PyErr_SetString( PyExc_KeyError, n );
		throw_error_already_set();
	}
	o.members().erase( it );
}

static bool contains( const CompoundObject &o, const char *n )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	if( it==o.members().end() )
	{
		return false;
	}
	return true;
}

static bool has_key( const CompoundObject &o, const char *n )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( n );
	return ( it != o.members().end() );
}

static boost::python::list keys( const CompoundObject &o )
{
	boost::python::list result;
	CompoundObject::ObjectMap::const_iterator it;
	for( it = o.members().begin(); it!=o.members().end(); it++ )
	{
		result.append( it->first.value() );
	}
	return result;
}

static boost::python::list values( const CompoundObject &o )
{
	boost::python::list result;
	CompoundObject::ObjectMap::const_iterator it;
	for( it = o.members().begin(); it!=o.members().end(); it++ )
	{
		result.append( it->second );
	}
	return result;
}

static boost::python::list items( const CompoundObject &o )
{
	boost::python::list result;
	CompoundObject::ObjectMap::const_iterator it;
	for( it = o.members().begin(); it!=o.members().end(); it++ )
	{
		result.append( boost::python::make_tuple( it->first.value(), it->second ) );
	}
	return result;
}
		
class CompoundObjectFromPythonDict
{
	public :

		CompoundObjectFromPythonDict()
		{
			converter::registry::push_back(
				&convertible,
				&construct,
				type_id<CompoundObjectPtr> ()
			);
		}

	private :

		static void *convertible( PyObject *obj_ptr )
		{
			if ( !PyDict_Check( obj_ptr ) )
			{
				return 0;
			}
			return obj_ptr;
		}

		static void construct(
	        	PyObject *obj_ptr,
	        	converter::rvalue_from_python_stage1_data *data )
		{
			assert( obj_ptr );
			assert( PyDict_Check( obj_ptr ) );

			handle<> h( borrowed(obj_ptr) );
			dict d( h );

			void* storage = (( converter::rvalue_from_python_storage<CompoundObjectPtr>* ) data )->storage.bytes;
			new( storage ) CompoundObjectPtr( compoundObjectFromDict( d ) );
			data->convertible = storage;
		}

		static CompoundObjectPtr compoundObjectFromDict( const dict &v )
		{
			CompoundObjectPtr x = new CompoundObject();
			list keys = v.keys();
			int len = boost::python::len(v);
			object key,value;
			for (int i = 0; i < len; i++)
			{
				key = keys[i];
				value = v[key];
				extract<const char *> keyElem(key);
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
					CompoundObjectPtr sub = compoundObjectFromDict( dictValueE() );
					setItem( *x, keyElem(), *sub );
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
};

/// binding for update method
static void update( CompoundObject &x, ConstCompoundObjectPtr y )
{
	assert( y );
	CompoundObject::ObjectMap::const_iterator it = y->members().begin();

	for (; it != y->members().end(); it++)
	{
		setItem( x, it->first.value().c_str(), *it->second );
	}
}

/// copy constructor
static CompoundObjectPtr copyConstructor( ConstCompoundObjectPtr other )
{
	assert( other );
	CompoundObjectPtr r = new CompoundObject();
	update( *r, other );
	return r;
}

/// binding for get method
static ObjectPtr get( const CompoundObject &o, const char *key, ObjectPtr defaultValue )
{
	CompoundObject::ObjectMap::const_iterator it = o.members().find( key );
	if ( it == o.members().end() )
	{
		return defaultValue;
	}
	// return the value from the CompoundObject
	return it->second;
}

static CompoundObjectPtr defaultInstance()
{
	return CompoundObject::defaultInstance();
}

void bindCompoundObject()
{

	RunTimeTypedClass<CompoundObject>()
		.def( init<>() )
		.def("__init__", make_constructor(&copyConstructor), "Copy constructor.")
		.def( "__repr__", &repr )
		.def( "__len__", &len )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "has_key", &has_key )
		.def( "keys", &keys )
		.def( "values", &values )
		.def( "items", &items )
		.def( "update", &update )
		.def( "get", &get, "m.get(k [, v])\nReturns m[k] if found; otherwise, returns v.",
			(
				boost::python::arg( "self" ),
				boost::python::arg( "key" ),
				boost::python::arg( "defaultValue" ) = object()
			)
		)
		.def( "defaultInstance", &defaultInstance ).staticmethod( "defaultInstance" )
	;

	CompoundObjectFromPythonDict();

}

} // namespace IECorePython
