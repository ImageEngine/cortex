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


#ifndef IE_CORE_SWEEPANDPRUNETEST_H
#define IE_CORE_SWEEPANDPRUNETEST_H

#include <math.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/random.hpp"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "IECore/SweepAndPrune.h"
#include "IECore/BoxTraits.h"
#include "IECore/VectorTraits.h"
#include "IECore/VectorOps.h"

using namespace Imath;

namespace IECore
{

void addSweepAndPruneTest( boost::unit_test::test_suite* test );

struct SweepAndPruneTest
{
	template<typename BoundIterator>
	struct TestCallback
	{
		typedef std::set< std::pair< unsigned int, unsigned int > > IntersectingBoundIndices;
		IntersectingBoundIndices m_indices;
				
		BoundIterator m_begin;	
		unsigned int m_numBoxesPerTest;
	
		TestCallback( BoundIterator begin, unsigned int numBoxesPerTest ) : m_begin( begin ), m_numBoxesPerTest( numBoxesPerTest )
		{			
		}
			
		void operator()( BoundIterator b1, BoundIterator b2 )
		{
			BOOST_CHECK( b1->intersects( *b2 ) );	
			
			unsigned int idx0 = std::distance( m_begin, b1 );
			unsigned int idx1 = std::distance( m_begin, b2 );

			BOOST_CHECK( idx0 < m_numBoxesPerTest );
			BOOST_CHECK( idx1 < m_numBoxesPerTest );				

			BOOST_CHECK( idx0 != idx1 );				
#ifndef NDEBUG				
			unsigned oldSize = m_indices.size();
#endif				

			m_indices.insert( IntersectingBoundIndices::value_type( idx0, idx1 ) );			
			m_indices.insert( IntersectingBoundIndices::value_type( idx1, idx0 ) );							

			assert( m_indices.size() == oldSize + 2);
		}
	};

	template<typename T>
	void test()
	{
		unsigned seed = 42;
		boost::mt19937 generator( static_cast<boost::mt19937::result_type>( seed ) );
		
		typedef typename BoxTraits<T>::BaseType VecType;
		typedef typename VectorTraits<VecType>::BaseType BaseType;
		
		boost::uniform_real<> uni_dist( 0.0f, 1.0f );
		boost::variate_generator<boost::mt19937&, boost::uniform_real<> > uni( generator, uni_dist );

		// Run 50 tests, each intersecting 1000 boxes within a random 5x5x5 world
		const unsigned numTests = 10u;
		const unsigned numBoxesPerTest = 1000u;
		const unsigned numPostChecksPerTest = numBoxesPerTest;
		
		for ( unsigned i = 0; i < numTests; i ++ )
		{
			std::vector<T> input;

			for ( unsigned n = 0; n < numBoxesPerTest; n++ )
			{
				T b;
				
				VecType corner( uni() * 5.0, uni() * 5.0, uni() * 5.0 );
				VecType size( uni(), uni(), uni() ); 
				
				b.extendBy( corner );
				b.extendBy( corner + size );	
							
				input.push_back( b );
			}

			typedef typename std::vector<T>::iterator BoundIterator;
			
			typedef SweepAndPrune<BoundIterator, TestCallback> SAP;
						
			SAP sap;			
			typename SAP::Callback cb( input.begin(), numBoxesPerTest );

			sap.intersectingBounds( input.begin(), input.end(), cb, SAP::XZY );
												
			/// Pick some random box pairs
			for ( unsigned n = 0; n < numPostChecksPerTest; n++ )
			{
				unsigned int idx0 = (unsigned int)( uni() * numBoxesPerTest );
				unsigned int idx1 = 0;
				do
				{
					idx1 = (unsigned int)( uni() * numBoxesPerTest );
				} 
				while ( idx0 == idx1 );
				
				assert( idx0 != idx1 );
				assert( idx0 < numBoxesPerTest );
				assert( idx1 < numBoxesPerTest );				
				
				/// If SweepAndPrune hasn't determined this pair is intersecting, verify it actually is not.								
				if ( cb.m_indices.find( typename TestCallback<BoundIterator>::IntersectingBoundIndices::value_type( idx0, idx1 ) ) == cb.m_indices.end() )
				{
					const T &bound0 = input[ idx0 ];
					const T &bound1 = input[ idx1 ];
					
					BOOST_CHECK( ! bound0.intersects( bound1 ) );
				}				
			}
		}
	}
};

struct SweepAndPruneTestSuite : public boost::unit_test::test_suite
{

	SweepAndPruneTestSuite() : boost::unit_test::test_suite( "SweepAndPruneTestSuite" )
	{
		static boost::shared_ptr<SweepAndPruneTest> instance( new SweepAndPruneTest() );

		add( BOOST_CLASS_TEST_CASE( &SweepAndPruneTest::test<Imath::Box3f>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SweepAndPruneTest::test<Imath::Box3d>, instance ) );		
	}

};
}


#endif // IE_CORE_SWEEPANDPRUNETEST_H
