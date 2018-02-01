//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_EXPORT_H
#define IE_CORE_EXPORT_H

// Define platform-specific macros for importing/exporting symbols
#ifdef _MSC_VER
	#define IECORE_IMPORT __declspec(dllimport)
	#define IECORE_EXPORT __declspec(dllexport)
#else
	#define IECORE_IMPORT __attribute__((visibility("default")))
	#define IECORE_EXPORT __attribute__((visibility("default")))
#endif

// When compiling with `-fvisibility=hidden` with GCC or Clang, we run into
// problems when including 3rd party headers that don't define symbol
// visibility. Because they don't explicitly assign default visibility to
// anything, such headers end up inheriting hidden visibility for _everything_.
// This is particularly problematic if we wish to build template classes around
// those 3rd party types, because if they are hidden, GCC will force our
// class to be hidden too, no matter how we declare it. For instance,
// we would be unable to export `TypedData<Imath::V2f>`. We use these macros
// to push/pop default visibility around such problematic includes.
#ifdef __GNUC__
#define IECORE_PUSH_DEFAULT_VISIBILITY _Pragma( "GCC visibility push(default)" )
#define IECORE_POP_DEFAULT_VISIBILITY _Pragma( "GCC visibility pop" )
#else
#define IECORE_PUSH_DEFAULT_VISIBILITY
#define IECORE_POP_DEFAULT_VISIBILITY
#endif

// Define IECORE_API macro based on whether or not we are compiling IECore,
// or including headers for linking to it. The IECORE_API macro is the one that is
// used in the class definitions.
#ifdef IECore_EXPORTS
	#define IECORE_API IECORE_EXPORT
#else
	#define IECORE_API IECORE_IMPORT
#endif

#endif // #ifndef IE_CORE_EXPORT_H
