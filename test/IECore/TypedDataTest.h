//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <iostream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <OpenEXR/ImathRandom.h>

#include <IECore/TypedData.h>

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
		boost::intrusive_ptr<TypedData<T> > m_data;

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

		boost::intrusive_ptr< TypedData<T> > m_data;

};


template<unsigned int N>
struct TypedDataTestSuite : public boost::unit_test::test_suite
{

	TypedDataTestSuite() : boost::unit_test::test_suite("TypedDataTestSuite")
	{
		addVectorTypedDataTest<std::vector<float> >();
		addVectorTypedDataTest<std::vector<int> >();
		addVectorTypedDataTest<std::vector<double> >();

		addSimpleTypedDataTest<float>();
		addSimpleTypedDataTest<double>();
		addSimpleTypedDataTest<int>();
		addSimpleTypedDataTest<unsigned int>();
		addSimpleTypedDataTest<char>();
		addSimpleTypedDataTest<unsigned char>();
	}

	template<typename T>
	void addVectorTypedDataTest()
	{
		static boost::shared_ptr<VectorTypedDataTest<T> > instance(new VectorTypedDataTest<T>(N));

		add( BOOST_CLASS_TEST_CASE( &VectorTypedDataTest<T>::testSize, instance ) );
		add( BOOST_CLASS_TEST_CASE( &VectorTypedDataTest<T>::testRead, instance ) );
		add( BOOST_CLASS_TEST_CASE( &VectorTypedDataTest<T>::testWrite, instance ) );
		add( BOOST_CLASS_TEST_CASE( &VectorTypedDataTest<T>::testAssign, instance ) );
	}

	template<typename T>
	void addSimpleTypedDataTest()
	{
		static boost::shared_ptr<SimpleTypedDataTest<T> > instance(new SimpleTypedDataTest<T>());

		add( BOOST_CLASS_TEST_CASE( &SimpleTypedDataTest<T>::testRead, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SimpleTypedDataTest<T>::testWrite, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SimpleTypedDataTest<T>::testAssign, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SimpleTypedDataTest<T>::testIsEqualTo, instance ) );
	}
};

}

#include "TypedDataTest.inl"

#endif // IE_CORE_TYPEDDATATEST_H
