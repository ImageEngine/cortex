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

#include "IECorePython/Export.h"
#include "IECorePython/WrapperGarbageCollector.h"

namespace IECorePython
{

IECOREPYTHON_API void bindRefCounted();

/// A class similar to boost::python::wrapper, but with specialisations
/// making it more suitable for use wrapping RefCounted types. See
/// RunTimeTypedWrapper for a good example of its use.
template<typename T>
class RefCountedWrapper : public T, public WrapperGarbageCollector
{

	public :

		template<typename... Args>
		RefCountedWrapper( PyObject *self, Args&&... args );

		virtual ~RefCountedWrapper();
	
	protected :
				
		/// You must hold the GIL before calling this method. In most cases
		/// you should use isSubclassed() to check that it is worth calling.
		/// However, if the method is pure virtual, and therefor is required
		/// to be overriden in python, there is no need to call isSubclassed().
		boost::python::object methodOverride( const char *name ) const;
      
	private :
		
		// Returns the Python type this class is bound as.
		static PyTypeObject *pyType();

};

namespace Detail
{

template<typename T>
class GILReleasePtr;

};

/// A class to simplify the binding of RefCounted derived classes - this should be used in place of the usual
/// boost::python::class_. It deals with many issues relating to intrusive pointers and object identity.
///
/// - T : the type being bound
/// - Base : the base class of the type being bound
/// - TWrapper : optional Wrapper class derived from RefCountedWrapper<T>.
///   This can be used to allow Python subclasses to override C++ virtual functions.
template<typename T, typename Base, typename TWrapper=T>
class RefCountedClass : public boost::python::class_<T, Detail::GILReleasePtr<TWrapper>, boost::noncopyable, boost::python::bases<Base> >
{

	public :

		typedef boost::python::class_<T, Detail::GILReleasePtr<TWrapper>, boost::noncopyable, boost::python::bases<Base> > BaseClass;

		RefCountedClass( const char *className, const char *docString = 0 );

};

/// A class to simplify the binding of functions returning raw pointers to
/// RefCounted objects, where we must cast to intrusive_ptr to allow python
/// to share ownership with C++. Use as follows :
///
/// def( "f", &functionReturningRawPointer, return_value_policy<CastToIntrusivePtr>() )
struct CastToIntrusivePtr
{
	template <class T>
	struct apply
	{
		typedef boost::python::to_python_value<boost::intrusive_ptr<typename boost::remove_pointer<T>::type> > type;
	};
};

} // namespace IECorePython

#include "IECorePython/RefCountedBinding.inl"

#endif // IECOREPYTHON_REFCOUNTEDBINDING_H
