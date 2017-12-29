//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECorePython/CubicBasisBinding.h"
#include "IECorePython/IECoreBinding.h"
#include "IECore/CubicBasis.h"

using namespace boost::python;
using namespace Imath;
using namespace std;
using namespace IECore;

namespace IECorePython
{

#define REPR_SPECIALISATION( TYPE )																		\
template<>																								\
string repr<TYPE>( TYPE &x )																			\
{																										\
	stringstream s;																						\
	s << "IECore." << #TYPE << "( ";																	\
	s << repr( x.matrix ) << ", " << x.step ;															\
	s << " )";																							\
	return s.str();																						\
}																										\

REPR_SPECIALISATION( CubicBasisf )
REPR_SPECIALISATION( CubicBasisd )

template<typename T>
static boost::python::tuple coefficients( const T &b, typename T::BaseType t )
{
	typename T::BaseType c0, c1, c2, c3;
	b.coefficients( t, c0, c1, c2, c3 );
	return boost::python::make_tuple( c0, c1, c2, c3 );
}

template<typename T>
static boost::python::tuple derivativeCoefficients( const T &b, typename T::BaseType t )
{
	typename T::BaseType c0, c1, c2, c3;
	b.derivativeCoefficients( t, c0, c1, c2, c3 );
	return boost::python::make_tuple( c0, c1, c2, c3 );
}

template<typename T>
static boost::python::tuple integralCoefficients( const T &b, typename T::BaseType t0, typename T::BaseType t1 )
{
	typename T::BaseType c0, c1, c2, c3;
	b.integralCoefficients( t0, t1, c0, c1, c2, c3 );
	return boost::python::make_tuple( c0, c1, c2, c3 );
}

template<typename T>
void bindCubicBasis( const char *name )
{
	typedef typename T::BaseType BaseType;

	class_<T>( name, init<const typename T::MatrixType, unsigned>() )
		.def_readwrite( "matrix", &T::matrix )
		.def_readwrite( "step", &T::step )
		.def( "coefficients", &coefficients<T> )
		.def( "derivativeCoefficients", &derivativeCoefficients<T> )
		.def( "integralCoefficients", &integralCoefficients<T> )
		.def( "__call__", (BaseType (T::*) ( BaseType, BaseType, BaseType, BaseType, BaseType )const)&T::template operator()<BaseType> )
		.def( "__call__", (V2f (T::*) ( float, const V2f &, const V2f &, const V2f &, const V2f &)const)&T::template operator()<V2f> )
		.def( "__call__", (V3f (T::*) ( float, const V3f &, const V3f &, const V3f &, const V3f &)const)&T::template operator()<V3f> )
		.def( "__call__", (V2d (T::*) ( double, const V2d &, const V2d &, const V2d &, const V2d &)const)&T::template operator()<V2d> )
		.def( "__call__", (V3d (T::*) ( double, const V3d &, const V3d &, const V3d &, const V3d &)const)&T::template operator()<V3d> )
		.def( "derivative", (BaseType (T::*) ( BaseType, BaseType, BaseType, BaseType, BaseType )const)&T::template derivative<BaseType> )
		.def( "derivative", (V2f (T::*) ( float, const V2f &, const V2f &, const V2f &, const V2f &)const)&T::template derivative<V2f> )
		.def( "derivative", (V3f (T::*) ( float, const V3f &, const V3f &, const V3f &, const V3f &)const)&T::template derivative<V3f> )
		.def( "derivative", (V2d (T::*) ( double, const V2d &, const V2d &, const V2d &, const V2d &)const)&T::template derivative<V2d> )
		.def( "derivative", (V3d (T::*) ( double, const V3d &, const V3d &, const V3d &, const V3d &)const)&T::template derivative<V3d> )
		.def( "integral", (BaseType (T::*) ( BaseType, BaseType, BaseType, BaseType, BaseType, BaseType )const)&T::template integral<BaseType> )
		.def( "integral", (V2f (T::*) ( float, float, const V2f &, const V2f &, const V2f &, const V2f &)const)&T::template integral<V2f> )
		.def( "integral", (V3f (T::*) ( float, float, const V3f &, const V3f &, const V3f &, const V3f &)const)&T::template integral<V3f> )
		.def( "integral", (V2d (T::*) ( double, double, const V2d &, const V2d &, const V2d &, const V2d &)const)&T::template integral<V2d> )
		.def( "integral", (V3d (T::*) ( double, double, const V3d &, const V3d &, const V3d &, const V3d &)const)&T::template integral<V3d> )
		.def( self==self )
		.def( self!=self )
		.def( "linear", &T::linear, return_value_policy<copy_const_reference>() ).staticmethod( "linear" )
		.def( "bezier", &T::bezier, return_value_policy<copy_const_reference>() ).staticmethod( "bezier" )
		.def( "bSpline", &T::bSpline, return_value_policy<copy_const_reference>() ).staticmethod( "bSpline" )
		.def( "catmullRom", &T::catmullRom, return_value_policy<copy_const_reference>() ).staticmethod( "catmullRom" )
		.def( "__repr__", &repr<T> )
	;
}

void bindCubicBasis()
{
	bindCubicBasis<CubicBasisf>( "CubicBasisf" );
	bindCubicBasis<CubicBasisd>( "CubicBasisd" );
}

}
