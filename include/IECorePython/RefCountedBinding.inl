//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREBINDINGS_REFCOUNTEDBINDING_INL
#define IECOREBINDINGS_REFCOUNTEDBINDING_INL

#include "IECorePython/WrapperGarbageCollector.h"
#include "IECorePython/ScopedGILRelease.h"

namespace IECorePython
{

//////////////////////////////////////////////////////////////////////////
// IntrusivePtr To/From Python
//////////////////////////////////////////////////////////////////////////

namespace Detail
{

// An intrusive pointer that releases the GIL in its
// destructor, so that other threads may acquire it
// while the C++ destructor is running.
template<typename T>
class GILReleasePtr : public IECore::IntrusivePtr<T>
{
	
	public :
	
		typedef IECore::IntrusivePtr<T> Base;

		typedef T element_type;
	
		GILReleasePtr( T *p )
			: 	Base( p )
		{
		}
	
		~GILReleasePtr()
		{
			IECorePython::ScopedGILRelease gilRelease;
			Base::reset( NULL );
		}
		
	private :

		// we don't expect boost::python to need any other
		// constructors or assignment operators, so we make
		// them private and unimplemented so we're alerted
		// should our assumptions ever become incorrect.
		template<typename U>
		GILReleasePtr( const U &rhs );

		template<typename U>
		GILReleasePtr &operator=( const U &rhs );

};

template<typename T>
struct IntrusivePtrToPython
{
	// Constructor registers the conversion with boost::python.
	IntrusivePtrToPython()
	{
		boost::python::to_python_converter<typename T::Ptr, IntrusivePtrToPython<T> >();
	}

	static PyObject *convert( typename T::Ptr const &x )
	{
		if (!x)
		{
			Py_INCREF( Py_None );
			return Py_None;
		}

		PyObject* converted = WrapperGarbageCollector::pyObject( x.get() );
		if( converted )
		{
			Py_INCREF( converted );
		}
		else
		{
			using namespace boost::python::objects;

			converted = class_value_wrapper<
				GILReleasePtr<T>,
				make_ptr_instance<
					T,
					pointer_holder<GILReleasePtr<T>, T>
				>
			>::convert( GILReleasePtr<T>( x.get() ) );
		}

		assert(converted);

		return converted;
	}

};

template<typename T>
struct IntrusivePtrFromPython
{

	// Constructor registers the conversion with boost::python.
	IntrusivePtrFromPython()
	{
		boost::python::converter::registry::push_back( &convertible, &construct, boost::python::type_id<typename T::Ptr>() );
	}

	static void *convertible( PyObject *p )
	{
		if( p == Py_None )
		{
			return p;
		}

		return boost::python::converter::get_lvalue_from_python( p, boost::python::converter::registered<T>::converters );
	}
	
	static void construct( PyObject *source, boost::python::converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((boost::python::converter::rvalue_from_python_storage<typename T::Ptr>*)data)->storage.bytes;

		if( data->convertible == source )
		{
			// Py_None case
			new (storage) typename T::Ptr();
		}
		else
		{
			new (storage) typename T::Ptr( static_cast<T*>( data->convertible ) );
		}
		data->convertible = storage;
	}

};

} // namespace Detail

//////////////////////////////////////////////////////////////////////////
// RefCountedWrapper
//////////////////////////////////////////////////////////////////////////

template<typename T>
RefCountedWrapper<T>::RefCountedWrapper( PyObject *self )
	:	T(), WrapperGarbageCollector( self, this, pyType() )
{
}

template<typename T>
template<typename Arg1>
RefCountedWrapper<T>::RefCountedWrapper( PyObject *self, Arg1 arg1 )
	:	T( arg1 ), WrapperGarbageCollector( self, this, pyType() )
{
}

template<typename T>
template<typename Arg1, typename Arg2>
RefCountedWrapper<T>::RefCountedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2 )
	:	T( arg1, arg2 ), WrapperGarbageCollector( self, this, pyType() )
{
}

template<typename T>
template<typename Arg1, typename Arg2, typename Arg3>
RefCountedWrapper<T>::RefCountedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3 )
	:	T( arg1, arg2, arg3 ), WrapperGarbageCollector( self, this, pyType() )
{
}

template<typename T>
RefCountedWrapper<T>::~RefCountedWrapper()
{
}
		
/// You must hold the GIL before calling this method, and should
/// first have used isSubclassed() to check that it is worth trying.
template<typename T>
boost::python::object RefCountedWrapper<T>::methodOverride( const char *name ) const
{
	return WrapperGarbageCollector::methodOverride( name, pyType() );
}

template<typename T>
PyTypeObject *RefCountedWrapper<T>::pyType()
{
	boost::python::converter::registration const &r
		= boost::python::converter::registered<T>::converters;
	return r.get_class_object();
}

//////////////////////////////////////////////////////////////////////////
// RefCountedClass
//////////////////////////////////////////////////////////////////////////

template<typename T, typename Base, typename TWrapper>
RefCountedClass<T, Base, TWrapper>::RefCountedClass( const char *className, const char *docString )
	:	BaseClass( className, docString, boost::python::no_init )
{

	// register smart pointer conversion to python
	Detail::IntrusivePtrToPython<T>();
	// register smart pointer conversion from python
	Detail::IntrusivePtrFromPython<T>();

	// register casts between T and Base
	boost::python::objects::register_dynamic_id<T>();
	boost::mpl::for_each( boost::python::objects::register_base_of<T>(), (typename BaseClass::metadata::bases*)0, (boost::add_pointer<boost::mpl::_>*) 0 );

	// implicit conversions
	boost::python::implicitly_convertible<IECore::IntrusivePtr<T> , IECore::IntrusivePtr<Base> >();
	boost::python::implicitly_convertible<IECore::IntrusivePtr<T> , IECore::IntrusivePtr<const T> >();

}

} // namespace IECorePython

#endif // IECOREBINDINGS_REFCOUNTEDBINDING_INL
