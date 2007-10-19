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

// This include needs to be the very first to prevent problems with warnings 
// regarding redefinition of _POSIX_C_SOURCE
#include <boost/python.hpp>

// System includes
#include <string>
#include <stdexcept>

#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathMatrixAlgo.h>

#include "IECore/MatrixAlgo.h"
#include "IECore/bindings/ImathMatrixBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace std;

// Binding implementations
namespace IECore 
{

template<typename T>
void bindMatrix33(const char *bindName);

template<typename T>
void bindMatrix44(const char *bindName);

void bindImathMatrix()
{
	bindMatrix33<float>("M33f");
	bindMatrix33<double>("M33d");

	bindMatrix44<float>("M44f");
	bindMatrix44<double>("M44d");
}	

template<typename T>
struct MatrixBaseType
{	
	typedef float BaseType;
};
		
template<>
struct MatrixBaseType< M33d >
{
	typedef double BaseType;
};
	
template<>
struct MatrixBaseType< M44d >
{
	typedef double BaseType;
};

template <typename T> 
struct MatrixDimensions
{
	static tuple get(const T &x)
	{
		return make_tuple(3, 3);
	}
};

template<typename T>
struct MatrixDimensions< Matrix44<T> >
{
	static tuple get(const Matrix44<T> &x)
	{
		return make_tuple(4, 4);
	}
};

template<typename T>
struct MatrixWrapper
{		
	typedef typename MatrixBaseType<T>::BaseType V;
	static V get(const T &m, tuple i)
	{	
		static tuple dims = MatrixDimensions<T>::get(m);
		
		int x = extract<int>(i[0]);
		int y = extract<int>(i[1]);
		
		if (x < 0 || x >= extract<int>(dims[0]) ||
			y < 0 || y >= extract<int>(dims[1]))
		{
			throw std::out_of_range("");	
		}		
		
		return m[x][y];
	}
	
	static void set(T &m, tuple i, const V &v)
	{
		static tuple dims = MatrixDimensions<T>::get(m);
		
		int x = extract<int>(i[0]);
		int y = extract<int>(i[1]);
		
		if (x < 0 || x >= extract<int>(dims[0]) ||
			y < 0 || y >= extract<int>(dims[1]))
		{
			throw std::out_of_range("");	
		}		
		
		m[x][y] = v;
	}	
};

template<typename M, typename V>
static M createScaled( const V &s )
{
	M m;
	m.scale( s );
	return m;
}

template<typename M, typename V>
static M createTranslated( const V &s )
{
	M m;
	m.translate( s );
	return m;
}

template<typename M, typename V>
static M createRotated( const V &s )
{
	M m;
	m.rotate( s );
	return m;
}

template<typename M, typename V>
static V multVecMatrix( const M &m, const V &v )
{
	V result;
	m.multVecMatrix( v, result );
	return result;
}

template<typename M, typename V>
static V multDirMatrix( const M &m, const V &v )
{
	V result;
	m.multDirMatrix( v, result );
	return result;
}

template<typename M, typename V>
V extractScaling( const M &m )
{
	V s;
	extractScaling( m, s );
	return s;
}

template<typename M>
M sansScaling2( const M &m )
{
	return sansScaling( m, true );
}

template<typename M>
void removeScaling( M &m )
{
	removeScaling( m, true );
}

template<typename T>
tuple extractScalingAndShear33( const Matrix33<T> &m )
{
	Vec2<T> scl;
	T shr;
	extractScalingAndShear( m, scl, shr );
	return make_tuple( scl, shr );
}

template<typename T>
tuple extractScalingAndShear44( const Matrix44<T> &m )
{
	Vec3<T> scl;
	Vec3<T> shr;
	extractScalingAndShear( m, scl, shr );
	return make_tuple( scl, shr );
}

template<typename M>
M sansScalingAndShear( const M &m )
{
	return sansScalingAndShear( m, true );
}

template<typename M>
void removeScalingAndShear( M &m )
{
	removeScalingAndShear( m, true );
}

template<typename T>
tuple extractAndRemoveScalingAndShear33( Matrix33<T> &m )
{
	Vec2<T> scl;
	T shr;
	extractAndRemoveScalingAndShear( m, scl, shr, true );
	return make_tuple( scl, shr );
}

template<typename T>
tuple extractAndRemoveScalingAndShear44( Matrix44<T> &m )
{
	Vec3<T> scl, shr;
	extractAndRemoveScalingAndShear( m, scl, shr, true );
	return make_tuple( scl, shr );
}

template<typename M, typename V>
V extractEulerXYZ( const M &m )
{
	V r;
	extractEulerXYZ( m, r );
	return r;
}

template<typename M, typename V>
V extractEulerZYX( const M &m )
{
	V r;
	extractEulerZYX( m, r );
	return r;
}

template<typename T>
tuple extractSHRT44( const Matrix44<T> &m )
{
	Vec3<T> s, h, r, t;
	extractSHRT( m, s, h, r, t, true );
	return make_tuple( s, h, r, t );
}

template<typename T>
tuple extractSHRT33( const Matrix33<T> &m )
{
	Vec2<T> s, t;
	T h, r;
	extractSHRT( m, s, h, r, t, true );
	return make_tuple( s, h, r, t );
}

#define DEFINEMATRIXSTRSPECIALISATION( TYPE, D )														\
template<>																								\
string repr<TYPE>( TYPE &x )																			\
{																										\
	stringstream s;																						\
	s << #TYPE << "( ";																					\
	for( int i=0; i<D; i++ )																			\
	{																									\
		for( int j=0; j<D; j++ )																		\
		{																								\
			s << x[i][j];																				\
			if( !(i==D-1 && j==D-1) )																	\
			{																							\
				s << ", ";																				\
			}																							\
		}																								\
	}																									\
	s << " )";																							\
	return s.str();																						\
}																										\
																										\
template<>																								\
string str<TYPE>( TYPE &x )																				\
{																										\
	stringstream s;																						\
	for( int i=0; i<D; i++ )																			\
	{																									\
		for( int j=0; j<D; j++ )																		\
		{																								\
			s << x[i][j];																				\
			if( !(i==D-1 && j==D-1) )																	\
			{																							\
				s << " ";																				\
			}																							\
		}																								\
	}																									\
	return s.str();																						\
}		

DEFINEMATRIXSTRSPECIALISATION( M33f, 3 );
DEFINEMATRIXSTRSPECIALISATION( M33d, 3 );
DEFINEMATRIXSTRSPECIALISATION( M44f, 4 );
DEFINEMATRIXSTRSPECIALISATION( M44d, 4 );

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix33InvertOverloads, invert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix33InverseOverloads, inverse, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix33GJInvertOverloads, gjInvert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix33GJInverseOverloads, gjInverse, 0, 1);		

