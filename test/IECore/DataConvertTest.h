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

#ifndef IE_CORE_DATACONVERTTEST_H
#define IE_CORE_DATACONVERTTEST_H

#include "IECore/DataConversion.h"
#include "IECore/DataConvert.h"
#include "IECore/Export.h"
#include "IECore/IECore.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/test/floating_point_comparison.hpp"

#include <cassert>

namespace IECore
{

void addDataConvertTest( boost::unit_test::test_suite* test );

struct DataConvertTest
{
	template<typename F, typename T>
	class DoubleItDataConversion : public DataConversion< F, T >
	{
		public :

			T operator()( F f ) const
			{
				return f * 2;
			}
	};

	template<typename F, typename T>
	void testVectorData()
	{
		typename F::Ptr from = new F();

		from->writable().resize( 1024 );

		for ( unsigned i = 0; i < 1024; i ++ )
		{
			from->writable()[i] = i;
		}

		typedef DoubleItDataConversion< typename F::ValueType::value_type, typename T::ValueType::value_type > Conv;

		typename T::Ptr to = DataConvert< F, T, Conv >()( from );
		BOOST_CHECK( to );
		BOOST_CHECK_EQUAL( (int)to->readable().size(), 1024 );
		BOOST_CHECK_EQUAL( (float)to->readable()[512], 1024.f );
	}

	template<typename F, typename T>
	void testSimpleData()
	{
		typename F::Ptr from = new F();

		from->writable() = 512;

		typedef DoubleItDataConversion< typename F::ValueType, typename T::ValueType > Conv;

		typename T::Ptr to = DataConvert< F, T, Conv >()( from );
		BOOST_CHECK( to );
		BOOST_CHECK_EQUAL( (float)to->readable(), 1024.f );
	}
};

struct DataConvertTestSuite : public boost::unit_test::test_suite
{

	DataConvertTestSuite() : boost::unit_test::test_suite( "DataConvertTestSuite" )
	{
		static boost::shared_ptr<DataConvertTest> instance( new DataConvertTest() );

		testVectorData( instance );
		testSimpleData( instance );
	}

	void testVectorData( boost::shared_ptr<DataConvertTest> instance )
	{
		void (DataConvertTest::*fn)() = 0;

		fn = &DataConvertTest::testVectorData< UIntVectorData, FloatVectorData >;
		auto test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "UIntVectorData-FloatVectorData" );
		add( test );

		fn = &DataConvertTest::testVectorData< ShortVectorData, DoubleVectorData >;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "ShortVectorData-DoubleVectorData" );
		add( test );
	}

	void testSimpleData( boost::shared_ptr<DataConvertTest> instance )
	{
		void (DataConvertTest::*fn)() = 0;

		fn = &DataConvertTest::testSimpleData< UIntData, FloatData >;
		auto test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "UIntData-FloatData" );
		add( test );

		fn = &DataConvertTest::testSimpleData< ShortData, DoubleData >;
		test = BOOST_CLASS_TEST_CASE( fn, instance );
		test->p_name.set( test->p_name.get() + "ShortData-DoubleData" );
		add( test );
	}

};

}

#endif // IE_CORE_DATACONVERTTEST_H

