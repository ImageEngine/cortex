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

#include <iostream>
#include <algorithm>
#include <cassert>

#include "IECore/Random.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/BoundedKDTree.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/DespatchTypedData.h"

#include "IECore/UniformRandomPointDistributionOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( UniformRandomPointDistributionOp );

UniformRandomPointDistributionOp::UniformRandomPointDistributionOp()
	:	Op(
		staticTypeName(),
		"The UniformRandomPointDistributionOp distributes points over a mesh using a random distribution.",
		new PointsPrimitiveParameter(
			"result",
			"Resulting points distributed over mesh.",
			new PointsPrimitive()
		)
	)
{
	constructCommon();
}

UniformRandomPointDistributionOp::UniformRandomPointDistributionOp( const std::string &name, const std::string &description )
	: Op(
		name,
		description,
		new PointsPrimitiveParameter(
			"result",
			"Resulting points distributed over mesh.",
			new PointsPrimitive()
		)
	)
{
	constructCommon();
}

void UniformRandomPointDistributionOp::constructCommon()
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh to distribute points over.",
		new MeshPrimitive()
	);

	m_numPointsParameter = new IntParameter(
		"numPoints",
		"The number of points to distribute",
		100,
		0
	);

	m_seedParameter = new IntParameter(
		"seed",
		"The random number seed",
		1,
		0
	);

	m_addSTParameter = new BoolParameter(
		"addST",
		"Adds the point primitive variables s and t representing the corresponding coordinates on the mesh, if enabled",
		false
	);

	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_numPointsParameter );
	parameters()->addParameter( m_seedParameter );
	parameters()->addParameter( m_addSTParameter );
}



UniformRandomPointDistributionOp::~UniformRandomPointDistributionOp()
{
}

MeshPrimitiveParameterPtr UniformRandomPointDistributionOp::meshParameter()
{
	return m_meshParameter;
}

ConstMeshPrimitiveParameterPtr UniformRandomPointDistributionOp::meshParameter() const
{
	return m_meshParameter;
}

IntParameterPtr UniformRandomPointDistributionOp::numPointsParameter()
{
	return m_numPointsParameter;
}

ConstIntParameterPtr UniformRandomPointDistributionOp::numPointsParameter() const
{
	return m_numPointsParameter;
}

IntParameterPtr UniformRandomPointDistributionOp::seedParameter()
{
	return m_seedParameter;
}

ConstIntParameterPtr UniformRandomPointDistributionOp::seedParameter() const
{
	return m_seedParameter;
}

BoolParameterPtr UniformRandomPointDistributionOp::addSTParameter()
{
	return m_addSTParameter;
}

ConstBoolParameterPtr UniformRandomPointDistributionOp::addSTParameter() const
{
	return m_addSTParameter;
}

float UniformRandomPointDistributionOp::density( ConstMeshPrimitivePtr mesh, const Imath::V3f &point, const Imath::V2f &uv ) const
{
	return 1.0f;
}

struct UniformRandomPointDistributionOp::DistributeFn
{
	typedef PointsPrimitivePtr ReturnType;

	ConstUniformRandomPointDistributionOpPtr m_op;
	ConstMeshPrimitivePtr m_mesh;
	const int m_numPoints;
	const int m_seed;
	FloatVectorDataPtr m_sData, m_tData;
	bool m_addST;

	DistributeFn( UniformRandomPointDistributionOpPtr op, ConstMeshPrimitivePtr mesh, int numPoints, int seed, FloatVectorDataPtr sData, FloatVectorDataPtr tData, bool addST )
	: m_op( op ), m_mesh( mesh ), m_numPoints( numPoints ), m_seed( seed ), m_sData( sData ), m_tData( tData ), m_addST( addST )
	{
		assert( m_mesh );
	}

