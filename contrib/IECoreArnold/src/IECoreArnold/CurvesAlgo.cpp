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

#include "IECore/MessageHandler.h"
#include "IECoreScene/CurvesPrimitive.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"
#include "IECoreArnold/CurvesAlgo.h"
#include "IECoreArnold/ParameterAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreArnold;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

const AtString g_pointsArnoldString("points");
const AtString g_basisArnoldString("basis");
const AtString g_bezierArnoldString("bezier");
const AtString g_bSplineArnoldString("b-spline");
const AtString g_catmullRomArnoldString("catmull-rom");
const AtString g_curvesArnoldString("curves");
const AtString g_linearArnoldString("linear");
const AtString g_modeArnoldString("mode");
const AtString g_motionStartArnoldString("motion_start");
const AtString g_motionEndArnoldString("motion_end");
const AtString g_numPointsArnoldString("num_points");
const AtString g_orientationsArnoldString("orientations");
const AtString g_orientedArnoldString("oriented");


NodeAlgo::ConverterDescription<CurvesPrimitive> g_description( CurvesAlgo::convert, CurvesAlgo::convert );

AtNode *convertCommon( const IECoreScene::CurvesPrimitive *curves, const std::string &nodeName, const AtNode *parentNode )
{

	AtNode *result = AiNode( g_curvesArnoldString, AtString( nodeName.c_str() ), parentNode );

	const std::vector<int> verticesPerCurve = curves->verticesPerCurve()->readable();
	AiNodeSetArray(
		result,
		g_numPointsArnoldString,
		AiArrayConvert( verticesPerCurve.size(), 1, AI_TYPE_INT, (void *)&( verticesPerCurve[0] ) )
	);

	// set basis

	if( curves->basis() == CubicBasisf::bezier() )
	{
		AiNodeSetStr( result, g_basisArnoldString, g_bezierArnoldString );
	}
	else if( curves->basis() == CubicBasisf::bSpline() )
	{
		AiNodeSetStr( result, g_basisArnoldString, g_bSplineArnoldString );
	}
	else if( curves->basis() == CubicBasisf::catmullRom() )
	{
		AiNodeSetStr( result, g_basisArnoldString, g_catmullRomArnoldString );
	}
	else if( curves->basis() == CubicBasisf::linear() )
	{
		AiNodeSetStr( result, g_basisArnoldString, g_linearArnoldString );
	}
	else
	{
		// just accept the default
	}

	// add arbitrary user parameters

	const char *ignore[] = { "P", "N", "width", "radius", nullptr };
	ShapeAlgo::convertPrimitiveVariables( curves, result, ignore );

	return result;

}

} // namespace

AtNode *CurvesAlgo::convert( const IECoreScene::CurvesPrimitive *curves, const std::string &nodeName, const AtNode *parentNode )
{
	AtNode *result = convertCommon( curves, nodeName, parentNode );
	ShapeAlgo::convertP( curves, result, g_pointsArnoldString );
	ShapeAlgo::convertRadius( curves, result );

	// Convert "N" to orientations

	if( const V3fVectorData *n = curves->variableData<V3fVectorData>( "N", PrimitiveVariable::Vertex ) )
	{
		AiNodeSetStr( result, g_modeArnoldString, g_orientedArnoldString );
		AiNodeSetArray(
			result,
			g_orientationsArnoldString,
			AiArrayConvert( n->readable().size(), 1, AI_TYPE_VECTOR, (void *)&( n->readable()[0] ) )
		);
	}

	return result;
}

AtNode *CurvesAlgo::convert( const std::vector<const IECoreScene::CurvesPrimitive *> &samples, float motionStart, float motionEnd, const std::string &nodeName, const AtNode *parentNode )
{
	AtNode *result = convertCommon( samples.front(), nodeName, parentNode );

	std::vector<const IECoreScene::Primitive *> primitiveSamples( samples.begin(), samples.end() );
	ShapeAlgo::convertP( primitiveSamples, result, g_pointsArnoldString );
	ShapeAlgo::convertRadius( primitiveSamples, result );

	// Convert "N" to orientations

	vector<const Data *> nSamples;
	nSamples.reserve( samples.size() );
	for( vector<const CurvesPrimitive *>::const_iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
	{
		if( const V3fVectorData *n = (*it)->variableData<V3fVectorData>( "N", PrimitiveVariable::Vertex ) )
		{
			nSamples.push_back( n );
		}
	}

	if( nSamples.size() == samples.size() )
	{
		AiNodeSetStr( result, g_modeArnoldString, g_orientedArnoldString );
		AtArray *array = ParameterAlgo::dataToArray( nSamples, AI_TYPE_VECTOR );
		AiNodeSetArray( result, g_orientationsArnoldString, array );
	}
	else if( nSamples.size() )
	{
		IECore::msg( IECore::Msg::Warning, "CurvesAlgo::convert", "Missing sample for primitive variable \"N\" - not setting orientations." );
	}

	AiNodeSetFlt( result, g_motionStartArnoldString, motionStart );
	AiNodeSetFlt( result, g_motionEndArnoldString, motionEnd );

	return result;
}

