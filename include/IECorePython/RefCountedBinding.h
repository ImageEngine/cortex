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

#ifndef IECOREPYTHON_REFCOUNTEDBINDING_H
#define IECOREPYTHON_REFCOUNTEDBINDING_H

#include "IECore/IntrusivePtr.h"
#include "IECorePython/WrapperGarbageCollector.h"

namespace IECorePython
{

void bindRefCounted();

/// \todo This class is now just an internal implementation detail of
/// RefCountedClass - remove it from the public API for the next major version.
template<typename T>
struct IntrusivePtrToPython
{
	/// Constructor registers the conversion with boost::python.
	IntrusivePtrToPython();

	static PyObject *convert( typename T::Ptr const &x );

};

/// \todo This class is now just an internal implementation detail of
/// RefCountedClass - remove it from the public API for the next major version.
template<typename T>
struct IntrusivePtrFromPython
{
	/// Constructor registers the conversion with boost::python.
	IntrusivePtrFromPython();

	static void *convertible( PyObject *p );
	static void construct( PyObject *source, boost::python::converter::rvalue_from_python_stage1_data *data );

};

/// A class similar to boost::python::wrapper, but with specialisations
/// making it more suitable for use wrapping RefCounted types. See
/// RunTimeTypedWrapper for a good example of its use.
template<typename T>
class RefCountedWrapper : public T, public WrapperGarbageCollector
{

	public :
	
		RefCountedWrapper( PyObject *self );
		
		template<typename Arg1>
		RefCountedWrapper( PyObject *self, Arg1 arg1 );

		template<typename Arg1, typename Arg2>
		RefCountedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2 );
		
		template<typename Arg1, typename Arg2, typename Arg3>
		RefCountedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3 );

		virtual ~RefCountedWrapper();
	
	protected :
				
		/// You must hold the GIL before calling this method, and should
		/// first have used isSubclassed() to check that it is worth trying.
		boost::python::object methodOverride( const char *name ) const;
      
	private :
		
		// Returns the Python type this class is bound as.
		static PyTypeObject *pyType();

};

/// A class to simplify the binding of RefCounted derived classes - this should be used in place of the usual
/// boost::python::class_. It deals with many issues relating to intrusive pointers and object identity.
template<typename T, typename Base, typename Ptr=IECore::IntrusivePtr<T> >
class RefCountedClass : public boost::python::class_<T, Ptr, boost::noncopyable, boost::python::bases<Base> >
{

	public :

		typedef boost::python::class_<T, Ptr, boost::noncopyable, boost::python::bases<Base> > BaseClass;

		RefCountedClass( const char *className, const char *docString = 0 );

};

} // namespace IECorePython

#include "IECorePython/RefCountedBinding.inl"

#endif // IECOREPYTHON_REFCOUNTEDBINDING_H
