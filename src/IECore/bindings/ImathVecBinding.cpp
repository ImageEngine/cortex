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

// This include needs to be the very first to prevent problems with warnings 
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

// System includes
#include <string>
#include <stdexcept>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/Exception.h"

#include "IECore/bindings/ImathVecBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace std;

namespace IECore 
{

template<class L>
static const char *typeName()
{
	BOOST_STATIC_ASSERT( sizeof(L) == 0 );
	return "";
}

template<>
static const char *typeName<V2f>()
{
	return "V2f";
}

template<>
static const char *typeName<V2d>()
{
	return "V2d";
}

template<>
static const char *typeName<V2i>()
{
	return "V2i";
}

template<>
static const char *typeName<V3f>()
{
	return "V3f";
}

template<>
static const char *typeName<V3d>()
{
	return "V3d";
}

template<>
static const char *typeName<V3i>()
{
	return "V3i";
}

template<typename T>
void bindVec2();

template<typename T>
void bindVec3();

void bindImathVec()
{
	bindVec2<float>();
	bindVec2<double>();
	bindVec2<int>();

	bindVec3<float>();
	bindVec3<double>();
	bindVec3<int>();
}

template<typename T>
struct VectorIndexer
{
	typedef typename T::BaseType V;
	static V get(const T &x, int i)
	{	
		// Do we want to handle backward indexing?
		if ( i >= 0 && i < static_cast<int>(T::dimensions()) ) 
		{
			return x[i];
		}
		else
		{
			/// \todo Give a description of the error! NB Boost 1.38.0 will translate these into IndexError python exceptions
			throw std::out_of_range("");	
		}
		
	}
	
