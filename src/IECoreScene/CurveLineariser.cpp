//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/CurveLineariser.h"

#include "IECoreScene/CurvesPrimitiveEvaluator.h"

#include "IECore/CompoundParameter.h"
#include "IECore/FastFloat.h"
#include "IECore/MessageHandler.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( CurveLineariser );

CurveLineariser::CurveLineariser()
	:	CurvesPrimitiveOp( "Converts cubic curves to linear curves." )
{
	FloatParameterPtr verticesPerSegmentParameter = new FloatParameter(
		"verticesPerSegment",
		"The number of vertices to use to approximate a single segment "
		"of the input cubic curve.",
		10.0f,
		0.0f
	);

	parameters()->addParameter( verticesPerSegmentParameter );
}

CurveLineariser::~CurveLineariser()
{
}

FloatParameter * CurveLineariser::verticesPerSegmentParameter()
{
	return parameters()->parameter<FloatParameter>( "verticesPerSegment" );
}

const FloatParameter * CurveLineariser::verticesPerSegmentParameter() const
{
	return parameters()->parameter<FloatParameter>( "verticesPerSegment" );
}

void CurveLineariser::modifyTypedPrimitive( CurvesPrimitive * curves, const CompoundObject * operands )
{
	if( curves->basis()==CubicBasisf::linear() )
	{
		return;
	}

	CurvesPrimitiveEvaluatorPtr evaluator = new CurvesPrimitiveEvaluator( curves );
	PrimitiveEvaluator::ResultPtr evaluatorResult = evaluator->createResult();

	std::vector<PrimitiveVariable> primitiveVariables;
	std::vector<IECore::TypeId> primitiveVariableTypes;
	std::vector<void *> primitiveVariableVectors;
	for( PrimitiveVariableMap::iterator it=curves->variables.begin(); it!=curves->variables.end(); it++ )
	{
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Invalid :
			case PrimitiveVariable::Constant :
			case PrimitiveVariable::Uniform :
				// we don't need to process these as they're not interpolated
				continue;
			default :
				// fall through to process the variable
				;
		}

		switch( it->second.data->typeId() )
		{
			case V3fVectorDataTypeId :
			{
				primitiveVariables.push_back( evaluator->primitive()->variables.find( it->first )->second );
				primitiveVariableTypes.push_back( V3fVectorDataTypeId );
				std::vector<V3f> &v = static_cast<V3fVectorData *>( it->second.data.get() )->writable();
				v.clear();
				primitiveVariableVectors.push_back( &v );
				break;
			}
			case FloatVectorDataTypeId :
			{
				primitiveVariables.push_back( evaluator->primitive()->variables.find( it->first )->second );
				primitiveVariableTypes.push_back( FloatVectorDataTypeId );
				std::vector<float> &v = static_cast<FloatVectorData *>( it->second.data.get() )->writable();
				v.clear();
				primitiveVariableVectors.push_back( &v );
				break;
			}
			case IntVectorDataTypeId :
			{
				primitiveVariables.push_back( evaluator->primitive()->variables.find( it->first )->second );
				primitiveVariableTypes.push_back( IntVectorDataTypeId );
				std::vector<int> &v = static_cast<IntVectorData *>( it->second.data.get() )->writable();
				v.clear();
				primitiveVariableVectors.push_back( &v );
				break;
			}
			case Color3fVectorDataTypeId :
			{
				primitiveVariables.push_back( evaluator->primitive()->variables.find( it->first )->second );
				primitiveVariableTypes.push_back( Color3fVectorDataTypeId );
				std::vector<Color3f> &v = static_cast<Color3fVectorData *>( it->second.data.get() )->writable();
				v.clear();
				primitiveVariableVectors.push_back( &v );
				break;
			}
			default :
				msg(
					Msg::Warning,
					"CurveLineariser::modifyTypedPrimitive",
					boost::format( "Ignoring primitive variable \"%s\" with unsupported type \"%s\"" ) % it->first % it->second.data->typeName()
				);
		}
	}

	size_t numCurves = curves->numCurves();
	bool periodic = curves->periodic();

	IntVectorDataPtr newVerticesPerCurveData = new IntVectorData();
	std::vector<int> &newVerticesPerCurve = newVerticesPerCurveData->writable();
	newVerticesPerCurve.resize( numCurves );

	float verticesPerSegment = operands->member<FloatData>( "verticesPerSegment" )->readable();

	for( size_t curveIndex=0; curveIndex<numCurves; curveIndex++ )
	{
		int numVertices = fastFloatFloor( verticesPerSegment * (float)curves->numSegments( curveIndex ) );
		numVertices = std::max( numVertices, periodic ? 3 : 2 );

		float vStep = periodic ? ( 1.0f / (float)( numVertices ) ) : ( 1.0f / (float)( numVertices - 1 ) );
		for( int i=0; i<numVertices; i++ )
		{
			float v = std::min( vStep * i, 1.0f );
			evaluator->pointAtV( curveIndex, v, evaluatorResult.get() );
			for( size_t j=0; j<primitiveVariables.size(); j++ )
			{
				switch( primitiveVariableTypes[j] )
				{
					case V3fVectorDataTypeId :
						static_cast<std::vector<V3f> *>( primitiveVariableVectors[j] )->push_back( evaluatorResult->vectorPrimVar( primitiveVariables[j] ) );
						break;
					case FloatVectorDataTypeId :
						static_cast<std::vector<float> *>( primitiveVariableVectors[j] )->push_back( evaluatorResult->floatPrimVar( primitiveVariables[j] ) );
						break;
					case IntVectorDataTypeId :
						static_cast<std::vector<int> *>( primitiveVariableVectors[j] )->push_back( evaluatorResult->intPrimVar( primitiveVariables[j] ) );
						break;
					case Color3fVectorDataTypeId :
						static_cast<std::vector<Color3f> *>( primitiveVariableVectors[j] )->push_back( evaluatorResult->colorPrimVar( primitiveVariables[j] ) );
						break;
					default :
						assert( 0 ); // shouldn't get here
				}
			}
		}

		newVerticesPerCurve[curveIndex] = numVertices;
	}

	curves->setTopology( newVerticesPerCurveData, CubicBasisf::linear(), periodic );
}
