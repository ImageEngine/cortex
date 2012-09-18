//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MATRIXTRAITS_H
#define IE_CORE_MATRIXTRAITS_H

#include <cassert>

#include "OpenEXR/ImathMatrix.h"

namespace IECore
{

/// The MatrixTraits struct provides a means of using different matrix
/// classes within templated code. It provides the basis for the functions
/// in MatrixOps.h, which provide common matrix operations using the underlying
/// MatrixTraits classes. To add support for a new matrix class simply specialise
/// the struct appropriately for that type.
template<typename T>
struct MatrixTraits
{
	typedef float BaseType; /* should be specialised */
	static unsigned int dimensions() { /* must be specialised */ assert( 0 ); return 0; };
	static BaseType get( const T &v, unsigned int i, unsigned int j ) { /* must be specialised */ assert( 0 ); return 0; };
	static void set( T &v, unsigned int i, unsigned int j, BaseType x ) { /* must be specialised */ assert( 0 ); };
};

/// Specialisations for Imath types
template<>
struct MatrixTraits<Imath::M33f>
{
	typedef float BaseType;
	static unsigned int dimensions() { return 3; };
	static BaseType get( const Imath::M33f &v, unsigned int i, unsigned int j ) { return v[i][j]; };
	static void set( Imath::M33f &v, unsigned int i, unsigned int j, BaseType x ) { v[i][j] = x; };
};

template<>
struct MatrixTraits<Imath::M44f>
{
	typedef float BaseType;
	static unsigned int dimensions() { return 4; };
	static BaseType get( const Imath::M44f &v, unsigned int i, unsigned int j ) { return v[i][j]; };
	static void set( Imath::M44f &v, unsigned int i, unsigned int j, BaseType x ) { v[i][j] = x; };
};

template<>
struct MatrixTraits<Imath::M33d>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 3; };
	static BaseType get( const Imath::M33d &v, unsigned int i, unsigned int j ) { return v[i][j]; };
	static void set( Imath::M33d &v, unsigned int i, unsigned int j, BaseType x ) { v[i][j] = x; };
};

template<>
struct MatrixTraits<Imath::M44d>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 4; };
	static BaseType get( const Imath::M44d &v, unsigned int i, unsigned int j ) { return v[i][j]; };
	static void set( Imath::M44d &v, unsigned int i, unsigned int j, BaseType x ) { v[i][j] = x; };
};

} // namespace IECore

#endif // IE_CORE_MATRIXTRAITS_H
