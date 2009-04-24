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

#include "boost/python.hpp"

#include <sstream>

#include "IECore/CompoundObject.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

#include <exception>

using std::string;
using namespace boost;
using namespace boost::python;

namespace IECore
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
		throw std::out_of_range( "Bad index" );
	}
	return it->second;
}

static void setItem( CompoundObject &o, const std::string &n, Object &v )
{
	o.members()[n] = &v;
}

static ObjectPtr getAttr( const CompoundObject &o, const char *n )
{
	if( PyErr_WarnEx( PyExc_DeprecationWarning, "Access to CompoundObject children as attributes is deprecated - please use item style access instead.", 1 ) )
	{
		// warning converted to exception;
		throw error_already_set();
	}
	return getItem( o, n );
}

static void setAttr( CompoundObject &o, const std::string &n, Object &v )
{
	if( PyErr_WarnEx( PyExc_DeprecationWarning, "Access to CompoundObject children as attributes is deprecated - please use item style access instead.", 1 ) )
	{
		// warning converted to exception;
		throw error_already_set();
	}
	setItem( o, n, v );
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

			handle<> h( obj_ptr );
			dict d( h );

			void* storage = (( converter::rvalue_from_python_storage<CompoundObjectPtr>* ) data )->storage.bytes;
			new( storage ) CompoundObjectPtr( compoundObjectFromDict( d ) );
			data->convertible = storage;
		}
	
		static CompoundObjectPtr compoundObjectFromDict( const dict &v )
		{
			CompoundObjectPtr x = new CompoundObject();
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
		setItem( x, it->first, *it->second );
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

void bindCompoundObject()
{
	
	RunTimeTypedClass<CompoundObject>()
		.def( init<>() )
		.def("__init__", make_constructor(&copyConstructor), "Copy constructor.")
		.def( "__repr__", &repr )
		.def( "__len__", &len )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		/// \todo Remove attribute access in major version 5.
		.def( "__getattr__", &getAttr )
		.def( "__setattr__", &setAttr )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "has_key", &has_key )
		.def( "keys", &keys )
		.def( "values", &values )
		.def( "update", &update )
	;
	
	CompoundObjectFromPythonDict();
	
}

} // namespace IECore
