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

#ifndef IE_CORE_DESPATCHTYPEDDATATEST_H
#define IE_CORE_DESPATCHTYPEDDATATEST_H

#include <cassert>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"

using namespace Imath;

namespace IECore
{

void addDespatchTypedDataTest( boost::unit_test::test_suite* test );

struct TestFunctor
{
	typedef void ReturnType;
	int m_successes;

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		BOOST_CHECK( data );

		m_successes ++;
		BOOST_CHECK( data->typeId() == M33fDataTypeId || data->typeId() == StringDataTypeId );
	}
};

struct TestFunctorErrorHandler
{
	int m_failures;

	template<typename T, typename F >
	void operator()( typename T::ConstPtr data, F& functor )
	{
		BOOST_CHECK( data );

		m_failures++;
	}
};

struct DespatchTypedDataTest
{
	void test()
	{
		TestFunctor func;
		func.m_successes = 0;

		TestFunctorErrorHandler eh;
		eh.m_failures = 0;

		despatchTypedData< TestFunctor, TypeTraits::IsSimpleTypedData, TestFunctorErrorHandler >( new V3fData(), func, eh );
		despatchTypedData< TestFunctor, TypeTraits::IsSimpleTypedData, TestFunctorErrorHandler >( new M33fData(), func, eh );
		despatchTypedData< TestFunctor, TypeTraits::IsSimpleTypedData, TestFunctorErrorHandler >( new StringData(), func, eh );
		despatchTypedData< TestFunctor, TypeTraits::IsSimpleTypedData, TestFunctorErrorHandler >( new V3fVectorData(), func, eh ); // should fail

		BOOST_CHECK_EQUAL( func.m_successes, 3 );
		BOOST_CHECK_EQUAL( eh.m_failures, 1 );
	}
};

struct DespatchTypedDataTestSuite : public boost::unit_test::test_suite
{
	DespatchTypedDataTestSuite() : boost::unit_test::test_suite( "DespatchTypedDataTestSuite" )
	{
		static boost::shared_ptr<DespatchTypedDataTest> instance( new DespatchTypedDataTest() );

		add( BOOST_CLASS_TEST_CASE( &DespatchTypedDataTest::test, instance ) );
	}
};

}

#endif // IE_CORE_DESPATCHTYPEDDATATEST_H