	template<typename T>
	ReturnType operator()( typename T::ConstPtr p ) const
	{
		typedef typename T::ValueType::value_type Vec;
		typedef std::vector<double> ProbabilityVector;

		assert( p );
		PointsPrimitivePtr points = new PointsPrimitive();

		assert( m_mesh );
		ConstIntVectorDataPtr vertexIds = m_mesh->vertexIds();
		assert( vertexIds );

		/// Iterate over each triangle, building a cumulative area list. We then normalize this list with the total triangle area
		/// and later access it as a list of cumulative probabilities.
		ProbabilityVector cummulativeProbabilities;
		double totalArea = 0.0;
		for ( IntVectorData::ValueType::const_iterator it = vertexIds->readable().begin(); it != vertexIds->readable().end(); )
		{
			const int &v0 = *it++;
			const int &v1 = *it++;
			const int &v2 = *it++;

			const Vec &p0 = p->readable()[ v0 ];
			const Vec &p1 = p->readable()[ v1 ];
			const Vec &p2 = p->readable()[ v2 ];

			totalArea += triangleArea<Vec>( p0, p1, p2 );

			cummulativeProbabilities.push_back( totalArea );
		}

		/// If the total area is effectively zero, we don't have any space to distribute points over!
		if ( totalArea < 1.e-06 )
		{
			return points;
		}

		for ( ProbabilityVector::iterator it = cummulativeProbabilities.begin(); it != cummulativeProbabilities.end(); ++it )
		{
			*it /= totalArea;
		}

		V3fVectorDataPtr positions = new V3fVectorData();
		points->variables.insert( PrimitiveVariableMap::value_type( "P", PrimitiveVariable( PrimitiveVariable::Vertex, positions ) ) );

		FloatVectorDataPtr sData = 0, tData = 0;
		if ( m_addST )
		{
			sData = new FloatVectorData();
			points->variables.insert( PrimitiveVariableMap::value_type( "s", PrimitiveVariable( PrimitiveVariable::Vertex, sData ) ) );

			tData = new FloatVectorData();
			points->variables.insert( PrimitiveVariableMap::value_type( "t", PrimitiveVariable( PrimitiveVariable::Vertex, tData ) ) );

			assert( sData );
			assert( tData );
		}

		Rand48 generator( m_seed );

		points->setNumPoints( m_numPoints );

		int i = 0;
		int numTrialsSinceLastPoint = 0;
		while ( i < m_numPoints )
		{
			numTrialsSinceLastPoint ++;

			/// Check every million trials if we actually emitted anything, to ensure we don't go into
			/// an infinite loop (e.g. if density returns 0.0)
			if ( numTrialsSinceLastPoint % 1000000 == 0 )
			{
				if ( (float)i / (float)numTrialsSinceLastPoint  < 1.e-10 )
				{
					/// \todo Warn

					points->setNumPoints( positions->readable().size() );
					return points;
				}
			}

			/// Pick a triangle, weighted by its area
			ProbabilityVector::iterator probabilityIt = lower_bound( cummulativeProbabilities.begin(), cummulativeProbabilities.end(), generator.nextf() );
			ProbabilityVector::size_type triangleIndex = std::distance( cummulativeProbabilities.begin(), probabilityIt );

			/// Generate a random point on that triangle
			const int &v0 = vertexIds->readable()[ triangleIndex*3 + 0 ];
			const int &v1 = vertexIds->readable()[ triangleIndex*3 + 1 ];
			const int &v2 = vertexIds->readable()[ triangleIndex*3 + 2 ];

			const Vec &p0 = p->readable()[ v0 ];
			const Vec &p1 = p->readable()[ v1 ];
			const Vec &p2 = p->readable()[ v2 ];

			V3f bary = barycentricRand<V3f, Rand48>( generator );

			/// Add the point to the primitive
			V3f p = p0*bary.x + p1*bary.y + p2*bary.z;


			float s = 0.0f;
			if ( m_sData )
			{
				const float &s0 = m_sData->readable()[ triangleIndex*3 + 0 ];
				const float &s1 = m_sData->readable()[ triangleIndex*3 + 1 ];
				const float &s2 = m_sData->readable()[ triangleIndex*3 + 2 ];

				s = s0*bary.x + s1*bary.y + s2*bary.z;
			}

			float t = 0.0f;
			if ( m_tData )
			{
				const float &t0 = m_tData->readable()[ triangleIndex*3 + 0 ];
				const float &t1 = m_tData->readable()[ triangleIndex*3 + 1 ];
				const float &t2 = m_tData->readable()[ triangleIndex*3 + 2 ];

				t = t0*bary.x + t1*bary.y + t2*bary.z;
			}

			float d = m_op->density( m_mesh, p, Imath::V2f( s, t ) );
			assert( d >= 0.0f );
			assert( d <= 1.0f );

			if ( generator.nextf() <= d )
			{
				positions->writable().push_back( p );

				if ( m_addST )
				{
					assert( sData );
					assert( tData );

					sData->writable().push_back( s );
					tData->writable().push_back( t );
				}

				i ++;
				numTrialsSinceLastPoint = 0;
			}
		}

		return points;
	}

	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );

			throw InvalidArgumentException( ( boost::format( "UniformRandomPointDistributionOp: Invalid data type \"%s\" for mesh primitive variable \"P\"." ) % Object::typeNameFromTypeId( data->typeId() ) ).str() );
		}
	};
};

ObjectPtr UniformRandomPointDistributionOp::doOperation( ConstCompoundObjectPtr operands )
{
	MeshPrimitivePtr mesh = m_meshParameter->getTypedValue<MeshPrimitive>();
	assert( mesh );

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( mesh );
	op->toleranceParameter()->setNumericValue( 1.e-3 );
        mesh = runTimeCast< MeshPrimitive > ( op->operate() );
	assert( mesh );

	bool addST = m_addSTParameter->getTypedValue();

	FloatVectorDataPtr sData = 0, tData = 0;

	PrimitiveVariableMap::const_iterator sIt = mesh->variables.find("s");
	if ( sIt != mesh->variables.end() )
	{
		if ( sIt->second.interpolation != PrimitiveVariable::FaceVarying )
		{
			throw InvalidArgumentException( "UniformRandomPointDistributionOp: Primitive variable 's' must have facevarying interpolation" );
		}
		sData = runTimeCast<FloatVectorData>( sIt->second.data );
	}

	PrimitiveVariableMap::const_iterator tIt = mesh->variables.find("t");
	if ( tIt != mesh->variables.end() )
	{
		if ( tIt->second.interpolation != PrimitiveVariable::FaceVarying )
		{
			throw InvalidArgumentException( "UniformRandomPointDistributionOp: Primitive variable 't' must have facevarying interpolation" );
		}
		tData = runTimeCast<FloatVectorData>( tIt->second.data );
	}

	if ( addST && ( !sData || !tData ) )
	{
		throw InvalidArgumentException( "blah" );
	}

	DistributeFn fn( this, mesh, m_numPointsParameter->getNumericValue(), m_seedParameter->getNumericValue(), sData, tData, addST );
	return despatchTypedData< DistributeFn, TypeTraits::IsVec3VectorTypedData, DistributeFn::ErrorHandler >( mesh->variables["P"].data, fn );
}