	static void set(T &x, int i, const V &v)
	{
		if ( i >= 0 && i < static_cast<int>(T::dimensions()) ) 
		{
			x[i] = v;
		}
		else
		{
			/// \todo Give a description of the error! NB Boost 1.38.0 will translate these into IndexError python exceptions
			throw std::out_of_range("");	
		}
	}
	
};

#define DEFINEVECSTRSPECIALISATION( VEC )\
\
template<>\
std::string repr<VEC>( VEC &x )\
{\
	std::stringstream s;\
	s << "IECore." << typeName<VEC>() << "( ";\
	for( unsigned i=0; i<VEC::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=VEC::dimensions()-1 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<>\
std::string str<VEC>( VEC &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<VEC::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=VEC::dimensions()-1 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINEVECSTRSPECIALISATION( V2i );
DEFINEVECSTRSPECIALISATION( V2f );
DEFINEVECSTRSPECIALISATION( V2d );
DEFINEVECSTRSPECIALISATION( V3i );
DEFINEVECSTRSPECIALISATION( V3f );
DEFINEVECSTRSPECIALISATION( V3d );

template<typename V>
V *constructFromList( list l )
{
	if ( len( l ) != (int)V::dimensions() )
	{
		throw InvalidArgumentException( std::string( "Invalid list length given to IECore." ) + typeName<V>() + " constructor" );
	}
	
	V *r = new V();
	
	for ( unsigned i = 0; i < V::dimensions(); i ++ )
	{
		extract< typename V::BaseType > ex( l[i] );
		if ( !ex.check() )
		{
			throw InvalidArgumentException( std::string( "Invalid list element given to IECore." ) + typeName<V>() + " constructor" );
		}
		
		(*r)[i] = ex();
	}
	
	return r ;
}

template<typename T>
void bindVec2()
{	
	// To allow correct resolve of overloaded methods
	void (Vec2<T>::*sv1)(T, T) = &Vec2<T>::template setValue<T>;
	void (Vec2<T>::*sv2)(const Vec2<T>&) = &Vec2<T>::template setValue<T>;
	
	class_< Vec2<T> >( typeName<Vec2<T> >() )
		.def_readwrite("x", &Vec2<T>::x)
		.def_readwrite("y", &Vec2<T>::y)
	
		// [] operator support
		.def("__getitem__", &VectorIndexer<Vec2<T> >::get)
		.def("__setitem__", &VectorIndexer<Vec2<T> >::set)
	
		.def(init<>())
		.def(init<T>())
		.def(init<T, T>())
	
		.def(init<const Vec2<float> &>())
		.def(init<const Vec2<double> &>())
		.def(init<const Vec2<int> &>())
		
		.def("__init__", make_constructor( &constructFromList< Vec2<T> > ) )
	
		.def("setValue", sv1)
		.def("setValue", sv2)
	
		.def(self == self)
		.def(self != self)
	
		.def("equalWithAbsError", &Vec2<T>::equalWithAbsError)
		.def("equalWithRelError", &Vec2<T>::equalWithRelError)

		.def("dot", &Vec2<T>::dot)
	
		.def("cross", &Vec2<T>::cross)
	
		.def(self ^ self)
		
		.def(self % self)
	
		.def(self += self)
		.def(self + self)
		
		.def(self -= self)
		.def(self - self)
				
		.def(- self)
		.def("negate", &Vec2<T>::negate, return_self<>())
		
		.def(self *= self)
		.def(self *= T())
		.def(self * self)
		.def(self * T())
		.def(T() * self)
		
		.def(self /= self)
		.def(self /= T())
		.def(self / self)
		.def(self / T())
		
		.def(self * Matrix33<T>())
		.def(self *= Matrix33<T>())
	
		.def("length", &Vec2<T>::length)
		.def("length2", &Vec2<T>::length2)
	
		.def("normalize", &Vec2<T>::normalize, return_self<>())
		.def("normalizeExc", &Vec2<T>::normalizeExc, return_self<>())
		.def("normalizeNonNull", &Vec2<T>::normalizeNonNull, return_self<>())
		
		.def("normalized", &Vec2<T>::normalized)
		.def("normalizedExc", &Vec2<T>::normalizedExc)
		.def("normalizedNonNull", &Vec2<T>::normalizedNonNull)
	
		.def("dimensions", &Vec2<T>::dimensions).staticmethod("dimensions")
	
		.def("baseTypeMin", &Vec2<T>::baseTypeMin).staticmethod("baseTypeMin")
		.def("baseTypeMax", &Vec2<T>::baseTypeMax).staticmethod("baseTypeMax")
		.def("baseTypeSmallest", &Vec2<T>::baseTypeSmallest).staticmethod("baseTypeSmallest")
		.def("baseTypeEpsilon", &Vec2<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon")

		.def( "__str__", &IECore::str<Vec2<T> > )
		.def( "__repr__", &IECore::repr<Vec2<T> > )
	;
}


template<typename T>
void bindVec3()
{	
	// To allow correct resolve of overloaded methods
	void (Vec3<T>::*sv1)(T, T, T) = &Vec3<T>::template setValue<T>;
	void (Vec3<T>::*sv2)(const Vec3<T>&) = &Vec3<T>::template setValue<T>;
	
	class_< Vec3<T> >( typeName<Vec3<T> >() )
		.def_readwrite("x", &Vec3<T>::x)
		.def_readwrite("y", &Vec3<T>::y)
		.def_readwrite("z", &Vec3<T>::z)
	
		// [] operator support
		.def("__getitem__", &VectorIndexer<Vec3<T> >::get)
		.def("__setitem__", &VectorIndexer<Vec3<T> >::set)
	
		.def(init<>())
		.def(init<T>())
		.def(init<T, T, T>())
	
		.def(init<const Vec3<float> &>())
		.def(init<const Vec3<double> &>())
		.def(init<const Vec3<int> &>())
		
		.def("__init__", make_constructor( &constructFromList< Vec3<T> > ) )
	
		.def("setValue", sv1)
		.def("setValue", sv2)
	
		.def(self == self)
		.def(self != self)
	
		.def("equalWithAbsError", &Vec3<T>::equalWithAbsError)
		.def("equalWithRelError", &Vec3<T>::equalWithRelError)

		.def("dot", &Vec3<T>::dot)
	
		.def("cross", &Vec3<T>::cross)
	
		.def(self ^ self)

		.def(self %= self)
		.def(self % self)
	
		.def(self += self)
		.def(self + self)
		
		.def(self -= self)
		.def(self - self)
				
		.def(- self)
		.def("negate", &Vec3<T>::negate, return_self<>())
		
		.def(self *= self)
		.def(self *= T())
		.def(self * self)
		.def(self * T())
		.def(T() * self)
		
		.def(self /= self)
		.def(self /= T())
		.def(self / self)
		.def(self / T())	
	
		.def(self * Matrix44<T>())
		.def(self *= Matrix44<T>())

		.def("length", &Vec3<T>::length)
		.def("length2", &Vec3<T>::length2)
	
		.def("normalize", &Vec3<T>::normalize, return_self<>())
		.def("normalizeExc", &Vec3<T>::normalizeExc, return_self<>())
		.def("normalizeNonNull", &Vec3<T>::normalizeNonNull, return_self<>())
		
		.def("normalized", &Vec3<T>::normalized)
		.def("normalizedExc", &Vec3<T>::normalizedExc)
		.def("normalizedNonNull", &Vec3<T>::normalizedNonNull)
	
		.def("dimensions", &Vec3<T>::dimensions).staticmethod("dimensions")
	
		.def("baseTypeMin", &Vec3<T>::baseTypeMin).staticmethod("baseTypeMin")
		.def("baseTypeMax", &Vec3<T>::baseTypeMax).staticmethod("baseTypeMax")
		.def("baseTypeSmallest", &Vec3<T>::baseTypeSmallest).staticmethod("baseTypeSmallest")
		.def("baseTypeEpsilon", &Vec3<T>::baseTypeEpsilon).staticmethod("baseTypeEpsilon")
		
		.def( "__str__", &IECore::str<Vec3<T> > )
		.def( "__repr__", &IECore::repr<Vec3<T> > )
	;
}

}
