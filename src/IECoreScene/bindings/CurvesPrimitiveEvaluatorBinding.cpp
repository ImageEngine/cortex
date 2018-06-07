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

#include "CurvesPrimitiveEvaluatorBinding.h"

#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/CurvesPrimitiveEvaluator.h"

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "OpenEXR/ImathRandom.h"

#include "tbb/tbb.h"

using namespace tbb;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Wrappers
//////////////////////////////////////////////////////////////////////////

namespace
{

bool pointAtV( const CurvesPrimitiveEvaluator &e, unsigned curveIndex, float v, PrimitiveEvaluator::Result *r )
{
	e.validateResult( r );
	return e.pointAtV( curveIndex, v, r );
}

IntVectorDataPtr verticesPerCurve( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.verticesPerCurve() );
}

IntVectorDataPtr vertexDataOffsets( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.vertexDataOffsets() );
}

IntVectorDataPtr varyingDataOffsets( const CurvesPrimitiveEvaluator &e )
{
	return new IntVectorData( e.varyingDataOffsets() );
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////

namespace
{

static const unsigned g_numCurves = 10000;

CurvesPrimitiveEvaluatorPtr makeEvaluator()
{
	Rand32 rand;

	IntVectorDataPtr vertsPerCurveData = new IntVectorData;
	std::vector<int> &vertsPerCurve = vertsPerCurveData->writable();
	V3fVectorDataPtr pointsData = new V3fVectorData;
	std::vector<V3f> &points = pointsData->writable();
	for( unsigned curveIndex = 0; curveIndex < g_numCurves; curveIndex++ )
	{
		unsigned numVerts = 2 + rand.nexti() % 10;
		vertsPerCurve.push_back( numVerts );
		for( unsigned vertIndex=0; vertIndex<numVerts; vertIndex++ )
		{
			points.push_back( V3f( rand.nextf(), rand.nextf(), rand.nextf() ) );
		}
	}

	CurvesPrimitivePtr curves = new CurvesPrimitive( vertsPerCurveData, CubicBasisf::linear(), false, pointsData );
	return new CurvesPrimitiveEvaluator( curves );
}

struct CreateResultAndQueryPointAtV
{
	public :

		CreateResultAndQueryPointAtV( CurvesPrimitiveEvaluator &evaluator )
			:	m_evaluator( evaluator )
		{
		}

		void operator()( const blocked_range<size_t> &r ) const
		{
			for( size_t i=r.begin(); i!=r.end(); ++i )
			{
				PrimitiveEvaluator::ResultPtr result = m_evaluator.createResult();
				unsigned curveIndex = i % g_numCurves;
				m_evaluator.pointAtV( curveIndex, 0.5, result.get() );
			}
		}

	private :

		CurvesPrimitiveEvaluator &m_evaluator;

};

void testCurvesPrimitiveEvaluatorParallelResultCreation()
{
	CurvesPrimitiveEvaluatorPtr evaluator = makeEvaluator();
	const size_t pRefCount = evaluator->primitive()->variableData<Data>( "P" )->refCount();
	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
	parallel_for( blocked_range<size_t>( 0, 1000000 ), CreateResultAndQueryPointAtV( *evaluator ), taskGroupContext );
	if( pRefCount != evaluator->primitive()->variableData<Data>( "P" )->refCount() )
	{
		throw Exception( "Unexpected reference count." );
	}
}

struct CheckClosestPoint
{
	public :

		CheckClosestPoint( CurvesPrimitiveEvaluator &evaluator )
			:	m_evaluator( evaluator )
		{
		}

		void operator()( const blocked_range<size_t> &r ) const
		{
			PrimitiveEvaluator::ResultPtr result = m_evaluator.createResult();
			for( size_t i=r.begin(); i!=r.end(); ++i )
			{
				unsigned curveIndex = i % g_numCurves;
				bool ok = m_evaluator.pointAtV( curveIndex, 0.5, result.get() );
				if( !ok )
				{
					throw Exception( "Not OK." );
				}
				V3f p = result->point();
				ok = m_evaluator.closestPoint( p, result.get() );
				if( !ok )
				{
					throw Exception( "Not OK." );
				}
				if( fabs( (p-result->point()).length() ) > 0.001 )
				{
					std::cerr << p << " | " << result->point() << std::endl;
					throw Exception( "Closest point not close enough." );
				}
			}
		}

	private :

		CurvesPrimitiveEvaluator &m_evaluator;

};

void testCurvesPrimitiveEvaluatorParallelClosestPoint()
{
	CurvesPrimitiveEvaluatorPtr evaluator = makeEvaluator();
	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
	parallel_for( blocked_range<size_t>( 0, 10000 ), CheckClosestPoint( *evaluator ), taskGroupContext );
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

namespace IECoreSceneModule
{

void bindCurvesPrimitiveEvaluator()
{

	/// \todo Move these to an IECoreSceneTest module
	def( "testCurvesPrimitiveEvaluatorParallelResultCreation", &testCurvesPrimitiveEvaluatorParallelResultCreation );
	def( "testCurvesPrimitiveEvaluatorParallelClosestPoint", &testCurvesPrimitiveEvaluatorParallelClosestPoint );

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
