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

#include "boost/python.hpp"

#include "IECore/PrimitiveEvaluator.h"
#include "IECorePython/PrimitiveEvaluatorBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace IECore;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct PrimitiveEvaluatorHelper
{
	static PrimitiveEvaluatorPtr create( PrimitivePtr primitive )
	{
		if( !primitive )
		{
			PyErr_SetString( PyExc_ValueError, "Null primitive" );
			throw_error_already_set();
		}
		return PrimitiveEvaluator::create( primitive );
	}

	static float signedDistance( PrimitiveEvaluator &evaluator, const Imath::V3f &p )
	{

		float distance = 0.0;
		bool success = evaluator.signedDistance( p, distance );

		if ( !success )
		{
		}

		return distance;
	}

	static bool closestPoint( PrimitiveEvaluator &evaluator, const Imath::V3f &p, PrimitiveEvaluator::Result *result )
	{
		evaluator.validateResult( result );

		return evaluator.closestPoint( p, result );
	}

	static bool pointAtUV( PrimitiveEvaluator &evaluator, const Imath::V2f &uv, PrimitiveEvaluator::Result *result )
	{
		evaluator.validateResult( result );

		return evaluator.pointAtUV( uv, result );
	}

	static bool intersectionPoint( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction, PrimitiveEvaluator::Result *result )
	{
		evaluator.validateResult( result );

		return evaluator.intersectionPoint( origin, direction, result );
	}

	static bool intersectionPointMaxDist( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction, PrimitiveEvaluator::Result *result, float maxDist )
	{
		evaluator.validateResult( result );

		return evaluator.intersectionPoint( origin, direction, result, maxDist );
	}

	static list intersectionPoints( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction )
	{
		std::vector< PrimitiveEvaluator::ResultPtr > results;
		evaluator.intersectionPoints( origin, direction, results );

		list result;

		for ( std::vector< PrimitiveEvaluator::ResultPtr >::const_iterator it = results.begin(); it != results.end(); ++it)
		{
			result.append( *it );
		}

		return result;
	}

	static list intersectionPoints( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction, float maxDistance )
	{
		std::vector< PrimitiveEvaluator::ResultPtr > results;
		evaluator.intersectionPoints( origin, direction, results, maxDistance );

		list result;

		for ( std::vector< PrimitiveEvaluator::ResultPtr >::const_iterator it = results.begin(); it != results.end(); ++it)
		{
			result.append( *it );
		}

		return result;
	}

	static PrimitivePtr primitive( PrimitiveEvaluator &evaluator )
	{
		return evaluator.primitive()->copy();
	}

};

static object primVar( PrimitiveEvaluator::Result &r, PrimitiveVariable &v )
{
	switch( v.data->typeId() )
	{
		case V3fDataTypeId :
		case V3fVectorDataTypeId :
			return object( r.vectorPrimVar( v ) );
		case FloatDataTypeId :
		case FloatVectorDataTypeId :
			return object( r.floatPrimVar( v ) );
		case IntDataTypeId :
		case IntVectorDataTypeId :
			return object( r.intPrimVar( v ) );
		case StringDataTypeId :
		case StringVectorDataTypeId :
			return object( r.stringPrimVar( v ) );
		case Color3fDataTypeId :
		case Color3fVectorDataTypeId :
			return object( r.colorPrimVar( v ) );
		case HalfDataTypeId :
		case HalfVectorDataTypeId :
			return object( r.halfPrimVar( v ) );	
		default :
			throw Exception( "Unsupported PrimitiveVariable datatype." );
	}
}

void bindPrimitiveEvaluator()
{
	list (*intersectionPoints)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &) = &PrimitiveEvaluatorHelper::intersectionPoints;
	list (*intersectionPointsMaxDist)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, float) = &PrimitiveEvaluatorHelper::intersectionPoints;

	bool (*intersectionPoint)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, PrimitiveEvaluator::Result *) = &PrimitiveEvaluatorHelper::intersectionPoint;
	bool (*intersectionPointMaxDist)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, PrimitiveEvaluator::Result *, float ) = &PrimitiveEvaluatorHelper::intersectionPointMaxDist;

	object p = RunTimeTypedClass<PrimitiveEvaluator>()
		.def( "create", &PrimitiveEvaluatorHelper::create ).staticmethod("create")
		.def( "createResult", &PrimitiveEvaluator::createResult )
		.def( "validateResult", &PrimitiveEvaluator::validateResult )
		.def( "signedDistance", &PrimitiveEvaluatorHelper::signedDistance )
		.def( "closestPoint", &PrimitiveEvaluatorHelper::closestPoint )
		.def( "pointAtUV", &PrimitiveEvaluatorHelper::pointAtUV )
		.def( "intersectionPoint", intersectionPoint )
		.def( "intersectionPoint", intersectionPointMaxDist )
		.def( "intersectionPoints", intersectionPoints )
		.def( "intersectionPoints", intersectionPointsMaxDist )
		.def( "primitive", &PrimitiveEvaluatorHelper::primitive )
		.def( "volume", &PrimitiveEvaluator::volume )
		.def( "centerOfGravity", &PrimitiveEvaluator::centerOfGravity )
		.def( "surfaceArea", &PrimitiveEvaluator::surfaceArea )
	;

	{
		scope ps( p );
		RefCountedClass<PrimitiveEvaluator::Result, RefCounted>( "Result" )
			.def( "point", &PrimitiveEvaluator::Result::point )
			.def( "normal", &PrimitiveEvaluator::Result::normal )
			.def( "uv", &PrimitiveEvaluator::Result::uv )
			.def( "uTangent", &PrimitiveEvaluator::Result::uTangent )
			.def( "vTangent", &PrimitiveEvaluator::Result::vTangent )
			.def( "vectorPrimVar", &PrimitiveEvaluator::Result::vectorPrimVar )
			.def( "floatPrimVar", &PrimitiveEvaluator::Result::floatPrimVar )
			.def( "intPrimVar", &PrimitiveEvaluator::Result::intPrimVar )
			.def( "stringPrimVar", &PrimitiveEvaluator::Result::stringPrimVar, return_value_policy<copy_const_reference>() )
			.def( "colorPrimVar", &PrimitiveEvaluator::Result::colorPrimVar )
			.def( "halfPrimVar", &PrimitiveEvaluator::Result::halfPrimVar )
			.def( "primVar", &primVar )
		;

	}
}

}