template<typename T>
void bindMatrix33(const char *bindName)
{
	const Matrix33<T> &(Matrix33<T>::*setScale1)(const Vec2<T> &) = &Matrix33<T>::setScale;	
	const Matrix33<T> &(Matrix33<T>::*setScale2)(T) = &Matrix33<T>::setScale;	
	
	const Matrix33<T> &(Matrix33<T>::*setShear1)(const T &) = &Matrix33<T>::template setShear<T>;
	const Matrix33<T> &(Matrix33<T>::*setShear2)(const Vec2<T> &) = &Matrix33<T>::template setShear<T>;
	
	const Matrix33<T> &(Matrix33<T>::*shear1)(const T &) = &Matrix33<T>::template shear<T>;
	const Matrix33<T> &(Matrix33<T>::*shear2)(const Vec2<T> &) = &Matrix33<T>::template shear<T>;
	
	class_< Matrix33<T> >(bindName)
		//.def_readwrite("x", &Matrix33<T>::x)
		.def(init<>())
		.def(init<T>())
		.def(init<T, T, T, T, T, T, T, T, T>())
		
		.def(init<const Matrix33<T> &>())
	
		.def("dimensions", &MatrixDimensions<Matrix33<T> >::get)
	
		// [] operator support
		.def("__getitem__", &MatrixWrapper< Matrix33<T> >::get)
		.def("__setitem__", &MatrixWrapper< Matrix33<T> >::set)
		
		.def("makeIdentity", &Matrix33<T>::makeIdentity)
	
		.def(self == self)
		.def(self != self)
	
		.def("equalWithAbsError", &Matrix33<T>::equalWithAbsError)
		.def("equalWithRelError", &Matrix33<T>::equalWithRelError)
	
		.def(self += self)
		.def(self += T())
		.def(self + self)
	
		.def(self -= self)
		.def(self -= T())
		.def(self - self)
		
		.def(- self)
		.def("negate", &Matrix33<T>::negate, return_self<>())
		
		.def(self *= T())
		.def(self * T())
		
		.def(self *= self)
		.def(self * self)
		
		.def("multVecMatrix", multVecMatrix<Matrix33<T>, Vec2<T> > )
		.def("multDirMatrix", multDirMatrix<Matrix33<T>, Vec2<T> > )
		
		.def(self /= T())
		.def(self / T())
		
		.def("transpose", &Matrix33<T>::transpose, return_self<>())
		.def("transposed", &Matrix33<T>::transposed)
		
		.def("invert", &Matrix33<T>::invert, return_self<>(), Matrix33InvertOverloads() )
		.def("inverse", &Matrix33<T>::inverse, Matrix33InverseOverloads() )
		.def("gjInvert", &Matrix33<T>::gjInvert, return_self<>(), Matrix33GJInvertOverloads() )
		.def("gjInverse", &Matrix33<T>::gjInverse, Matrix33GJInverseOverloads() )
		
		.def("setRotation", &Matrix33<T>::template setRotation<T>, return_self<>())
		.def("rotate", &Matrix33<T>::template rotate<T>, return_self<>())
		
		.def("setScale", setScale1, return_self<>())
		.def("setScale", setScale2, return_self<>())
		
		.def("scale", &Matrix33<T>::template scale<T>, return_self<>())
		.def("setTranslation", &Matrix33<T>::template setTranslation<T>, return_self<>())
		
		.def("translation", &Matrix33<T>::translation)
		.def("translate", &Matrix33<T>::template translate<T>, return_self<>())
		
		.def("setShear", setShear1, return_self<>())
		.def("setShear", setShear2, return_self<>())
		
		.def("shear", shear1, return_self<>())
		.def("shear", shear2, return_self<>())
		
		.def("baseTypeMin", &Matrix33<T>::baseTypeMin).staticmethod("baseTypeMin")
		.def("baseTypeMax", &Matrix33<T>::baseTypeMax).staticmethod("baseTypeMax")
		.def("baseTypeSmallest", &Matrix33<T>::baseTypeSmallest).staticmethod("baseTypeSmallest")
		.def("baseTypeEpsilon", &Matrix33<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon")
		
		.def("__str__", &IECore::str<Matrix33<T> > ) 
		.def("__repr__", &IECore::repr<Matrix33<T> > )
		
		.def("createScaled", &createScaled<Matrix33<T>, Vec2<T> > ).staticmethod( "createScaled" )
		.def("createTranslated", &createTranslated<Matrix33<T>, Vec2<T> > ).staticmethod( "createTranslated" )
		.def("createRotated", &createRotated<Matrix33<T>, T > ).staticmethod( "createRotated" )
		
		.def("extractScaling", &extractScaling<Matrix33<T>, Vec2<T> > )
		.def("sansScaling", (Matrix33<T>(*)( const Matrix33<T> &))&sansScaling2<Matrix33<T> > )
		.def("removeScaling", &removeScaling<Matrix33<T> > )
		.def("extractScalingAndShear", &extractScalingAndShear33<T> )
		.def("sansScalingAndShear", &sansScalingAndShear<Matrix33<T> > )
		.def("removeScalingAndShear", &removeScalingAndShear<Matrix33<T> > )
		.def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear33<T> )
		.def("extractSHRT", &extractSHRT33<T> )
	;
	
}


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44InvertOverloads, invert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44InverseOverloads, inverse, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44GJInvertOverloads, gjInvert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44GJInverseOverloads, gjInverse, 0, 1);

