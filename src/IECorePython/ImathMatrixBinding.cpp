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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

// System includes
#include <string>
#include <stdexcept>

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathMatrixAlgo.h"

#include "IECore/Exception.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/MatrixTraits.h"
#include "IECorePython/ImathMatrixBinding.h"
#include "IECorePython/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace std;
using namespace IECore;

// Binding implementations
namespace IECorePython
{

template<class L>
static const char *typeName()
{
	BOOST_STATIC_ASSERT( sizeof(L) == 0 );
	return "";
}

template<>
const char *typeName<M33f>()
{
	return "M33f";
}

template<>
const char *typeName<M33d>()
{
	return "M33d";
}

template<>
const char *typeName<M44f>()
{
	return "M44f";
}

template<>
const char *typeName<M44d>()
{
	return "M44d";
}

template<typename T>
void bindMatrix33();

template<typename T>
void bindMatrix44();

void bindImathMatrix()
{
	bindMatrix33<float>();
	bindMatrix33<double>();

	bindMatrix44<float>();
	bindMatrix44<double>();
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

template<typename M>
M *constructFromList( list l )
{
	if ( IECorePython::len( l ) != (int)(MatrixTraits<M>::dimensions() * MatrixTraits<M>::dimensions() ) )
	{
		throw InvalidArgumentException( std::string( "Invalid list length given to IECore." ) + typeName<M>() + " constructor" );
	}

	M *r = new M();

	int i = 0;
	for ( unsigned row = 0; row < MatrixTraits<M>::dimensions(); row ++ )
	{
		for ( unsigned col = 0; col < MatrixTraits<M>::dimensions(); col ++ )
		{
			extract< typename MatrixTraits<M>::BaseType > ex( l[i++] );
			if ( !ex.check() )
			{
				throw InvalidArgumentException( std::string( "Invalid list element given to IECore." ) + typeName<M>() + " constructor" );
			}

			(*r)[row][col] = ex();
		}
	}

	return r ;
}

template<typename M, typename T>
M *constructFromMatrix33( const T &m )
{
	return new M(
		m[0][0], m[0][1], m[0][2],
		m[1][0], m[1][1], m[1][2],
		m[2][0], m[2][1], m[2][2]
	);
}

template<typename M, typename T>
M *constructFromMatrix44( const T &m )
{
	return new M(
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]
	);
}

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
			/// \todo Give a description of the error! NB Boost 1.38.0 will translate these into IndexError python exceptions
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
			/// \todo Give a description of the error! NB Boost 1.38.0 will translate these into IndexError python exceptions
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

#define DEFINEMATRIXSTRSPECIALISATION( TYPE, D )\
template<>\
string repr<TYPE>( TYPE &x )\
{\
	stringstream s;\
	s << "IECore." << typeName<TYPE>() << "( ";\
	for( int i=0; i<D; i++ )\
	{\
		for( int j=0; j<D; j++ )\
		{\
			s << x[i][j];\
			if( !(i==D-1 && j==D-1) )\
			{\
				s << ", ";\
			}\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<>\
string str<TYPE>( TYPE &x )\
{\
	stringstream s;\
	for( int i=0; i<D; i++ )\
	{\
		for( int j=0; j<D; j++ )\
		{\
			s << x[i][j];\
			if( !(i==D-1 && j==D-1) )\
			{\
				s << " ";\
			}\
		}\
	}\
	return s.str();\
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
void bindMatrix33()
{
	const Matrix33<T> &(Matrix33<T>::*setScale1)(const Vec2<T> &) = &Matrix33<T>::setScale;
	const Matrix33<T> &(Matrix33<T>::*setScale2)(T) = &Matrix33<T>::setScale;

	const Matrix33<T> &(Matrix33<T>::*setShear1)(const T &) = &Matrix33<T>::template setShear<T>;
	const Matrix33<T> &(Matrix33<T>::*setShear2)(const Vec2<T> &) = &Matrix33<T>::template setShear<T>;

	const Matrix33<T> &(Matrix33<T>::*shear1)(const T &) = &Matrix33<T>::template shear<T>;
	const Matrix33<T> &(Matrix33<T>::*shear2)(const Vec2<T> &) = &Matrix33<T>::template shear<T>;

	const char *bindName = typeName<Matrix33<T> >();

	class_< Matrix33<T> > cls( bindName );

		cls.def(init<>());
		cls.def(init<T>());
		cls.def(init<T, T, T, T, T, T, T, T, T>());
		cls.def("__init__", make_constructor( &constructFromMatrix33< Matrix33<T>, Matrix33<float> > ) );
		cls.def("__init__", make_constructor( &constructFromMatrix33< Matrix33<T>, Matrix33<double> > ) );
		cls.def("__init__", make_constructor( &constructFromList< Matrix33<T> > ) );

		cls.def("dimensions", &MatrixDimensions<Matrix33<T> >::get);

		// [] operator support
		cls.def("__getitem__", &MatrixWrapper< Matrix33<T> >::get);
		cls.def("__setitem__", &MatrixWrapper< Matrix33<T> >::set);

		cls.def("makeIdentity", &Matrix33<T>::makeIdentity);

		cls.def(self == self);
		cls.def(self != self);

		cls.def("equalWithAbsError", &Matrix33<T>::equalWithAbsError);
		cls.def("equalWithRelError", &Matrix33<T>::equalWithRelError);

		cls.def(self += self);
		cls.def(self += T());
		cls.def(self + self);

		cls.def(self -= self);
		cls.def(self -= T());
		cls.def(self - self);

		cls.def(- self);
		cls.def("negate", &Matrix33<T>::negate, return_self<>());

		cls.def(self *= T());
		cls.def(self * T());

		cls.def(self *= self);
		cls.def(self * self);

		cls.def("multVecMatrix", multVecMatrix<Matrix33<T>, Vec2<T> > );
		cls.def("multDirMatrix", multDirMatrix<Matrix33<T>, Vec2<T> > );

		cls.def(self /= T());
		cls.def(self / T());

		cls.def("transpose", &Matrix33<T>::transpose, return_self<>());
		cls.def("transposed", &Matrix33<T>::transposed);

		cls.def("invert", &Matrix33<T>::invert, return_self<>(), Matrix33InvertOverloads() );
		cls.def("inverse", &Matrix33<T>::inverse, Matrix33InverseOverloads() );
		cls.def("gjInvert", &Matrix33<T>::gjInvert, return_self<>(), Matrix33GJInvertOverloads() );
		cls.def("gjInverse", &Matrix33<T>::gjInverse, Matrix33GJInverseOverloads() );

		cls.def("setRotation", &Matrix33<T>::template setRotation<T>, return_self<>());
		cls.def("rotate", &Matrix33<T>::template rotate<T>, return_self<>());

		cls.def("setScale", setScale1, return_self<>());
		cls.def("setScale", setScale2, return_self<>());

		cls.def("scale", &Matrix33<T>::template scale<T>, return_self<>());
		cls.def("setTranslation", &Matrix33<T>::template setTranslation<T>, return_self<>());

		cls.def("translation", &Matrix33<T>::translation);
		cls.def("translate", &Matrix33<T>::template translate<T>, return_self<>());

		cls.def("setShear", setShear1, return_self<>());
		cls.def("setShear", setShear2, return_self<>());

		cls.def("shear", shear1, return_self<>());
		cls.def("shear", shear2, return_self<>());

		cls.def("baseTypeMin", &Matrix33<T>::baseTypeMin).staticmethod("baseTypeMin");
		cls.def("baseTypeMax", &Matrix33<T>::baseTypeMax).staticmethod("baseTypeMax");
		cls.def("baseTypeSmallest", &Matrix33<T>::baseTypeSmallest).staticmethod("baseTypeSmallest");
		cls.def("baseTypeEpsilon", &Matrix33<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon");

		cls.def("__str__", &IECorePython::str<Matrix33<T> > );
		cls.def("__repr__", &IECorePython::repr<Matrix33<T> > );

		cls.def("createScaled", &createScaled<Matrix33<T>, Vec2<T> > ).staticmethod( "createScaled" );
		cls.def("createTranslated", &createTranslated<Matrix33<T>, Vec2<T> > ).staticmethod( "createTranslated" );
		cls.def("createRotated", &createRotated<Matrix33<T>, T > ).staticmethod( "createRotated" );

		cls.def("extractScaling", &extractScaling<Matrix33<T>, Vec2<T> > );
		cls.def("sansScaling", (Matrix33<T>(*)( const Matrix33<T> &))&sansScaling2<Matrix33<T> > );
		cls.def("removeScaling", &removeScaling<Matrix33<T> > );
		cls.def("extractScalingAndShear", &extractScalingAndShear33<T> );
		cls.def("sansScalingAndShear", &sansScalingAndShear<Matrix33<T> > );
		cls.def("removeScalingAndShear", &removeScalingAndShear<Matrix33<T> > );
		cls.def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear33<T> );
		cls.def("extractSHRT", &extractSHRT33<T> );

		cls.def("determinant", (float (*)( const Matrix33<T> & ))&IECore::determinant<T> );
	;

}


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44InvertOverloads, invert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44InverseOverloads, inverse, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44GJInvertOverloads, gjInvert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Matrix44GJInverseOverloads, gjInverse, 0, 1);

template<typename T>
void bindMatrix44()
{
	const Matrix44<T> &(Matrix44<T>::*setScale1)(const Vec3<T> &) = &Matrix44<T>::setScale;
	const Matrix44<T> &(Matrix44<T>::*setScale2)(T) = &Matrix44<T>::setScale;

	const Matrix44<T> &(Matrix44<T>::*setShear2)(const Vec3<T> &) = &Matrix44<T>::template setShear<T>;

	const Matrix44<T> &(Matrix44<T>::*shear2)(const Vec3<T> &) = &Matrix44<T>::template shear<T>;

	const char *bindName = typeName<Matrix44<T> >();

	class_< Matrix44<T> > cls( bindName );

		cls.def(init<>());
		cls.def(init<T>());
		cls.def(init<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>());
		cls.def(init<Matrix33<T>, Vec3<T> >());
		cls.def("__init__", make_constructor( &constructFromMatrix44< Matrix44<T>, Matrix44<float> > ) );
		cls.def("__init__", make_constructor( &constructFromMatrix44< Matrix44<T>, Matrix44<double> > ) );
		cls.def("__init__", make_constructor( &constructFromList< Matrix44<T> > ) );

		cls.def("dimensions", &MatrixDimensions<Matrix44<T> >::get);

		// [] operator support
		cls.def("__getitem__", &MatrixWrapper< Matrix44<T> >::get);
		cls.def("__setitem__", &MatrixWrapper< Matrix44<T> >::set);

		cls.def("makeIdentity", &Matrix44<T>::makeIdentity);

		cls.def(self == self);
		cls.def(self != self);

		cls.def("equalWithAbsError", &Matrix44<T>::equalWithAbsError);
		cls.def("equalWithRelError", &Matrix44<T>::equalWithRelError);

		cls.def(self += self);
		cls.def(self += T());
		cls.def(self + self);

		cls.def(self -= self);
		cls.def(self -= T());
		cls.def(self - self);

		cls.def(- self);
		cls.def("negate", &Matrix44<T>::negate, return_self<>());

		cls.def(self *= T());
		cls.def(self * T());

		cls.def(self *= self);
		cls.def(self * self);

		cls.def("multVecMatrix", multVecMatrix<Matrix44<T>, Vec3<T> > );
		cls.def("multDirMatrix", multDirMatrix<Matrix44<T>, Vec3<T> > );

		cls.def(self /= T());
		cls.def(self / T());

		cls.def("transpose", &Matrix44<T>::transpose, return_self<>());
		cls.def("transposed", &Matrix44<T>::transposed);

		cls.def("invert", &Matrix44<T>::invert, return_self<>(), Matrix44InvertOverloads() );
		cls.def("inverse", &Matrix44<T>::inverse, Matrix44InverseOverloads() );
		cls.def("gjInvert", &Matrix44<T>::gjInvert, return_self<>(), Matrix44GJInvertOverloads() );
		cls.def("gjInverse", &Matrix44<T>::gjInverse, Matrix44GJInverseOverloads() );

		cls.def("setEulerAngles", &Matrix44<T>::template setEulerAngles<T>, return_self<>());
		cls.def("setAxisAngle", &Matrix44<T>::template setAxisAngle<T>, return_self<>());
		cls.def("rotate", &Matrix44<T>::template rotate<T>, return_self<>());

		cls.def("setScale", setScale1, return_self<>());
		cls.def("setScale", setScale2, return_self<>());

		cls.def("scale", &Matrix44<T>::template scale<T>, return_self<>());
		cls.def("setTranslation", &Matrix44<T>::template setTranslation<T>, return_self<>());

		cls.def("translation", &Matrix44<T>::translation);
		cls.def("translate", &Matrix44<T>::template translate<T>, return_self<>());

		cls.def("setShear", setShear2, return_self<>());

		cls.def("shear", shear2, return_self<>());

		cls.def("baseTypeMin", &Matrix44<T>::baseTypeMin).staticmethod("baseTypeMin");
		cls.def("baseTypeMax", &Matrix44<T>::baseTypeMax).staticmethod("baseTypeMax");
		cls.def("baseTypeSmallest", &Matrix44<T>::baseTypeSmallest).staticmethod("baseTypeSmallest");
		cls.def("baseTypeEpsilon", &Matrix44<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon");

		cls.def("__str__", &IECorePython::str<Matrix44<T> > );
		cls.def("__repr__", &IECorePython::repr<Matrix44<T> > );

		cls.def("createScaled", &createScaled<Matrix44<T>, Vec3<T> > ).staticmethod( "createScaled" );
		cls.def("createTranslated", &createTranslated<Matrix44<T>, Vec3<T> > ).staticmethod( "createTranslated" );
		cls.def("createRotated", &createRotated<Matrix44<T>, Vec3<T> > ).staticmethod( "createRotated" );
		cls.def("createAimed", &Imath::rotationMatrix<T> );
		cls.def("createAimed", &Imath::rotationMatrixWithUpDir<T> ).staticmethod( "createAimed" );
		cls.def("createFromBasis", &matrixFromBasis<T> ).staticmethod( "createFromBasis" );

		cls.def("extractScaling", &extractScaling<Matrix44<T>, Vec3<T> > );
		cls.def("sansScaling", &sansScaling2<Matrix44<T> > );
		cls.def("removeScaling", &removeScaling<Matrix44<T> > );
		cls.def("extractScalingAndShear", &extractScalingAndShear44<T> );
		cls.def("sansScalingAndShear", &sansScalingAndShear<Matrix44<T> > );
		cls.def("removeScalingAndShear", &removeScalingAndShear<Matrix44<T> > );
		cls.def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear44<T> );
		cls.def("extractEulerXYZ", &extractEulerXYZ<Matrix44<T>, Vec3<T> > );
		cls.def("extractEulerZYX", &extractEulerZYX<Matrix44<T>, Vec3<T> > );
		cls.def("extractQuat", &extractQuat<T> );
		cls.def("extractSHRT", &extractSHRT44<T> );

		cls.def("determinant", (float (*)( const Matrix44<T> & ))&IECore::determinant<T> );
	;
}

}
