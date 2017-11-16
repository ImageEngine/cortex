//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/CurvesPrimitiveEvaluator.h"
#include "IECoreScene/CurvesPrimitive.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/RefCountedBinding.h"

#include "CurvesPrimitiveEvaluatorBinding.h"

using namespace IECore;
using namespace IECoreScene;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

static bool pointAtV( const CurvesPrimitiveEvaluator &e, unsigned curveIndex, float v, PrimitiveEvaluator::Result *r )
{
	e.validateResult( r );
	return e.pointAtV( curveIndex, v, r );
}

static IntVectorDataPtr verticesPerCurve( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.verticesPerCurve() );
}

static IntVectorDataPtr vertexDataOffsets( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.vertexDataOffsets() );
}

static IntVectorDataPtr varyingDataOffsets( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.varyingDataOffsets() );
}

void bindCurvesPrimitiveEvaluator()
{
	scope s = RunTimeTypedClass<CurvesPrimitiveEvaluator>()
		.def( init<CurvesPrimitivePtr>() )
		.def( "pointAtV", &pointAtV )
		.def( "curveLength", &CurvesPrimitiveEvaluator::curveLength,
			(
				arg( "curveIndex" ),
				arg( "vStart" ) = 0.0f,
				arg( "vEnd" ) = 1.0f
			)
		)
		.def( "verticesPerCurve", &verticesPerCurve )
		.def( "vertexDataOffsets", &vertexDataOffsets )
		.def( "varyingDataOffsets", &varyingDataOffsets )
	;

	RefCountedClass<CurvesPrimitiveEvaluator::Result, PrimitiveEvaluator::Result>( "Result" )
		.def( "curveIndex", &CurvesPrimitiveEvaluator::Result::curveIndex )
	;

}

}