template<typename T>
void bindMatrix44(const char *bindName)
{
	const Matrix44<T> &(Matrix44<T>::*setScale1)(const Vec3<T> &) = &Matrix44<T>::setScale;	
	const Matrix44<T> &(Matrix44<T>::*setScale2)(T) = &Matrix44<T>::setScale;	
	
	//const Matrix44<T> &(Matrix44<T>::*setShear1)(const Shear6<T> &) = &Matrix44<T>::template setShear<T>;
	const Matrix44<T> &(Matrix44<T>::*setShear2)(const Vec3<T> &) = &Matrix44<T>::template setShear<T>;
	
	//const Matrix44<T> &(Matrix44<T>::*shear1)(const Shear6<T> &) = &Matrix44<T>::template shear<T>;
	const Matrix44<T> &(Matrix44<T>::*shear2)(const Vec3<T> &) = &Matrix44<T>::template shear<T>;
	
	class_< Matrix44<T> >(bindName)
		//.def_readwrite("x", &Matrix44<T>::x)
		.def(init<>())
		.def(init<T>())
		.def(init<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>())
		.def(init<Matrix33<T>, Vec3<T> >())
		.def(init<const Matrix44<T> &>())
		
		//.def(self = T())
		
		.def("dimensions", &MatrixDimensions<Matrix44<T> >::get)
	
		// [] operator support
		.def("__getitem__", &MatrixWrapper< Matrix44<T> >::get)
		.def("__setitem__", &MatrixWrapper< Matrix44<T> >::set)
		
		.def("makeIdentity", &Matrix44<T>::makeIdentity)
	
		.def(self == self)
		.def(self != self)
	
		.def("equalWithAbsError", &Matrix44<T>::equalWithAbsError)
		.def("equalWithRelError", &Matrix44<T>::equalWithRelError)
	
		.def(self += self)
		.def(self += T())
		.def(self + self)
	
		.def(self -= self)
		.def(self -= T())
		.def(self - self)
		
		.def(- self)
		.def("negate", &Matrix44<T>::negate, return_self<>())
		
		.def(self *= T())
		.def(self * T())
		
		.def(self *= self)
		.def(self * self)
		
		.def("multVecMatrix", multVecMatrix<Matrix44<T>, Vec3<T> > )
		.def("multDirMatrix", multDirMatrix<Matrix44<T>, Vec3<T> > )
		
		.def(self /= T())
		.def(self / T())
		
		.def("transpose", &Matrix44<T>::transpose, return_self<>())
		.def("transposed", &Matrix44<T>::transposed)
		
		.def("invert", &Matrix44<T>::invert, return_self<>(), Matrix44InvertOverloads() )
		.def("inverse", &Matrix44<T>::inverse, Matrix44InverseOverloads() )
		.def("gjInvert", &Matrix44<T>::gjInvert, return_self<>(), Matrix44GJInvertOverloads() )
		.def("gjInverse", &Matrix44<T>::gjInverse, Matrix44GJInverseOverloads() )
		
		.def("setEulerAngles", &Matrix44<T>::template setEulerAngles<T>, return_self<>())
		.def("setAxisAngle", &Matrix44<T>::template setAxisAngle<T>, return_self<>())
		.def("rotate", &Matrix44<T>::template rotate<T>, return_self<>())
		
		.def("setScale", setScale1, return_self<>())
		.def("setScale", setScale2, return_self<>())
		
		.def("scale", &Matrix44<T>::template scale<T>, return_self<>())
		.def("setTranslation", &Matrix44<T>::template setTranslation<T>, return_self<>())
		
		.def("translation", &Matrix44<T>::translation)
		.def("translate", &Matrix44<T>::template translate<T>, return_self<>())
		
		//.def("setShear", setShear1, return_self<>())
		.def("setShear", setShear2, return_self<>())
		
		//.def("shear", shear1, return_self<>())
		.def("shear", shear2, return_self<>())
		
		.def("baseTypeMin", &Matrix44<T>::baseTypeMin).staticmethod("baseTypeMin")
		.def("baseTypeMax", &Matrix44<T>::baseTypeMax).staticmethod("baseTypeMax")
		.def("baseTypeSmallest", &Matrix44<T>::baseTypeSmallest).staticmethod("baseTypeSmallest")
		.def("baseTypeEpsilon", &Matrix44<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon")
		
		.def("__str__", &IECore::str<Matrix44<T> > ) 
		.def("__repr__", &IECore::repr<Matrix44<T> > )
		
		.def("createScaled", &createScaled<Matrix44<T>, Vec3<T> > ).staticmethod( "createScaled" )
		.def("createTranslated", &createTranslated<Matrix44<T>, Vec3<T> > ).staticmethod( "createTranslated" )
		.def("createRotated", &createRotated<Matrix44<T>, Vec3<T> > ).staticmethod( "createRotated" )
		.def("createAimed", &Imath::rotationMatrix<T> )
		.def("createAimed", &Imath::rotationMatrixWithUpDir<T> ).staticmethod( "createAimed" )
		.def("createFromBasis", &matrixFromBasis ).staticmethod( "createFromBasis" )
		
		.def("extractScaling", &extractScaling<Matrix44<T>, Vec3<T> > )
		.def("sansScaling", &sansScaling2<Matrix44<T> > )
		.def("removeScaling", &removeScaling<Matrix44<T> > )
		.def("extractScalingAndShear", &extractScalingAndShear44<T> )
		.def("sansScalingAndShear", &sansScalingAndShear<Matrix44<T> > )
		.def("removeScalingAndShear", &removeScalingAndShear<Matrix44<T> > )
		.def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear44<T> )
		.def("extractEulerXYZ", &extractEulerXYZ<Matrix44<T>, Vec3<T> > )
		.def("extractEulerXYZ", &extractEulerZYX<Matrix44<T>, Vec3<T> > )
		.def("extractQuat", &extractQuat<T> )
		.def("extractSHRT", &extractSHRT44<T> )
	;
	
	/// \todo deprecate this form in favour of the static createFromBasis method.
	def("matrixFromBasis", &matrixFromBasis );
	
}

}
