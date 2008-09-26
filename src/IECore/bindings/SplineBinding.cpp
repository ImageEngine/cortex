//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/bindings/SplineBinding.h"
#include "IECore/bindings/IECoreBinding.h"
#include "IECore/Spline.h"

using namespace boost::python;
using namespace Imath;
using namespace std;

namespace IECore 
{

#define REPR_SPECIALISATION( TYPE )																		\
template<>																								\
string repr<TYPE>( TYPE &x )																			\
{																										\
	stringstream s;																						\
	s << "IECore." << #TYPE << "( ";																	\
	s << repr( x.basis ) << ", ";																		\
	s << "(";																							\
	int i = 0;																							\
	int l = x.points.size();																			\
	TYPE::PointContainer::iterator it;																	\
	for( it=x.points.begin(); it!=x.points.end(); it++, i++ )											\
	{																									\
		s << " ( " << it->first << ", " << repr( it->second ) << " )";									\
		if( i!=l-1 )																					\
		{																								\
			s << ",";																					\
		}																								\
	}																									\
	s << " ) )";																						\
	return s.str();																						\
}																										\

REPR_SPECIALISATION( Splineff )
REPR_SPECIALISATION( Splinedd )
REPR_SPECIALISATION( SplinefColor3f )
REPR_SPECIALISATION( SplinefColor4f )

template<typename T>
static T *construct( const typename T::Basis &basis, object o )
{
	typename T::PointContainer points;
	int s = extract<int>( o.attr( "__len__" )() );
	for( int i=0; i<s; i++ )
	{
		object e = o[i];
		int es = extract<int>( e.attr( "__len__" )() );
		if( es!=2 )
		{
			throw Exception( "Each entry in the point sequence must contain two values." );
		}
		object xo = e[0];
		object yo = e[1];
		typename T::XType x = extract<typename T::XType>( xo );
		typename T::YType y = extract<typename T::YType>( yo );
		points.insert( typename T::PointContainer::value_type( x, y ) );
	}
	return new T( basis, points );
}

template<typename T>
static tuple points( const T &s )
{
	boost::python::list p;
	typename T::PointContainer::const_iterator it;
	for( it=s.points.begin(); it!=s.points.end(); it++ )
	{
		p.append( make_tuple( it->first, it->second ) );
	}
	return tuple( p );
}

template<typename T>
static typename T::YType getItem( const T &s, const typename T::XType &x )
{
	typename T::PointContainer::const_iterator it = s.points.find( x );
	if( it==s.points.end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	return it->second;
}

template<typename T>
static void setItem( T &s, const typename T::XType &x, const typename T::YType &y )
{
	s.points.insert( typename T::PointContainer::value_type( x, y ) );
}

template<typename T>
static void delItem( T &s, const typename T::XType &x )
{
	typedef typename T::PointContainer::iterator It;
	std::pair<It, It> r = s.points.equal_range( x );
	if( r.first==s.points.end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	s.points.erase( r.first, r.second );
}

template<typename T>
static bool contains( const T &s, const typename T::XType &x )
{
	typename T::PointContainer::const_iterator it = s.points.find( x );
	return it!=s.points.end();
}

template<typename T>
static size_t len( const T &s )
{
	return s.points.size();
}

template<typename T>
static tuple keys( const T &s )
{
	boost::python::list p;
	typename T::PointContainer::const_iterator it;
	for( it=s.points.begin(); it!=s.points.end(); it++ )
	{
		p.append( it->first );
	}
	return tuple( p );
}

template<typename T>
static tuple values( const T &s )
{
	boost::python::list p;
	typename T::PointContainer::const_iterator it;
	for( it=s.points.begin(); it!=s.points.end(); it++ )
	{
		p.append( it->second );
	}
	return tuple( p );
}

template<typename T>
static tuple interval( const T &s )
{
	/// \todo If we had bindings for boost::interval then we could return one
	/// of those instead.
	typename T::XInterval i = s.interval();
	return make_tuple( i.lower(), i.upper() );
}

template<typename T>
static tuple solve( const T &s, typename T::XType x )
{
	typename T::YType segment[4];
	typename T::XType t = s.solve( x, segment );
	return make_tuple( t, make_tuple( segment[0], segment[1], segment[2], segment[3] ) );
}

template<typename T>
void bindSpline( const char *name )
{
	class_<T>( name, init<>() )
		.def( init<const typename T::Basis &>() )
		.def( "__init__", make_constructor( &construct<T> ) )
		.def_readwrite( "basis", &T::basis )
		.def( "points", &points<T>, "Read only access to the control points as a tuple of tuples of ( x, y ) pairs." )
		.def( "items", &points<T> ) // dictionary style
		.def( "__getitem__", &getItem<T> )
		.def( "__setitem__", &setItem<T> )
		.def( "__delitem__", &delItem<T> )
		.def( "__contains__", &contains<T> )
		.def( "__len__", &len<T> )
		.def( "keys", &keys<T> )
		.def( "values", &values<T> )
		.def( "interval", &interval<T> )
		.def( "solve", &solve<T> )
		.def( "__call__", &T::operator() )
		.def( self==self )
		.def( self!=self )
		.def( "__repr__", &repr<T> )
	;
}

void bindSpline()
{
	bindSpline<Splineff>( "Splineff" );
	bindSpline<Splinedd>( "Splinedd" );
	bindSpline<SplinefColor3f>( "SplinefColor3f" );
	bindSpline<SplinefColor4f>( "SplinefColor4f" );
}

}
