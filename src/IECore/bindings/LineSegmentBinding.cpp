//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
#include <cassert>

#include "boost/static_assert.hpp"

#include "IECore/LineSegment.h"
#include "IECore/bindings/LineSegmentBinding.h"

using namespace boost::python;

namespace IECore
{

template<class L>
static const char *typeName()
{
	BOOST_STATIC_ASSERT( sizeof(L) == 0 );
	return "";
}

template<>
const char *typeName<LineSegment2f>()
{
	return "LineSegment2f";
}

template<>
const char *typeName<LineSegment2d>()
{
	return "LineSegment2d";
}

template<>
const char *typeName<LineSegment3f>()
{
	return "LineSegment3f";
}

template<>
const char *typeName<LineSegment3d>()
{
	return "LineSegment3d";
}

template<class L>
static std::string repr( L &x )
{
	std::stringstream s;

	s << "IECore." << typeName<L>() << "( ";

	object item0( x.p0 );
	assert( item0.attr( "__repr__" ) != object() );
	s << call_method< std::string >( item0.ptr(), "__repr__" ) << ", ";

	object item1( x.p1 );
	assert( item1.attr( "__repr__" ) != object() );
	s << call_method< std::string >( item1.ptr(), "__repr__" ) << " )";

	return s.str();
}

template<class L>
static tuple closestPoints( const L &l, const L &l2 )
{
	typename L::Point a, b;
	a = l.closestPoints( l2, b );
	return make_tuple( a, b );
}

template<class L>
static tuple intersect(  const L &l, const Imath::Plane3< typename L::BaseType > &plane )
{
	typename L::Point pt;
	bool hit = l.intersect( plane, pt );
	return make_tuple( hit, pt );
}

template<class L>
static tuple intersectT(  const L &l, const Imath::Plane3< typename L::BaseType > &plane )
{
	typename L::BaseType d;
	bool hit = l.intersectT( plane, d );
	return make_tuple( hit, d );
}

template<typename Vec>
static void bind3D()
{
	typedef typename Vec::BaseType BaseType;
	typedef LineSegment<Vec> L;
	typedef Imath::Matrix44<BaseType> M;

	const char *name = typeName< L >();

	class_<L>( name )
		.def( init<L>() )
		.def( init<Vec, Vec>() )
		.def_readwrite( "p0", &L::p0 )
		.def_readwrite( "p1", &L::p1 )
		.def( "__repr__", &repr<L> )
		.def( "__call__", &L::operator() )
		.def( "direction", &L::direction )
		.def( "normalizedDirection", &L::normalizedDirection )
		.def( "length", &L::length )
		.def( "length2", &L::length2 )
		.def( "closestPointTo", &L::closestPointTo )
		.def( "closestPoints", &closestPoints<L> )
		.def( "distanceTo", (typename L::BaseType (L::*)( const Vec & ) const)&L::distanceTo )
		.def( "distanceTo", (typename L::BaseType (L::*)( const L & ) const)&L::distanceTo )
		.def( "distance2To", (typename L::BaseType (L::*)( const Vec & ) const)&L::distance2To )
		.def( "distance2To", (typename L::BaseType (L::*)( const L & ) const)&L::distance2To )
		.def( "intersect", &intersect< L > )
		.def( "intersectT", &intersectT< L > )
		.def( self *= M() )
		.def( self * M() )
		.def( self == self )
		.def( self != self )
	;

}

template<typename Vec>
static void bind2D()
{
	typedef typename Vec::BaseType BaseType;
	typedef LineSegment<Vec> L;
	typedef Imath::Matrix33<BaseType> M;

	const char *name = typeName< L >();

	class_<L>( name )
		.def( init<L>() )
		.def( init<Vec, Vec>() )
		.def_readwrite( "p0", &L::p0 )
		.def_readwrite( "p1", &L::p1 )
		.def( "__repr__", &repr<L> )
		.def( "__call__", &L::operator() )
		.def( "direction", &L::direction )
		.def( "normalizedDirection", &L::normalizedDirection )
		.def( "length", &L::length )
		.def( "length2", &L::length2 )
		.def( "closestPointTo", &L::closestPointTo )
		.def( "distanceTo", (typename L::BaseType (L::*)( const Vec & ) const)&L::distanceTo )
		.def( "distance2To", (typename L::BaseType (L::*)( const Vec & ) const)&L::distance2To )
		.def( self *= M() )
		.def( self * M() )
		.def( self == self )
		.def( self != self )
	;

}

void bindLineSegment()
{
	bind3D<Imath::V3f>();
	bind3D<Imath::V3d>();
	bind2D<Imath::V2f>();
	bind2D<Imath::V2d>();
}

} // namespace IECore
