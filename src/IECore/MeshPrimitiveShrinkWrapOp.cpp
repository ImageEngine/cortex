//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace Imath;


MeshPrimitiveShrinkWrapOp::MeshPrimitiveShrinkWrapOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to shrink-wrap one mesh onto another" )
{
	m_targetMeshParameter = new MeshPrimitiveParameter(
	        "target",
	        "The target mesh to shrink-wrap onto.",
	        new MeshPrimitive()
	);

	IntParameter::PresetsContainer directionPresets;
	directionPresets.push_back( IntParameter::Preset( "Inside", Inside ) );
	directionPresets.push_back( IntParameter::Preset( "Outside", Outside ) );
	directionPresets.push_back( IntParameter::Preset( "Both", Both ) );

	m_directionParameter = new IntParameter(
	        "direction",
	        "The direction by which rays are cast to find the target mesh.",
	        Both,
	        Inside,
	        Both,
	        directionPresets,
	        true
	);
	
	IntParameter::PresetsContainer methodPresets;
	methodPresets.push_back( IntParameter::Preset( "Normal", Normal ) );
	methodPresets.push_back( IntParameter::Preset( "X-axis", XAxis ) );
	methodPresets.push_back( IntParameter::Preset( "Y-axis", YAxis ) );
	methodPresets.push_back( IntParameter::Preset( "Z-axis", ZAxis ) );
	methodPresets.push_back( IntParameter::Preset( "DirectionMesh", DirectionMesh ) );

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

struct MeshPrimitiveShrinkWrapOp::ShrinkWrapFn
{
	typedef void ReturnType;
	
	PrimitivePtr m_sourceMesh;
	ConstPrimitivePtr m_targetMesh;
	ConstDataPtr m_directionData;
	Direction m_direction;
	Method m_method;
	float m_tolerance;
	
	ShrinkWrapFn( PrimitivePtr sourceMesh, ConstPrimitivePtr targetMesh, ConstDataPtr directionData, Direction direction, Method method, float tolerance )
	: m_sourceMesh( sourceMesh ), m_targetMesh( targetMesh ), m_directionData( directionData ), m_direction( direction ), m_method( method ), m_tolerance( tolerance )
	{
	}

	template<typename T>
	void operator()( typename T::Ptr vertexData ) const
	{
		assert( vertexData );
		
		typedef typename T::ValueType::value_type Vec;

		assert( m_sourceMesh );

		if ( ! m_targetMesh )
		{
			return;
		}

		typename T::ValueType &vertices = vertexData->writable();
		
		typename T::ConstPtr directionVerticesData = runTimeCast<const T>( m_directionData );

		if ( m_method == DirectionMesh  )
		{
			if ( !directionVerticesData )
			{
				throw InvalidArgumentException( (boost::format("MeshPrimitiveShrinkWrapOp: Direction mesh has no primitive variable \"P\" of type \"%s\" ") % T::staticTypeName() ).str() );
			}
			else if ( directionVerticesData->readable().size() != vertices.size() )
			{
				throw InvalidArgumentException("MeshPrimitiveShrinkWrapOp: Direction mesh has incorrect number of vertices" );
			}
		}

		TriangulateOpPtr op = new TriangulateOp();
		op->inputParameter()->setValue( m_sourceMesh );
		op->toleranceParameter()->setNumericValue( m_tolerance );
		MeshPrimitivePtr triangulatedSourcePrimitive = runTimeCast< MeshPrimitive > ( op->operate() );

		PrimitiveEvaluatorPtr sourceEvaluator = 0;
		PrimitiveEvaluator::ResultPtr sourceResult = 0;

		if ( m_method == Normal )
		{
			sourceEvaluator = PrimitiveEvaluator::create( triangulatedSourcePrimitive );
			assert( sourceEvaluator );
			sourceResult = sourceEvaluator->createResult();
			assert( sourceResult );
		}

		PrimitiveEvaluatorPtr targetEvaluator = PrimitiveEvaluator::create( m_targetMesh );
		assert( targetEvaluator );
		PrimitiveEvaluator::ResultPtr insideResult = targetEvaluator->createResult();
		PrimitiveEvaluator::ResultPtr outsideResult = targetEvaluator->createResult();
		assert( insideResult );
		assert( outsideResult );

		PrimitiveVariableMap::const_iterator it = triangulatedSourcePrimitive->variables.find( "N" );
		if (it == m_sourceMesh->variables.end())
		{
			throw InvalidArgumentException("MeshPrimitiveShrinkWrapOp: MeshPrimitive has no primitive variable \"N\"" );
		}

		const PrimitiveVariable &nPrimVar = it->second;	

		typename T::ValueType::size_type vertexId = 0;
		for ( typename T::ValueType::iterator pit = vertices.begin(); pit != vertices.end(); ++pit, ++vertexId )
		{
			Vec &vertexPosition = *pit;

			Vec rayDirection;

			if ( m_method == Normal )
			{
				assert( sourceEvaluator );
				assert( sourceResult );
				sourceEvaluator->closestPoint( vertexPosition, sourceResult );
				rayDirection = sourceResult->vectorPrimVar( nPrimVar ).normalized();
			} 
			else if ( m_method == XAxis )
			{
				rayDirection = Vec( 1.0, 0.0, 0.0 );
			}
			else if ( m_method == YAxis )
			{
				rayDirection = Vec( 0.0, 1.0, 0.0 );
			}
			else if ( m_method == ZAxis )
			{
				rayDirection = Vec( 0.0, 0.0, 1.0 );
			}
			else 
			{
				assert( m_method == DirectionMesh );
				assert( directionVerticesData );
				assert( vertexId < directionVerticesData->readable().size() );

				rayDirection = ( directionVerticesData->readable()[ vertexId ] - vertexPosition ).normalized();	
			}

			bool hit = false;

			if ( m_direction == Inside )
			{
				hit = targetEvaluator->intersectionPoint( vertexPosition, -rayDirection, insideResult );
				if ( hit )
				{
					vertexPosition = insideResult->point();
				}
			}
			else if ( m_direction == Outside )
			{
				hit = targetEvaluator->intersectionPoint( vertexPosition, rayDirection, outsideResult );
				if ( hit )
				{
					vertexPosition = outsideResult->point();
				}
			}
			else
			{
				assert( m_direction == Both );

				bool insideHit  = targetEvaluator->intersectionPoint( vertexPosition, -rayDirection, insideResult  );
				bool outsideHit = targetEvaluator->intersectionPoint( vertexPosition,  rayDirection, outsideResult );

				/// Choose the closest, or the only, intersection
				if ( insideHit && outsideHit )
				{
					typename Vec::BaseType insideDist  = vecDistance2( vertexPosition, Vec( insideResult->point()  ) );
					typename Vec::BaseType outsideDist = vecDistance2( vertexPosition, Vec( outsideResult->point() ) );

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
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );
		
			throw InvalidArgumentException( ( boost::format( "MeshPrimitiveShrinkWrapOp: Invalid data type \"%s\" for primitive variable \"P\"." ) % Object::typeNameFromTypeId( data->typeId() ) ).str() );		
		}
	};
};

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
	
	ShrinkWrapFn fn( mesh, target, directionVerticesData, direction, method, this->triangulationToleranceParameter()->getNumericValue() );
	despatchTypedData< ShrinkWrapFn, TypeTraits::IsVec3VectorTypedData, ShrinkWrapFn::ErrorHandler >( verticesData, fn );	
}
