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

#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/VectorTypedData.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/MeshPrimitiveShrinkWrapOp.h"
#include "IECore/PrimitiveEvaluator.h"
#include "IECore/VectorOps.h"
#include "IECore/TriangulateOp.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveShrinkWrapOp::MeshPrimitiveShrinkWrapOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to shrink-wrap one mesh onto another" )
{
	m_targetMeshParameter = new MeshPrimitiveParameter(
	        "target",
	        "The target mesh to shrink-wrap onto.",
	        new MeshPrimitive()
	);

	IntParameter::PresetsMap methodPresets;
	methodPresets["Inside"] = Inside;
	methodPresets["Outside"] = Outside;
	methodPresets["Both"] = Both;

	m_methodParameter = new IntParameter(
	        "method",
	        "The method by which rays are cast to find the target mesh.",
	        Both,
	        Inside,
	        Both,
	        methodPresets,
	        true
	);

	parameters()->addParameter( m_targetMeshParameter );
	parameters()->addParameter( m_methodParameter );
}

MeshPrimitiveShrinkWrapOp::~MeshPrimitiveShrinkWrapOp()
{
}

MeshPrimitiveParameterPtr MeshPrimitiveShrinkWrapOp::targetMeshParameter()
{
	return m_targetMeshParameter;
}

ConstMeshPrimitiveParameterPtr MeshPrimitiveShrinkWrapOp::targetMeshParameter() const
{
	return m_targetMeshParameter;
}

IntParameterPtr MeshPrimitiveShrinkWrapOp::methodParameter()
{
	return m_methodParameter;
}

ConstIntParameterPtr MeshPrimitiveShrinkWrapOp::methodParameter() const
{
	return m_methodParameter;
}

template<typename T>
void MeshPrimitiveShrinkWrapOp::doShrinkWrap( std::vector<T> &vertices, PrimitivePtr sourceMesh, ConstPrimitivePtr targetMesh, Method method )
{
	assert( sourceMesh );

	if ( ! targetMesh )
	{
		return;
	}

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( sourceMesh );
	MeshPrimitivePtr triangulatedSourcePrimitive = runTimeCast< MeshPrimitive > ( op->operate() );

	PrimitiveEvaluatorPtr sourceEvaluator = PrimitiveEvaluator::create( triangulatedSourcePrimitive );
	assert( sourceEvaluator );
	PrimitiveEvaluator::ResultPtr sourceResult = sourceEvaluator->createResult();
	assert( sourceResult );

	PrimitiveEvaluatorPtr targetEvaluator = PrimitiveEvaluator::create( targetMesh );
	assert( targetEvaluator );
	PrimitiveEvaluator::ResultPtr insideResult = targetEvaluator->createResult();
	PrimitiveEvaluator::ResultPtr outsideResult = targetEvaluator->createResult();
	assert( insideResult );
	assert( outsideResult );

	PrimitiveVariableMap::const_iterator it = triangulatedSourcePrimitive->variables.find("N");
	if (it == sourceMesh->variables.end())
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"N\" in MeshPrimitiveShrinkWrapOp");
	}

	const PrimitiveVariable &nPrimVar = it->second;

	for ( typename std::vector<T>::iterator pit = vertices.begin(); pit != vertices.end(); ++pit )
	{
		T &vertexPosition = *pit;

		sourceEvaluator->closestPoint( vertexPosition, sourceResult );
		T vertexNormal = sourceResult->vectorPrimVar( nPrimVar );

		bool hit = false;

		if ( method == Inside )
		{
			hit = targetEvaluator->intersectionPoint( vertexPosition, -vertexNormal, insideResult );
			if ( hit )
			{
				vertexPosition = insideResult->point();
			}
		}
		else if ( method == Outside )
		{
			hit = targetEvaluator->intersectionPoint( vertexPosition, vertexNormal, outsideResult );
			if ( hit )
			{
				vertexPosition = outsideResult->point();
			}
		}
		else
		{
			assert( method == Both );

			bool insideHit  = targetEvaluator->intersectionPoint( vertexPosition, -vertexNormal, insideResult  );
			bool outsideHit = targetEvaluator->intersectionPoint( vertexPosition,  vertexNormal, outsideResult );

			/// Choose the closest, or the only, intersection
			if ( insideHit && outsideHit )
			{
				typename T::BaseType insideDist  = vecDistance2( vertexPosition, T( insideResult->point()  ) );
				typename T::BaseType outsideDist = vecDistance2( vertexPosition, T( outsideResult->point() ) );

				if ( insideDist < outsideDist )
				{
					vertexPosition = insideResult->point();
				}
				else
				{
					vertexPosition = outsideResult->point();
				}
			}
			else if ( insideHit )
			{
				vertexPosition = insideResult->point();
			}
			else if ( outsideHit )
			{
				vertexPosition = outsideResult->point();
			}
		}
	}
}

void MeshPrimitiveShrinkWrapOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	assert( mesh );
	assert( operands );

	if (! mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to MeshPrimitiveShrinkWrapOp");
	}

	MeshPrimitivePtr target = targetMeshParameter()->getTypedValue< MeshPrimitive >( );
	if ( !target )
	{
		return;
	}

	if ( ! target->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Target mesh with invalid primitive variables given to MeshPrimitiveShrinkWrapOp");
	}

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( target );
	target = runTimeCast< MeshPrimitive > ( op->operate() );
	assert( target );

	Method method = static_cast<Method>( m_methodParameter->getNumericValue() );

	PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
	if (it == mesh->variables.end())
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" in MeshPrimitiveShrinkWrapOp");
	}

	const DataPtr &verticesData = it->second.data;
	if (runTimeCast<V3fVectorData>(verticesData))
	{
		V3fVectorDataPtr p = runTimeCast<V3fVectorData>(verticesData);
		doShrinkWrap<V3f>( p->writable(), mesh, target, method );
	}
	else if (runTimeCast<V3dVectorData>(verticesData))
	{
		V3dVectorDataPtr p = runTimeCast<V3dVectorData>(verticesData);
		doShrinkWrap<V3d>( p->writable(), mesh, target, method );
	}
	else
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" of type V3fVectorData/V3dVectorData in MeshPrimitiveShrinkWrapOp");
	}
}
