//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2016, Image Engine Design Inc. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "IECore/CurvesPrimitive.h"
#include "IECore/Exception.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"
#include "IECoreArnold/CurvesAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreArnold;

namespace
{

NodeAlgo::ConverterDescription<CurvesPrimitive> g_description( CurvesAlgo::convert );

} // namespace

AtNode *CurvesAlgo::convert( const IECore::CurvesPrimitive *curves )
{
	const V3fVectorData *p = curves->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		throw Exception( "CurvesPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}

	// make the result curves and add points

	AtNode *result = AiNode( "curves" );

	const std::vector<int> verticesPerCurve = curves->verticesPerCurve()->readable();
	AiNodeSetArray(
		result,
		"num_points",
		AiArrayConvert( verticesPerCurve.size(), 1, AI_TYPE_INT, (void *)&( verticesPerCurve[0] ) )
	);

	ShapeAlgo::convertP( p, result, "points" );

	// set basis

	if( curves->basis() == CubicBasisf::bezier() )
	{
		AiNodeSetStr( result, "basis", "bezier" );
	}
	else if( curves->basis() == CubicBasisf::bSpline() )
	{
		AiNodeSetStr( result, "basis", "b-spline" );
	}
	else if( curves->basis() == CubicBasisf::catmullRom() )
	{
		AiNodeSetStr( result, "basis", "catmull-rom" );
	}
	else if( curves->basis() == CubicBasisf::linear() )
	{
		AiNodeSetStr( result, "basis", "linear" );
	}
	else
	{
		// just accept the default
	}

	// add radius

	ShapeAlgo::convertRadius( curves, result );

	// add arbitrary user parameters

	const char *ignore[] = { "P", "width", "radius", 0 };
	ShapeAlgo::convertPrimitiveVariables( curves, result, ignore );

	return result;
}
