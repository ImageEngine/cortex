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

#include <iostream>

#include "tbb/tbb.h"

#include "OpenEXR/ImathRandom.h"

#include "IECore/CurvesPrimitiveEvaluator.h"
#include "IECore/CurvesPrimitive.h"

#include "CurvesPrimitiveEvaluatorThreadingTest.h"

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;
using namespace Imath;

namespace IECore
{

struct CurvesPrimitiveEvaluatorThreadingTest
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

	void testResultCreation()
	{
		CurvesPrimitiveEvaluatorPtr evaluator = makeEvaluator();
		unsigned pRefCount = evaluator->primitive()->variableData<Data>( "P" )->refCount();
		parallel_for( blocked_range<size_t>( 0, 1000000 ), CreateResultAndQueryPointAtV( *evaluator ) );
		BOOST_CHECK_EQUAL( pRefCount, evaluator->primitive()->variableData<Data>( "P" )->refCount() );
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
					// it would be preferable to call BOOST_CHECK in here instead
					// of throwing exceptions on errors, but as far as i can tell
					// BOOST_CHECK isn't threadsafe.
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

	void testClosestPoint()
	{
		CurvesPrimitiveEvaluatorPtr evaluator = makeEvaluator();
		parallel_for( blocked_range<size_t>( 0, 10000 ), CheckClosestPoint( *evaluator ) );
	}

};


struct CurvesPrimitiveEvaluatorThreadingTestSuite : public boost::unit_test::test_suite
{

	CurvesPrimitiveEvaluatorThreadingTestSuite() : boost::unit_test::test_suite( "CurvesPrimitiveEvaluatorThreadingTestSuite" )
	{
		boost::shared_ptr<CurvesPrimitiveEvaluatorThreadingTest> instance( new CurvesPrimitiveEvaluatorThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &CurvesPrimitiveEvaluatorThreadingTest::testResultCreation, instance ) );
		add( BOOST_CLASS_TEST_CASE( &CurvesPrimitiveEvaluatorThreadingTest::testClosestPoint, instance ) );
	}
};

void addCurvesPrimitiveEvaluatorThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new CurvesPrimitiveEvaluatorThreadingTestSuite( ) );
}

} // namespace IECore
