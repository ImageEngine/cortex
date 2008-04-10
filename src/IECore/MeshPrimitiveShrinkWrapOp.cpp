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

#include "boost/format.hpp"

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

	IntParameter::PresetsMap directionPresets;
	directionPresets["Inside"] = Inside;
	directionPresets["Outside"] = Outside;
	directionPresets["Both"] = Both;

	m_directionParameter = new IntParameter(
	        "direction",
	        "The direction by which rays are cast to find the target mesh.",
	        Both,
	        Inside,
	        Both,
	        directionPresets,
	        true
	);
	
	IntParameter::PresetsMap methodPresets;
	methodPresets["Normal"] = Normal;
	methodPresets["X-axis"] = XAxis;
	methodPresets["Y-axis"] = YAxis;
	methodPresets["Z-axis"] = ZAxis;		
	methodPresets["DirectionMesh"] = DirectionMesh;	

	m_methodParameter = new IntParameter(
	        "method",
	        "The method by which find the target mesh.",
	        Normal,
	        Normal,
	        DirectionMesh,
	        methodPresets,
	        true
	);
		
	m_directionMeshParameter = new MeshPrimitiveParameter(
	        "directionMesh",
	        "The direction mesh to use when determining where to cast rays",
	        new MeshPrimitive()
	);
	
	m_triangulationToleranceParameter = new FloatParameter(
		"triangulationTolerance",
		"Set the non-planar and non-convex tolerance for the internal triangulation tests",
		1.e-6f,
		0.0f
	);

	parameters()->addParameter( m_targetMeshParameter );
	parameters()->addParameter( m_directionParameter );
	parameters()->addParameter( m_methodParameter );
	parameters()->addParameter( m_directionMeshParameter );
	parameters()->addParameter( m_triangulationToleranceParameter );
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

IntParameterPtr MeshPrimitiveShrinkWrapOp::directionParameter()
{
	return m_directionParameter;
}

ConstIntParameterPtr MeshPrimitiveShrinkWrapOp::directionParameter() const
{
	return m_directionParameter;
}

MeshPrimitiveParameterPtr MeshPrimitiveShrinkWrapOp::directionMeshParameter()
{
	return m_directionMeshParameter;
}

ConstMeshPrimitiveParameterPtr MeshPrimitiveShrinkWrapOp::directionMeshParameter() const
{
	return m_directionMeshParameter;
}

FloatParameterPtr MeshPrimitiveShrinkWrapOp::triangulationToleranceParameter()
{	
	return m_triangulationToleranceParameter;
}

ConstFloatParameterPtr MeshPrimitiveShrinkWrapOp::triangulationToleranceParameter() const
{
	return m_triangulationToleranceParameter;
}

