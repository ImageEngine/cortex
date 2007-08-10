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

#ifndef IE_CORE_VECTORTRAITS_H
#define IE_CORE_VECTORTRAITS_H

namespace IECore
{

/// The VectorTraits struct provides a means of using different vector
/// classes within templated code. It provides the basis for the functions
/// in VectorOps.h, which provide common vector operations using the underlying
/// VectorTraits classes. The default implementation works for
/// the ImathVec types out of the box. For any other types you may need to
/// specialise it.
template<typename T>
struct VectorTraits
{
	/// The type of the components of the vector.
	typedef typename T::BaseType BaseType;
	static unsigned int dimensions() { return T::dimensions(); };
	static BaseType get( const T &v, unsigned int i ) { return v[i]; };
	static void set( T &v, unsigned int i, BaseType x ) { v[i] = x; };
};

/// Specialisation for float type to allow its use as a 1d vector
template<>
struct VectorTraits<float>
{
	typedef float BaseType;
	static unsigned int dimensions() { return 1; };
	static float get( const float &v, unsigned int i ) { return v; };
	static void set( float &v, unsigned int i, float x ) { v = x; };
};

/// Specialisation for double type to allow its use as a 1d vector
template<>
struct VectorTraits<double>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 1; };
	static double get( const double &v, unsigned int i ) { return v; };
	static void set( double &v, unsigned int i, double x ) { v = x; };
};

} // namespace IECore

#endif // IE_CORE_VECTORTRAITS_H
