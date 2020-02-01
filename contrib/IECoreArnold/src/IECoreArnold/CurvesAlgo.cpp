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

#include "IECoreArnold/CurvesAlgo.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ParameterAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"

#include "IECoreScene/CurvesAlgo.h"
#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/MessageHandler.h"

#include "ai.h"

using namespace std;
using namespace Imath;
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
const AtString g_uvsArnoldString( "uvs" );

NodeAlgo::ConverterDescription<CurvesPrimitive> g_description( IECoreArnold::CurvesAlgo::convert, IECoreArnold::CurvesAlgo::convert );

ConstCurvesPrimitivePtr resampleCurves( const CurvesPrimitive *curves )
{
	if( curves->basis().standardBasis() == StandardCubicBasis::Linear )
	{
		return curves;
	}

	CurvesPrimitivePtr updatedCurves = nullptr;
	for( const auto &it : curves->variables )
	{
		if( it.second.interpolation == PrimitiveVariable::Vertex && it.first != "P" && it.first != "N" )
		{
			if( !updatedCurves )
			{
				updatedCurves = curves->copy();
			}

			IECoreScene::CurvesAlgo::resamplePrimitiveVariable( updatedCurves.get(), updatedCurves->variables[it.first], PrimitiveVariable::Varying );
		}
	}

	return updatedCurves ? updatedCurves.get() : curves;
}

void convertUVs( const IECoreScene::CurvesPrimitive *curves, AtNode *node )
{
	auto it = curves->variables.find( "uv" );
	if( it == curves->variables.end() )
	{
		return;
	}

	if( !runTimeCast<const V2fVectorData>( it->second.data.get() ) )
	{
		msg( Msg::Warning, "CurvesAlgo", boost::format( "Variable \"uv\" has unsupported type \"%s\" (expected V2fVectorData)." ) % it->second.data->typeName() );
		return;
	}

	PrimitiveVariable::IndexedView<V2f> uvs( it->second );
	AtArray *array = AiArrayAllocate( uvs.size(), 1, AI_TYPE_VECTOR2 );
	for( size_t i = 0, e = uvs.size(); i < e; ++i )
	{
		const V2f &uv = uvs[i];
		AiArraySetVec2( array, i, AtVector2( uv[0], uv[1] ) );
	}

	AiNodeSetArray( node, g_uvsArnoldString, array );
}

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

	// Add UVs and arbitrary user parameters

	convertUVs( curves, result );

	const char *ignore[] = { "P", "N", "width", "radius", "uv", nullptr };
	ShapeAlgo::convertPrimitiveVariables( curves, result, ignore );

	return result;

}

} // namespace

AtNode *IECoreArnold::CurvesAlgo::convert( const IECoreScene::CurvesPrimitive *curves, const std::string &nodeName, const AtNode *parentNode )
{
	// Arnold (and IECoreArnold::ShapeAlgo) does not support Vertex PrimitiveVariables for
	// cubic CurvesPrimitives, so we resample the variables to Varying first.
	ConstCurvesPrimitivePtr resampledCurves = ::resampleCurves( curves );

	AtNode *result = convertCommon( resampledCurves.get(), nodeName, parentNode );
	ShapeAlgo::convertP( resampledCurves.get(), result, g_pointsArnoldString );
	ShapeAlgo::convertRadius( resampledCurves.get(), result );

	// Convert "N" to orientations

	if( const V3fVectorData *n = resampledCurves.get()->variableData<V3fVectorData>( "N", PrimitiveVariable::Vertex ) )
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

AtNode *IECoreArnold::CurvesAlgo::convert( const std::vector<const IECoreScene::CurvesPrimitive *> &samples, float motionStart, float motionEnd, const std::string &nodeName, const AtNode *parentNode )
{
	// Arnold (and IECoreArnold::ShapeAlgo) does not support Vertex PrimitiveVariables for
	// cubic CurvesPrimitives, so we resample the variables to Varying first.
	std::vector<ConstCurvesPrimitivePtr> updatedSamples;
	std::vector<const Primitive *> primitiveSamples;
	// Also convert "N" to orientations
	std::vector<const Data *> nSamples;
	updatedSamples.reserve( samples.size() );
	primitiveSamples.reserve( samples.size() );
	nSamples.reserve( samples.size() );
	for( const CurvesPrimitive *curves : samples )
	{
		ConstCurvesPrimitivePtr resampledCurves = ::resampleCurves( curves );
		updatedSamples.push_back( resampledCurves );
		primitiveSamples.push_back( resampledCurves.get() );

		if( const V3fVectorData *n = curves->variableData<V3fVectorData>( "N", PrimitiveVariable::Vertex ) )
		{
			nSamples.push_back( n );
		}
	}

	AtNode *result = convertCommon( updatedSamples.front().get(), nodeName, parentNode );

	ShapeAlgo::convertP( primitiveSamples, result, g_pointsArnoldString );
	ShapeAlgo::convertRadius( primitiveSamples, result );

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

