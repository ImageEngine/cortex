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


#ifndef IE_CORE_RADIXSORTTEST_H
#define IE_CORE_RADIXSORTTEST_H

#include <math.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "boost/random.hpp"

#include "IECore/RadixSort.h"

using namespace Imath;

namespace IECore
{

void addRadixSortTest( boost::unit_test::test_suite* test );

struct RadixSortTest
{
	template<typename T>
	void test()
	{
		unsigned seed = 42;
		boost::mt19937 generator( static_cast<boost::mt19937::result_type>( seed ) );
		
		boost::uniform_real<> uni_dist( std::numeric_limits<T>::min(), std::numeric_limits<T>::max() );
		boost::variate_generator<boost::mt19937&, boost::uniform_real<> > uni( generator, uni_dist );

		// Run 50 tests, each sorting 100000 numbers
		const unsigned numTests = 50u;
		const unsigned numValuesPerTest = 100000u;
		
		for ( unsigned i = 0; i < numTests; i ++ )
		{
			std::vector<T> input;

			for ( unsigned n = 0; n < numValuesPerTest; n++ )
			{
				input.push_back( static_cast<T>( uni() ) );
			}

			RadixSort sorter;

			const std::vector<unsigned int> &indices = sorter( input );
			BOOST_CHECK_EQUAL( indices.size(), numValuesPerTest );

			for ( unsigned n = 1; n < numValuesPerTest; n++ )
			{
				BOOST_CHECK(( input[ indices[n] ] >= input[ indices[n - 1] ] ) );
			}
		}
	}
};

struct RadixSortTestSuite : public boost::unit_test::test_suite
{

	RadixSortTestSuite() : boost::unit_test::test_suite( "RadixSortTestSuite" )
	{
		static boost::shared_ptr<RadixSortTest> instance( new RadixSortTest() );

		add( BOOST_CLASS_TEST_CASE( &RadixSortTest::test<float>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &RadixSortTest::test<unsigned int>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &RadixSortTest::test<int>, instance ) );
	}

};
}


#endif // IE_CORE_RADIXSORTTEST_H
