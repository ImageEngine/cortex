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

#include "IECore/CompoundDataConversion.h"
#include "IECore/Export.h"
#include "IECore/HalfTypeTraits.h"
#include "IECore/IECore.h"
#include "IECore/ScaledDataConversion.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/random.hpp"

#include <cassert>

#include <limits.h>

namespace IECore
{

void addDataConversionTest(boost::unit_test::test_suite* test);

struct DataConversionTest
{
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

		testSignedScaled( instance );
	}

	void testSignedScaled( boost::shared_ptr<DataConversionTest> instance )
	{
		void (DataConversionTest::*fn)() = 0;

		// "To" types have greater range/precision, so that we can accurately verify roundtrip. If we didn't do this
		/// we'd lose information on the way through.

		fn = &DataConversionTest::testSignedScaled<char, short>;
		auto test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "char-short" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<char, int>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "char-int" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<char, long>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "char-long" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<char, float>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "char-float" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<char, double>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "char-double" );
		add( test );

		fn = &DataConversionTest::testSignedScaled<short, int>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "short-int" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<short, long>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "short-long" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<short, float>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "short-float" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<short, double>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "short-double" );
		add( test );

		fn = &DataConversionTest::testSignedScaled<int, long>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "int-long" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<int, float>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "int-float" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<int, double>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "int-double" );
		add( test );

		fn = &DataConversionTest::testSignedScaled<long, float>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "long-float" );
		add( test );
		fn = &DataConversionTest::testSignedScaled<long, double>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "long-double" );
		add( test );

		fn = &DataConversionTest::testSignedScaled<float, double>;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "float-double" );
		add( test );
	}

};

}

#endif // IE_CORE_DATACONVERSIONTEST_H

