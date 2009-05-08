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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include <cassert>

#include "IECore/CubeColorLookupData.h"
#include "IECore/bindings/CubeColorLookupBinding.h"
#include "IECore/bindings/IECoreBinding.h"
#include "IECore/CubeColorLookup.h"
#include "IECore/ColorTransformOp.h"
#include "IECore/ImagePrimitive.h"

using namespace boost::python;
using namespace Imath;

namespace IECore
{

template<typename T>
static T *construct( const V3i &dimension, object o, const typename T::BoxType &domain, typename T::Interpolation interpolation )
{
	if ( dimension.x < 2 || dimension.y < 2 || dimension.z < 2 )
	{
		throw InvalidArgumentException( "CubeColorLookup: Dimension must be at least 2 in every axis" );
	}

	extract<ColorTransformOpPtr> ex( o );

	if ( ex.check() )
	{
		ColorTransformOpPtr op = ex();

		typename T::DataType data;
		typedef TypedData< typename T::DataType > TypedDataType;

		typename TypedDataType::Ptr Cs = new TypedDataType();

		typename T::DataType &samplePoints = Cs->writable();

		const typename T::VecType domainSize = domain.size();

		for ( int x = 0; x < dimension.x; x++ )
		{
			for ( int y = 0; y < dimension.y; y++ )
			{
				for ( int z = 0; z < dimension.z; z++ )
				{
					samplePoints.push_back(
						Color3f(
							domainSize.x * x / ( dimension.x - 1 ) + domain.min.x,
							domainSize.y * y / ( dimension.y - 1 ) + domain.min.y,
							domainSize.z * z / ( dimension.z - 1 ) + domain.min.z
						)
					);
				}
			}
		}
		assert( (int)samplePoints.size() == dimension.x * dimension.y * dimension.z );

		Box2i window( V2i( 0, 0 ), V2i( samplePoints.size() - 1, 0 ) );

		ImagePrimitivePtr img = new ImagePrimitive( window, window );
		img->variables["Cs"] = PrimitiveVariable( PrimitiveVariable::Varying, Cs );
		assert( img->arePrimitiveVariablesValid() );

		op->inputParameter()->setValue( img );

		ImagePrimitivePtr result = runTimeCast< ImagePrimitive> ( op->operate() );
		assert( result );
		assert( result->variables.find( "Cs" ) != result->variables.end() );
		assert( result->variables.find( "Cs" )->second.data );
		assert( runTimeCast< TypedDataType >( result->variables.find( "Cs" )->second.data ) );

		const typename T::DataType &resultData = runTimeCast< TypedDataType >( result->variables.find( "Cs" )->second.data)->readable();

		return new T( dimension, resultData, domain, interpolation);
	}
	else
	{
		typename T::DataType data;

		int s = extract<int>( o.attr( "__len__" )() );
		for( int i=0; i<s; i++ )
		{
			typename T::ColorType c = extract< typename T::ColorType >( o[i] );
			data.push_back( c );
		}
		return new T( dimension, data, domain, interpolation );
	}
}

template<typename T>
static object getData( const CubeColorLookup<T> &o )
{
	list result;
	const std::vector< Color3< T > > &data = o.data();

	for ( typename std::vector< Color3< T > >::const_iterator it = data.begin(); it != data.end(); ++it )
	{
		result.append( *it );

	}

	return result;
}

template<typename T>
void bindCubeColorLookup( const char *name )
{
	class_<T> o = class_<T>( name, no_init )
		.def( init<>() )
		.def( "__call__", &T::operator() )
		.def( "dimension", &T::dimension, return_value_policy<copy_const_reference>() )
		.def( "domain", &T::domain, return_value_policy<copy_const_reference>() )
		.def( "data", &getData<T> )
		.def( "getInterpolation", &T::getInterpolation )
		.def( "setInterpolation", &T::setInterpolation )

		.def( self==self )
		.def( self!=self )
	;

	{
		scope s( o );

		enum_< typename T::Interpolation >( "Interpolation" )
			.value( "NoInterpolation", T::NoInterpolation )
			.value( "Linear", T::Linear )
		;
	}

	/// Define this here because it depends on the "Interpolation" enum, defined above.
	o
		.def( "__init__",
			make_constructor(
				&construct<T>,
				default_call_policies(),
				(arg_("dimension"), arg_("data"), arg_("domain") = typename T::BoxType( typename T::VecType( 0, 0, 0 ), typename T::VecType( 1, 1, 1 ) ), arg_("interpolation") = T::Linear)
			)
		);
}

void bindCubeColorLookup()
{
	bindCubeColorLookup<CubeColorLookupf>( "CubeColorLookupf" );
	bindCubeColorLookup<CubeColorLookupd>( "CubeColorLookupd" );
}

}
