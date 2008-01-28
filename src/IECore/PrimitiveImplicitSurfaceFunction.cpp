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

#include "IECore/PrimitiveImplicitSurfaceFunction.h"

using namespace IECore;

PrimitiveImplicitSurfaceFunction::PrimitiveImplicitSurfaceFunction()
{
}

PrimitiveImplicitSurfaceFunction::PrimitiveImplicitSurfaceFunction(  PrimitivePtr primitive ) : m_primitive( primitive )
{
	m_evaluator = PrimitiveEvaluator::create( primitive );
	
	if (! m_evaluator )
	{
		throw InvalidArgumentException( "Cannot create evaluator in PrimitiveImplicitSurfaceFunction" );
	}
	
	nIt = m_primitive->variables.find("N");
}

PrimitiveImplicitSurfaceFunction::~PrimitiveImplicitSurfaceFunction()		
{
}

PrimitiveImplicitSurfaceFunction::Value PrimitiveImplicitSurfaceFunction::operator()( const PrimitiveImplicitSurfaceFunction::Point &p )
{	
	PrimitiveEvaluator::ResultPtr result = m_evaluator->createResult();
	
	bool found = m_evaluator->closestPoint( p, result );
	
	if (found)
	{		
		Point n = result->normal();
						
		// Use shading normal if available
		if ( nIt != m_primitive->variables.end() )
		{
			n = result->vectorPrimVar( nIt->second );			
		}
		
		/// Compute signed distance from plane, which is defined by closestPoint and closestNormal
		PrimitiveImplicitSurfaceFunction::Value planeConstant = n.dot( result->point() );		
		PrimitiveImplicitSurfaceFunction::Value distance = n.dot( p ) - planeConstant;
		
		return distance;
	}
	else
	{
		return Imath::limits<PrimitiveImplicitSurfaceFunction::Value>::min();
	}	
}

PrimitiveImplicitSurfaceFunction::Value PrimitiveImplicitSurfaceFunction::getValue( const Point &p )
{
	return this->operator()(p);
}
