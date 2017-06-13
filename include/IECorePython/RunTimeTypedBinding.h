//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#ifndef IECOREPYTHON_RUNTIMETYPEDBINDING_H
#define IECOREPYTHON_RUNTIMETYPEDBINDING_H

#include "IECore/RunTimeTyped.h"

#include "IECorePython/Export.h"
#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILLock.h"

namespace IECorePython
{

IECOREPYTHON_API void bindRunTimeTyped();

/// A class for wrapping RunTimeTyped objects to allow overriding
/// in Python. It automatically forwards all RunTimeTyped virtual
/// functions to Python overrides if they exist.
template<typename T>
class RunTimeTypedWrapper : public RefCountedWrapper<T>
{

	public :
	
		RunTimeTypedWrapper( PyObject *self );
	
		template<typename Arg1>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1 );
	
		template<typename Arg1, typename Arg2>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2 );
		
		template<typename Arg1, typename Arg2, typename Arg3>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3 );

		/// \todo Once we require c++11, replace all this with a single variadic template.
		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9 );

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
		RunTimeTypedWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10 );

		virtual IECore::TypeId typeId() const;
		virtual const char *typeName() const;
		virtual bool isInstanceOf( IECore::TypeId typeId ) const;
		virtual bool isInstanceOf( const char *typeName ) const;

};

/// A class to simplify the binding of RunTimeTyped derived classes. This should be used
/// in place of the usual boost::python::class_. It automatically makes sure the class is bound
/// with the correct name and base class, as well as dealing with all the issues that RefCountedClass
/// fixes.
template<typename T, typename TWrapper=T>
class RunTimeTypedClass : public RefCountedClass<T, typename T::BaseClass, TWrapper>
{

	public :

		typedef RefCountedClass<T, typename T::BaseClass, TWrapper> BaseClass;

		RunTimeTypedClass( const char *docString = 0 );

};

} // namespace IECorePython

#include "IECorePython/RunTimeTypedBinding.inl"

#endif // IECOREPYTHON_RUNTIMETYPEDBINDING_H
