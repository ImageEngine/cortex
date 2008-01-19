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

#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/bindings/MeshPrimitiveEvaluatorBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"

using namespace IECore;
using namespace boost::python;

namespace IECore
{

struct MeshPrimitiveEvaluatorWrap : public MeshPrimitiveEvaluator, public Wrapper<MeshPrimitiveEvaluatorWrap>
{
	IE_CORE_DECLAREMEMBERPTR( MeshPrimitiveEvaluatorWrap );
	
	MeshPrimitiveEvaluatorWrap( PyObject *self, ConstMeshPrimitivePtr mesh )
	: MeshPrimitiveEvaluator( mesh ), Wrapper<MeshPrimitiveEvaluatorWrap>( self, this )
	{	
	}
	
	void validateResult( PrimitiveEvaluator::ResultPtr result ) const
	{
		MeshPrimitiveEvaluator::ResultPtr mr = boost::dynamic_pointer_cast< MeshPrimitiveEvaluator::Result >( result );

		if ( ! mr )
		{
			throw InvalidArgumentException("Incorrect result type passed to MeshPrimitiveEvaluator");
		}
	}
	
	bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::ResultPtr result )
	{
		validateResult( result );
		
		return MeshPrimitiveEvaluator::closestPoint( p, result );
	}
			
	bool pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
	{
		validateResult( result );
		
		return MeshPrimitiveEvaluator::pointAtUV( uv, result );
	}
		
	bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
			const PrimitiveEvaluator::ResultPtr &result, float maxDistance = Imath::limits<float>::max() ) const
	{
		validateResult( result );
		
		return MeshPrimitiveEvaluator::intersectionPoint( origin, direction, result, maxDistance );
	}
	
};

void bindMeshPrimitiveEvaluator()
{
	typedef class_< MeshPrimitiveEvaluator, MeshPrimitiveEvaluatorWrap::Ptr, bases< PrimitiveEvaluator >, boost::noncopyable > MeshPrimitiveEvaluatorPyClass;
	
	object m = MeshPrimitiveEvaluatorPyClass ( "MeshPrimitiveEvaluator", no_init )
		.def( init< ConstMeshPrimitivePtr > () )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(MeshPrimitiveEvaluator)
	;
	INTRUSIVE_PTR_PATCH( MeshPrimitiveEvaluator, MeshPrimitiveEvaluatorPyClass );
	implicitly_convertible<MeshPrimitiveEvaluatorPtr, PrimitiveEvaluatorPtr>();
	
	{
		scope ms( m );
		typedef class_< MeshPrimitiveEvaluator::Result, MeshPrimitiveEvaluator::ResultPtr, bases< PrimitiveEvaluator::Result >, boost::noncopyable > ResultPyClass;
		
		ResultPyClass( "Result", no_init )
			.def( "triangleIndex", &MeshPrimitiveEvaluator::Result::triangleIndex )
			.def( "barycentricCoordinates", &MeshPrimitiveEvaluator::Result::barycentricCoordinates, return_value_policy<copy_const_reference>() )			
			.def( "vertexIds", &MeshPrimitiveEvaluator::Result::vertexIds, return_value_policy<copy_const_reference>() )						
		;
	
		INTRUSIVE_PTR_PATCH( MeshPrimitiveEvaluator::Result, ResultPyClass );
		implicitly_convertible<MeshPrimitiveEvaluator::ResultPtr, PrimitiveEvaluator::ResultPtr>();	
	}
}

}
