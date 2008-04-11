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

#include "boost/python.hpp"

#include "IECore/PrimitiveEvaluator.h"
#include "IECore/bindings/PrimitiveEvaluatorBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"

using namespace IECore;
using namespace boost::python;

namespace IECore
{

struct PrimitiveEvaluatorHelper
{
	static PrimitiveEvaluatorPtr create( PrimitivePtr primitive )
	{
		return PrimitiveEvaluator::create( primitive );
	}
	
	static bool closestPoint( PrimitiveEvaluator &evaluator, const Imath::V3f &p, const PrimitiveEvaluator::ResultPtr &result )
	{
		evaluator.validateResult( result );
		
		return evaluator.closestPoint( p, result );
	}
			
	static bool pointAtUV( PrimitiveEvaluator &evaluator, const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result )
	{
		evaluator.validateResult( result );
		
		return evaluator.pointAtUV( uv, result );
	}
		
	static bool intersectionPoint( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction, const PrimitiveEvaluator::ResultPtr &result )
	{
		evaluator.validateResult( result );
	
		return evaluator.intersectionPoint( origin, direction, result );
	}
	
	static bool intersectionPointMaxDist( PrimitiveEvaluator& evaluator, const Imath::V3f &origin, const Imath::V3f &direction, const PrimitiveEvaluator::ResultPtr &result, float maxDist )
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
};

void bindPrimitiveEvaluator()
{
	typedef class_< PrimitiveEvaluator, PrimitiveEvaluatorPtr, bases< RunTimeTyped >, boost::noncopyable > PrimitiveEvaluatorPyClass;
	
	list (*intersectionPoints)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &) = &PrimitiveEvaluatorHelper::intersectionPoints;
	list (*intersectionPointsMaxDist)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, float) = &PrimitiveEvaluatorHelper::intersectionPoints;	
	
	bool (*intersectionPoint)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, const PrimitiveEvaluator::ResultPtr &) = &PrimitiveEvaluatorHelper::intersectionPoint;
	bool (*intersectionPointMaxDist)(PrimitiveEvaluator&, const Imath::V3f &, const Imath::V3f &, const PrimitiveEvaluator::ResultPtr &, float ) = &PrimitiveEvaluatorHelper::intersectionPointMaxDist;	
	
	object p = PrimitiveEvaluatorPyClass ( "PrimitiveEvaluator", no_init )
		.def( "create", &PrimitiveEvaluatorHelper::create ).staticmethod("create")
		.def( "createResult", &PrimitiveEvaluator::createResult )
		.def( "validateResult", &PrimitiveEvaluator::validateResult )		
		.def( "closestPoint", &PrimitiveEvaluatorHelper::closestPoint )
		.def( "pointAtUV", &PrimitiveEvaluatorHelper::pointAtUV )
		.def( "intersectionPoint", intersectionPoint )
		.def( "intersectionPoint", intersectionPointMaxDist )		
		.def( "intersectionPoints", intersectionPoints )
		.def( "intersectionPoints", intersectionPointsMaxDist )
		.def( "primitive", &PrimitiveEvaluator::primitive )
		.def( "volume", &PrimitiveEvaluator::volume )
		.def( "centerOfGravity", &PrimitiveEvaluator::centerOfGravity )
		.def( "surfaceArea", &PrimitiveEvaluator::surfaceArea )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(PrimitiveEvaluator)
	;
	INTRUSIVE_PTR_PATCH( PrimitiveEvaluator, PrimitiveEvaluatorPyClass );
	implicitly_convertible<PrimitiveEvaluatorPtr, RunTimeTypedPtr>();
	
	{
		scope ps( p );
		typedef class_< PrimitiveEvaluator::Result, PrimitiveEvaluator::ResultPtr, bases< RefCounted >, boost::noncopyable > ResultPyClass;
		
		ResultPyClass( "Result", no_init )
			.def( "point", &PrimitiveEvaluator::Result::point )
			.def( "normal", &PrimitiveEvaluator::Result::normal )			
			.def( "uv", &PrimitiveEvaluator::Result::uv )			
			.def( "uTangent", &PrimitiveEvaluator::Result::uv )			
			.def( "vTangent", &PrimitiveEvaluator::Result::uv )									
			.def( "vectorPrimVar", &PrimitiveEvaluator::Result::vectorPrimVar )						
			.def( "floatPrimVar", &PrimitiveEvaluator::Result::floatPrimVar )
			.def( "intPrimVar", &PrimitiveEvaluator::Result::intPrimVar )			
			.def( "stringPrimVar", &PrimitiveEvaluator::Result::stringPrimVar, return_value_policy<copy_const_reference>() )
			.def( "colorPrimVar", &PrimitiveEvaluator::Result::colorPrimVar )
			.def( "halfPrimVar", &PrimitiveEvaluator::Result::halfPrimVar )								
		;
	
		INTRUSIVE_PTR_PATCH( PrimitiveEvaluator::Result, ResultPyClass );
		implicitly_convertible<PrimitiveEvaluator::ResultPtr, RefCountedPtr>();	
	}
}

}