template<typename T>
void MeshPrimitiveShrinkWrapOp::doShrinkWrap( std::vector<T> &vertices, PrimitivePtr sourceMesh, ConstPrimitivePtr targetMesh, typename TypedData< std::vector<T> >::ConstPtr directionVerticesData, Direction direction, Method method )
{
	assert( sourceMesh );

	if ( ! targetMesh )
	{
		return;
	}
	
	if ( method == DirectionMesh  )
	{
		if ( !directionVerticesData )
		{
			throw InvalidArgumentException( (boost::format("Direction mesh has no primitive variable \"P\" of type %s in MeshPrimitiveShrinkWrapOp") % TypedData< std::vector<T> >::staticTypeName() ).str() );
		}
		else if ( directionVerticesData->readable().size() != vertices.size() )
		{
			throw InvalidArgumentException("Direction mesh has incorrect number of vertices in MeshPrimitiveShrinkWrapOp" );
		}
	}

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( sourceMesh );
	op->toleranceParameter()->setNumericValue( this->triangulationToleranceParameter()->getNumericValue() );
	MeshPrimitivePtr triangulatedSourcePrimitive = runTimeCast< MeshPrimitive > ( op->operate() );

	PrimitiveEvaluatorPtr sourceEvaluator = 0;
	PrimitiveEvaluator::ResultPtr sourceResult = 0;
	
	if ( method == Normal )
	{
		sourceEvaluator = PrimitiveEvaluator::create( triangulatedSourcePrimitive );
		assert( sourceEvaluator );
		sourceResult = sourceEvaluator->createResult();
		assert( sourceResult );
	}

	PrimitiveEvaluatorPtr targetEvaluator = PrimitiveEvaluator::create( targetMesh );
	assert( targetEvaluator );
	PrimitiveEvaluator::ResultPtr insideResult = targetEvaluator->createResult();
	PrimitiveEvaluator::ResultPtr outsideResult = targetEvaluator->createResult();
	assert( insideResult );
	assert( outsideResult );

	PrimitiveVariableMap::const_iterator it = triangulatedSourcePrimitive->variables.find( "N" );
	if (it == sourceMesh->variables.end())
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"N\" in MeshPrimitiveShrinkWrapOp" );
	}

	const PrimitiveVariable &nPrimVar = it->second;

	typename std::vector<T>::size_type vertexId = 0;
	for ( typename std::vector<T>::iterator pit = vertices.begin(); pit != vertices.end(); ++pit, ++vertexId )
	{
		T &vertexPosition = *pit;

		T rayDirection;
		
		if ( method == Normal )
		{
			assert( sourceEvaluator );
			assert( sourceResult );
			sourceEvaluator->closestPoint( vertexPosition, sourceResult );
			rayDirection = sourceResult->vectorPrimVar( nPrimVar ).normalized();
		} 
		else if ( method == XAxis )
		{
			rayDirection = T( 1.0, 0.0, 0.0 );
		}
		else if ( method == YAxis )
		{
			rayDirection = T( 0.0, 1.0, 0.0 );
		}
		else if ( method == ZAxis )
		{
			rayDirection = T( 0.0, 0.0, 1.0 );
		}
		else 
		{
			assert( method == DirectionMesh );
			assert( directionVerticesData );
			assert( vertexId < directionVerticesData->readable().size() );
			
			rayDirection = ( directionVerticesData->readable()[ vertexId ] - vertexPosition ).normalized();	
		}

		bool hit = false;

		if ( direction == Inside )
		{
			hit = targetEvaluator->intersectionPoint( vertexPosition, -rayDirection, insideResult );
			if ( hit )
			{
				vertexPosition = insideResult->point();
			}
		}
		else if ( direction == Outside )
		{
			hit = targetEvaluator->intersectionPoint( vertexPosition, rayDirection, outsideResult );
			if ( hit )
			{
				vertexPosition = outsideResult->point();
			}
		}
		else
		{
			assert( direction == Both );

			bool insideHit  = targetEvaluator->intersectionPoint( vertexPosition, -rayDirection, insideResult  );
			bool outsideHit = targetEvaluator->intersectionPoint( vertexPosition,  rayDirection, outsideResult );

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
	
	PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
	if (it == mesh->variables.end())
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" in MeshPrimitiveShrinkWrapOp" );
	}
	const DataPtr &verticesData = it->second.data;

	if (! mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to MeshPrimitiveShrinkWrapOp" );
	}

	MeshPrimitivePtr target = targetMeshParameter()->getTypedValue< MeshPrimitive >( );
	if ( !target )
	{
		return;
	}

	if ( ! target->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Target mesh with invalid primitive variables given to MeshPrimitiveShrinkWrapOp" );
	}

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( target );
	target = runTimeCast< MeshPrimitive > ( op->operate() );
	assert( target );

	Direction direction = static_cast<Direction>( m_directionParameter->getNumericValue() );
	Method method = static_cast<Method>( m_methodParameter->getNumericValue() );
	
	MeshPrimitivePtr directionMesh = 0;
	
	ConstDataPtr directionVerticesData = 0;
	if ( method == DirectionMesh )
	{
		directionMesh = directionMeshParameter()->getTypedValue< MeshPrimitive >( );
				
		if ( ! directionMesh )
		{
			throw InvalidArgumentException( "No direction mesh given to MeshPrimitiveShrinkWrapOp" );
		}
		
		if ( ! directionMesh->arePrimitiveVariablesValid() )
		{
			throw InvalidArgumentException( "Direction mesh with invalid primitive variables given to MeshPrimitiveShrinkWrapOp" );
		}
		
		PrimitiveVariableMap::const_iterator it = directionMesh->variables.find("P");
		if (it == directionMesh->variables.end())
		{
			throw InvalidArgumentException("Direction mesh has no primitive variable \"P\" in MeshPrimitiveShrinkWrapOp" );
		}
		
		directionVerticesData = it->second.data;	
	}
	
	
	/// \todo Start using depatchTypedData
	if (runTimeCast<V3fVectorData>(verticesData))
	{
		V3fVectorDataPtr p = runTimeCast<V3fVectorData>(verticesData);
		doShrinkWrap<V3f>( p->writable(), mesh, target, runTimeCast<const V3fVectorData>(directionVerticesData), direction, method );
	}
	else if (runTimeCast<V3dVectorData>(verticesData))
	{
		V3dVectorDataPtr p = runTimeCast<V3dVectorData>(verticesData);
		doShrinkWrap<V3d>( p->writable(), mesh, target, runTimeCast<const V3dVectorData>(directionVerticesData), direction, method );
	}
	else
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" of type V3fVectorData/V3dVectorData in MeshPrimitiveShrinkWrapOp" );
	}
}
