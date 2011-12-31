//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CurvesPrimitive.h"
#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreArnold/ToArnoldCurvesConverter.h"

using namespace IECoreArnold;
using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( ToArnoldCurvesConverter );

ToArnoldCurvesConverter::ToArnoldCurvesConverter( IECore::CurvesPrimitivePtr toConvert )
	:	ToArnoldConverter( "Converts IECore::CurvesPrimitives to arnold curves nodes", IECore::CurvesPrimitive::staticTypeId() )
{
	srcParameter()->setValue( toConvert );
}

ToArnoldCurvesConverter::~ToArnoldCurvesConverter()
{
}

AtNode *ToArnoldCurvesConverter::doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const
{
	const CurvesPrimitive *curves = static_cast<const CurvesPrimitive *>( from.get() );
	
	// make the result curves and add points
	
	AtNode *result = AiNode( "curves" );

	const std::vector<int> verticesPerCurve = curves->verticesPerCurve()->readable();
	AiNodeSetArray(
		result,
		"num_points",
		AiArrayConvert( verticesPerCurve.size(), 1, AI_TYPE_INT, (void *)&( verticesPerCurve[0] ) )
	);

	const V3fVectorData *p = curves->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		AiNodeDestroy( result );
		throw Exception( "CurvesPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}
	AiNodeSetArray(
		result,
		"points",
		AiArrayConvert( p->readable().size(), 1, AI_TYPE_POINT, (void *)&( p->readable()[0] ) )
	);
	
	// add radius
	
	ConstFloatVectorDataPtr radius = curves->variableData<FloatVectorData>( "radius", PrimitiveVariable::Varying );
	if( !radius )
	{
		FloatVectorDataPtr calculatedRadius = new FloatVectorData();
		const FloatVectorData *width = curves->variableData<FloatVectorData>( "width" );
		if( width )
		{
			calculatedRadius->writable().resize( width->readable().size() );
			const std::vector<float>::iterator end = calculatedRadius->writable().end();
			std::vector<float>::const_iterator wIt = width->readable().begin();
			for( std::vector<float>::iterator it = calculatedRadius->writable().begin(); it != end; it++, wIt++ )
			{
				*it = *wIt / 2.0f;
			}
		}
		else
		{
			const FloatData *constantWidth = curves->variableData<FloatData>( "width", PrimitiveVariable::Constant );
			if( !constantWidth )
			{
				constantWidth = curves->variableData<FloatData>( "constantwidth", PrimitiveVariable::Constant );
			}
			float r = constantWidth ? constantWidth->readable() / 2.0f : 0.5f;
			calculatedRadius->writable().push_back( r );
		}
		radius = calculatedRadius;
	}
	
	AiNodeSetArray(
		result,
		"radius",
		AiArrayConvert( radius->readable().size(), 1, AI_TYPE_FLOAT, (void *)&( radius->readable()[0] ) )
	);
	
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
	else
	{
		// just accept the default
	}
	
	return result;
}
