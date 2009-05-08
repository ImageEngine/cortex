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

#include "OpenEXR/ImathRandom.h"
#include "OpenEXR/ImathQuat.h"

#include "IECore/TriangleAlgo.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/TriangulateOp.h"
#include "IECore/MeshPrimitiveImplicitSurfaceFunction.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveImplicitSurfaceFunction::MeshPrimitiveImplicitSurfaceFunction( MeshPrimitivePtr mesh ) : PrimitiveImplicitSurfaceFunction()
{
	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( mesh );

	mesh = runTimeCast< MeshPrimitive > ( op->operate() );

	m_evaluator = PrimitiveEvaluator::create( mesh );

	if (! m_evaluator )
	{
		throw InvalidArgumentException( "Cannot create evaluator in MeshPrimitiveImplicitSurfaceFunction" );
	}
}

MeshPrimitiveImplicitSurfaceFunction::~MeshPrimitiveImplicitSurfaceFunction()
{
}

MeshPrimitiveImplicitSurfaceFunction::Value MeshPrimitiveImplicitSurfaceFunction::operator()( const PrimitiveImplicitSurfaceFunction::Point &p )
{
	float distance = 0.0f;

	bool found = m_evaluator->signedDistance( p, distance );

	if (found)
	{
		return distance;
	}
	else
	{
		return Imath::limits<MeshPrimitiveImplicitSurfaceFunction::Value>::min();
	}
}

MeshPrimitiveImplicitSurfaceFunction::Value MeshPrimitiveImplicitSurfaceFunction::getValue( const Point &p )
{
	return this->operator()(p);
}
