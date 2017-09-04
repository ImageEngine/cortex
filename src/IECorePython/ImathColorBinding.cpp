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

#include "boost/python.hpp"

#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathColorAlgo.h"

#include "IECorePython/ImathColorBinding.h"
#include "IECorePython/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;

namespace IECorePython
{

template<typename T>
struct ColorIndexer
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

/// \todo The only reason this is a macro is so that it can turn the class type to a string. We should probably do this
/// with a small traits class instead, and get rid of the macro.
#define DEFINECOLSTRSPECIALISATION( COL )				\
														\
template<>												\
std::string repr<COL>( COL &x )							\
{														\
	std::stringstream s;								\
	s << "IECore." << #COL << "( ";						\
	for( unsigned i=0; i<COL::dimensions(); i++ )		\
	{													\
		s << boost::lexical_cast<std::string>( x[i] );	\
		if( i!=COL::dimensions()-1 )					\
		{												\
			s << ", ";									\
		}												\
	}													\
	s << " )";											\
	return s.str();										\
}														\
														\
template<>												\
std::string str<COL>( COL &x )							\
{														\
	std::stringstream s;								\
	for( unsigned i=0; i<COL::dimensions(); i++ )		\
	{													\
		s << boost::lexical_cast<std::string>( x[i] );	\
		if( i!=COL::dimensions()-1 )					\
		{												\
			s << " ";									\
		}												\
	}													\
	return s.str();										\
}														\

DEFINECOLSTRSPECIALISATION( Color3f );
DEFINECOLSTRSPECIALISATION( Color4f );
typedef Color3<double> Color3d;
typedef Color4<double> Color4d;
DEFINECOLSTRSPECIALISATION( Color3d );
DEFINECOLSTRSPECIALISATION( Color4d );

template<typename T>
T hsvToRGB( const T &c )
{
	return hsv2rgb( c );
}

template<typename T>
T rgbToHSV( const T &c )
{
	return rgb2hsv( c );
}

template<typename T>
void bindColorCommon( class_<T> &c )
{

	c.def( self==self );
	c.def( self!=self );

	c.def( self += self );
	c.def( self + self );

	c.def( self -= self );
	c.def( self - self );

	c.def( -self );
	c.def( "negate", &T::negate, return_self<>() );

	c.def( self *= self );
	
	c.def( self *= typename T::BaseType() );
	
	c.def( self * self );
	c.def( self * typename T::BaseType() );
	c.def( typename T::BaseType() * self );

	c.def( self /= self );
	c.def( self /= typename T::BaseType() );
	c.def( self / self );
	c.def( self / typename T::BaseType() );

	c.def( "dimensions", &T::dimensions ).staticmethod( "dimensions" );

	// [] operator support
	c.def("__getitem__", &ColorIndexer< T >::get);
	c.def("__setitem__", &ColorIndexer< T >::set);

	c.def( "baseTypeMin", &T::baseTypeMin ).staticmethod( "baseTypeMin" );
	c.def( "baseTypeMax", &T::baseTypeMax ).staticmethod( "baseTypeMax" );
	c.def( "baseTypeSmallest", &T::baseTypeSmallest ).staticmethod( "baseTypeSmallest" );
	c.def( "baseTypeEpsilon", &T::baseTypeEpsilon ).staticmethod( "baseTypeEpsilon" );

	c.def( "hsvToRGB", &hsvToRGB<T> );
	c.def( "rgbToHSV", &rgbToHSV<T> );

	c.def( "__str__", &str<T> );
	c.def( "__repr__", &repr<T> );
}

/// We need this and equalWithRelError so that we can call them passing colors instead of vectors.
/// We deliberately don't expose the fact that Color3 derives from Vec3 because we think that is weird.
template<typename T>
static bool equalWithAbsError( const T &c1, const T &c2, typename T::BaseType e )
{
	return c1.equalWithAbsError( c2, e );
}

template<typename T>
static bool equalWithRelError( const T &c1, const T &c2, typename T::BaseType e )
{
	return c1.equalWithRelError( c2, e );
}

template<typename T>
void bindColor3( const char *typeName )
{

	// we deliberately don't expose the fact that
	// Color3 derives from Vec3 because we think
	// that is weird.
	class_<T> c = class_<T>( typeName )

		.def_readwrite( "r", &T::x )
		.def_readwrite( "g", &T::y )
		.def_readwrite( "b", &T::z )

		.def( init<>() )
		.def( init<typename T::BaseType>() )
		.def( init<typename T::BaseType, typename T::BaseType, typename T::BaseType>() )
		.def( init<const T&>() )
		.def( init<const Imath::V3f &> () )
		.def( init<const Imath::V3d &> () )

		.def( "equalWithAbsError", &equalWithAbsError<T> )
		.def( "equalWithRelError", &equalWithRelError<T> )
	;

	bindColorCommon( c );

}

template<typename T>
void bindColor4( const char *typeName )
{
	class_<T> c = class_<T>( typeName )

		.def_readwrite( "r", &T::r )
		.def_readwrite( "g", &T::g )
		.def_readwrite( "b", &T::b )
		.def_readwrite( "a", &T::a )

		.def( init<>() )
		.def( init<typename T::BaseType>() )
		.def( init<typename T::BaseType, typename T::BaseType, typename T::BaseType, typename T::BaseType>() )
		.def( init<const T&>() )

	;

	bindColorCommon( c );

}

void bindImathColor()
{
	bindColor3<Color3f>( "Color3f" );
	bindColor4<Color4f>( "Color4f" );
	bindColor3<Color3<double> >( "Color3d" );
	bindColor4<Color4<double> >( "Color4d" );
}

} // namespace IECorePython
