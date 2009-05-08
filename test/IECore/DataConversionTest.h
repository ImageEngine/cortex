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

#ifndef IE_CORE_DATACONVERSIONTEST_H
#define IE_CORE_DATACONVERSIONTEST_H

#include <cassert>
#include <limits.h>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/random.hpp"

#include "IECore/HalfTypeTraits.h"
#include "IECore/IECore.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/CineonToLinearDataConversion.h"
#include "IECore/LinearToCineonDataConversion.h"
#include "IECore/SRGBToLinearDataConversion.h"
#include "IECore/LinearToSRGBDataConversion.h"
#include "IECore/Rec709ToLinearDataConversion.h"
#include "IECore/LinearToRec709DataConversion.h"
#include "IECore/CompoundDataConversion.h"

using namespace Imath;

namespace IECore
{

void addDataConversionTest(boost::unit_test::test_suite* test);

struct DataConversionTest
{
	template<typename F, typename T>
	void testCineonLinear()
	{
		typedef CineonToLinearDataConversion< F, T > Func;

		Func f;
		CompoundDataConversion< Func, typename Func::InverseType > f_fi( f, f.inverse() );

		/// Verify that f(f'(i)) == i
		for ( F i = 0; i < 1024; i++ )
		{
			BOOST_CHECK_EQUAL( f_fi(i), i );
		}
	}

	template<typename T>
	void testSRGBLinear()
	{
		typedef SRGBToLinearDataConversion< T, T > Func;
		CompoundDataConversion< Func, typename Func::InverseType > f;
		typename CompoundDataConversion< Func, typename Func::InverseType >::InverseType fi = f.inverse();

		/// Verify that f(f'(i)) ~ i
		for ( T i = T(0); i < T(10); i += T(0.2) )
		{
			BOOST_CHECK_CLOSE( double( f(i) ), double( i ), 1.e-4 );
			BOOST_CHECK_CLOSE( double( fi(i) ), double( i ), 1.e-4 );
		}
	}

	template<typename T>
	void testRec709Linear()
	{
		typedef Rec709ToLinearDataConversion< T, T > Func;
		CompoundDataConversion< Func, typename Func::InverseType > f;
		typename CompoundDataConversion< Func, typename Func::InverseType >::InverseType fi = f.inverse();

		/// Verify that f(f'(i)) ~ i
		for ( T i = T(0); i < T(10); i += T(0.2) )
		{
			BOOST_CHECK_CLOSE( double( f(i) ), double( i ), 1.e-4 );
			BOOST_CHECK_CLOSE( double( fi(i) ), double( i ), 1.e-4 );
		}
	}

	template<typename F, typename T>
	void testSignedScaled()
	{
		typedef ScaledDataConversion< F, T > Func;

		Func f;
		CompoundDataConversion< Func, typename Func::InverseType > f_fi( f, f.inverse() );

		unsigned seed = 42;
		boost::mt19937 generator( static_cast<boost::mt19937::result_type>( seed ) );

		/// Create a random number generator within this range
		boost::uniform_real<> uni_dist( std::numeric_limits<F>::min(), std::numeric_limits<F>::max() );
		boost::variate_generator<boost::mt19937&, boost::uniform_real<> > uni(generator, uni_dist);

		const int numTests = 100;
		for ( int t = 0; t < numTests; t++)
		{
			F i = static_cast<F>( uni() );

			/// Verify that f(f'(i)) ~ i
			BOOST_CHECK_CLOSE( double( f_fi(i) ), double( i ), 1.e-4 );
		}
	}
};

struct DataConversionTestSuite : public boost::unit_test::test_suite
{

	DataConversionTestSuite() : boost::unit_test::test_suite( "DataConversionTestSuite" )
	{
		static boost::shared_ptr<DataConversionTest> instance( new DataConversionTest() );

		testCineonLinear( instance );
		testSRGBLinear( instance );
		testRec709Linear( instance );
		testSignedScaled( instance );
	}

	void testCineonLinear( boost::shared_ptr<DataConversionTest> instance )
	{
		void (DataConversionTest::*fn)() = 0;

		fn = &DataConversionTest::testCineonLinear<unsigned, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testCineonLinear<unsigned, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testCineonLinear<short, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testCineonLinear<short, half>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
	}

	void testSRGBLinear( boost::shared_ptr<DataConversionTest> instance )
	{
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testSRGBLinear<float>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testSRGBLinear<double>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testSRGBLinear<half>, instance ) );
	}

	void testRec709Linear( boost::shared_ptr<DataConversionTest> instance )
	{
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testRec709Linear<float>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testRec709Linear<double>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &DataConversionTest::testRec709Linear<half>, instance ) );
	}

	void testSignedScaled( boost::shared_ptr<DataConversionTest> instance )
	{
		void (DataConversionTest::*fn)() = 0;

		// "To" types have greater range/precision, so that we can accurately verify roundtrip. If we didn't do this
		/// we'd lose information on the way through.

		fn = &DataConversionTest::testSignedScaled<char, short>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<char, int>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<char, long>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<char, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<char, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );

		fn = &DataConversionTest::testSignedScaled<short, int>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<short, long>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<short, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<short, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );

		fn = &DataConversionTest::testSignedScaled<int, long>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<int, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<int, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );

		fn = &DataConversionTest::testSignedScaled<long, float>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
		fn = &DataConversionTest::testSignedScaled<long, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );

		fn = &DataConversionTest::testSignedScaled<float, double>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
	}

};

}

#endif // IE_CORE_DATACONVERSIONTEST_H

