//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TYPEDDATATEST_H
#define IE_CORE_TYPEDDATATEST_H

#include "IECore/Export.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "OpenEXR/ImathRandom.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include <iostream>
#include <vector>
#include <boost/mpl/list.hpp>

namespace IECore
{

void addTypedDataTest(boost::unit_test::test_suite* test);

template<typename T>
class VectorTypedDataTest
{
	public:
		VectorTypedDataTest(unsigned int size);
		virtual ~VectorTypedDataTest();

		/// Some simple repeatable functions which are used to fill the vector with some initial data
		typename T::value_type f1(unsigned int i) const;
		typename T::value_type f2(unsigned int i) const;

		void testSize();
		void testRead();
		void testWrite();
		void testAssign();

		unsigned int randomElementPos();

		unsigned int m_size;
		typename TypedData<T>::Ptr m_data;

		Imath::Rand32 m_randGen;

};

template<typename T>
class SimpleTypedDataTest
{
	public:
		SimpleTypedDataTest();
		virtual ~SimpleTypedDataTest();

		void testRead();
		void testWrite();
		void testAssign();
		void testMemoryUsage();
		void testIsEqualTo();

		typename TypedData<T>::Ptr m_data;

};

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_VectorTypedData, T)
{
	auto values = std::vector<int>{0, 1, 10, 100, 1000000};
	for (auto v : values) {
		static boost::shared_ptr<VectorTypedDataTest<T> > instance(new VectorTypedDataTest<T>(v));
		instance->testSize();
		instance->testRead();
		instance->testWrite();
		instance->testAssign();
	}
}

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_SimpleTypedData, T)
{
	static boost::shared_ptr<SimpleTypedDataTest<T> > instance(new SimpleTypedDataTest<T>());
	instance->testRead();
	instance->testWrite();
	instance->testAssign();
	instance->testIsEqualTo();
}

struct TypedDataTestSuite : public boost::unit_test::test_suite
{

	TypedDataTestSuite() : boost::unit_test::test_suite("TypedDataTestSuite")
	{
		typedef boost::mpl::list<std::vector<float>,std::vector<int>,std::vector<double>> type_list;
		add( BOOST_TEST_CASE_TEMPLATE(test_VectorTypedData, type_list) );

		typedef boost::mpl::list<float,double,int,unsigned int, char, unsigned char> type_list2;
		add( BOOST_TEST_CASE_TEMPLATE(test_SimpleTypedData, type_list2));
	}
};

}

#include "TypedDataTest.inl"

#endif // IE_CORE_TYPEDDATATEST_H
