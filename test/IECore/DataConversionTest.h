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

#ifndef IE_CORE_DATACONVERSIONTEST_H
#define IE_CORE_DATACONVERSIONTEST_H

#include <cassert>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "IECore/HalfTypeTraits.h"
#include "IECore/IECore.h"
#include "IECore/CineonToLinearDataConversion.h"
#include "IECore/LinearToCineonDataConversion.h"
#include "IECore/SRGBToLinearDataConversion.h"
#include "IECore/LinearToSRGBDataConversion.h"
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
		CompoundDataConversion< Func, typename Func::InverseType > f;
	
		/// Verify that f(f'(i)) == i
		for ( F i = 0; i < 1024; i++ )
		{		
			BOOST_CHECK_EQUAL( f(i), i );
		}
	}
	
	template<typename T>
	void testSRGBLinear()
	{
		typedef SRGBToLinearDataConversion< T, T > Func;	
		CompoundDataConversion< Func, typename Func::InverseType > f;
	
		/// Verify that f(f'(i)) ~ i
		for ( T i = T(0); i < T(10); i += T(0.2) )
		{		
			BOOST_CHECK_CLOSE( double( f(i) ), double( i ), 1.e-4 );
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
		
};

}

#endif // IE_CORE_DATACONVERSIONTEST_H

