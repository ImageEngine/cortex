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

#ifndef IE_COREPYTHON_REFCOUNTEDBINDING_H
#define IE_COREPYTHON_REFCOUNTEDBINDING_H

#include "boost/intrusive_ptr.hpp"

namespace IECore
{
void bindRefCounted();

template<typename T>
struct IntrusivePtrToPython
{
	/// Constructor registers the conversion with boost::python.
	IntrusivePtrToPython();

	static PyObject *convert( typename T::Ptr const &x );

};

template<typename T>
struct IntrusivePtrFromPython
{
	/// Constructor registers the conversion with boost::python.
	IntrusivePtrFromPython();
	
	static void *convertible( PyObject *p );
	static void construct( PyObject *source, boost::python::converter::rvalue_from_python_stage1_data *data );		

};

/// A class to simplify the binding of RefCounted derived classes - this should be used in place of the usual
/// boost::python::class_. It deals with many issues relating to intrusive pointers and object identity.
template<typename T, typename Base, typename Ptr=boost::intrusive_ptr<T> >
class RefCountedClass : public boost::python::class_<T, Ptr, boost::noncopyable, boost::python::bases<Base> >
{

	public :
	
		typedef boost::python::class_<T, Ptr, boost::noncopyable, boost::python::bases<Base> > BaseClass;
	
		RefCountedClass( const char *className, const char *docString = 0 );
		
};

} // namespace IECore

#include "IECore/bindings/RefCountedBinding.inl"

#endif // IE_COREPYTHON_REFCOUNTEDBINDING_H
