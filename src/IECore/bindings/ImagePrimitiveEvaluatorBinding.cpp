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

#include "IECore/ImagePrimitiveEvaluator.h"
#include "IECore/bindings/ImagePrimitiveEvaluatorBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"

using namespace IECore;
using namespace boost::python;

namespace IECore
{

struct ImagePrimitiveEvaluatorHelper
{
	
	static bool pointAtPixel( ImagePrimitiveEvaluator &evaluator, const Imath::V2i &pixel, const PrimitiveEvaluator::ResultPtr &result )
	{
		evaluator.validateResult( result );
		
		return evaluator.pointAtPixel( pixel, result );
	}
	
	static object R( ImagePrimitiveEvaluator &evaluator )
	{
		PrimitiveVariableMap::const_iterator it = evaluator.R();
		
		if ( it != evaluator.primitive()->variables.end() )
		{
			return object( it->second );
		}
		else
		{
			return object();
		}
	}
	
	static object G( ImagePrimitiveEvaluator &evaluator )
	{
		PrimitiveVariableMap::const_iterator it = evaluator.G();
		
		if ( it != evaluator.primitive()->variables.end() )
		{
			return object( it->second );
		}
		else
		{
			return object();
		}
	}
	
	static object B( ImagePrimitiveEvaluator &evaluator )
	{
		PrimitiveVariableMap::const_iterator it = evaluator.B();
		
		if ( it != evaluator.primitive()->variables.end() )
		{
			return object( it->second );
		}
		else
		{
			return object();
		}
	}
	
	static object A( ImagePrimitiveEvaluator &evaluator )
	{
		PrimitiveVariableMap::const_iterator it = evaluator.A();
		
		if ( it != evaluator.primitive()->variables.end() )
		{
			return object( it->second );
		}
		else
		{
			return object();
		}
	}
	
	static object Y( ImagePrimitiveEvaluator &evaluator )
	{
		PrimitiveVariableMap::const_iterator it = evaluator.Y();
		
		if ( it != evaluator.primitive()->variables.end() )
		{
			return object( it->second );
		}
		else
		{
			return object();
		}
	}
	
};

void bindImagePrimitiveEvaluator()
{
	typedef class_< ImagePrimitiveEvaluator, ImagePrimitiveEvaluatorPtr, bases< PrimitiveEvaluator >, boost::noncopyable > ImagePrimitiveEvaluatorPyClass;
	
	object m = ImagePrimitiveEvaluatorPyClass ( "ImagePrimitiveEvaluator", no_init )
		.def( init< ImagePrimitivePtr > () )
		.def( "pointAtPixel", &ImagePrimitiveEvaluatorHelper::pointAtPixel )
		.def( "R", &ImagePrimitiveEvaluatorHelper::R )
		.def( "G", &ImagePrimitiveEvaluatorHelper::G )
		.def( "B", &ImagePrimitiveEvaluatorHelper::B )
		.def( "A", &ImagePrimitiveEvaluatorHelper::A )
		.def( "Y", &ImagePrimitiveEvaluatorHelper::Y )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(ImagePrimitiveEvaluator)
		
		/// \todo Move these into the base class
		.def( "primitive", &ImagePrimitiveEvaluator::primitive )
		.def( "volume", &ImagePrimitiveEvaluator::volume )
		.def( "centerOfGravity", &ImagePrimitiveEvaluator::centerOfGravity )
		.def( "surfaceArea", &ImagePrimitiveEvaluator::surfaceArea )
	;
	INTRUSIVE_PTR_PATCH( ImagePrimitiveEvaluator, ImagePrimitiveEvaluatorPyClass );
	implicitly_convertible<ImagePrimitiveEvaluatorPtr, PrimitiveEvaluatorPtr>();
	
	{
		scope ms( m );
		typedef class_< ImagePrimitiveEvaluator::Result, ImagePrimitiveEvaluator::ResultPtr, bases< PrimitiveEvaluator::Result >, boost::noncopyable > ResultPyClass;
		
		ResultPyClass( "Result", no_init )
			.def( "pixel", &ImagePrimitiveEvaluator::Result::pixel )
			.def( "uintPrimVar", &ImagePrimitiveEvaluator::Result::uintPrimVar )
			.def( "shortPrimVar", &ImagePrimitiveEvaluator::Result::shortPrimVar )
			.def( "ushortPrimVar", &ImagePrimitiveEvaluator::Result::ushortPrimVar )
			.def( "charPrimVar", &ImagePrimitiveEvaluator::Result::charPrimVar )
			.def( "ucharPrimVar", &ImagePrimitiveEvaluator::Result::ucharPrimVar )
			
		;
	
		INTRUSIVE_PTR_PATCH( ImagePrimitiveEvaluator::Result, ResultPyClass );
		implicitly_convertible<ImagePrimitiveEvaluator::ResultPtr, PrimitiveEvaluator::ResultPtr>();	
	}
}

}
