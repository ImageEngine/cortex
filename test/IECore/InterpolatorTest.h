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

#ifndef IE_CORE_INTERPOLATORTEST_H
#define IE_CORE_INTERPOLATORTEST_H

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <OpenEXR/ImathVec.h>

#include <IECore/Interpolator.h>

namespace IECore
{
void addInterpolatorTest(boost::unit_test::test_suite* test);

template<typename T>
class LinearInterpolatorTest
{
	public:
		void testSimple();
		void testTyped();
		void testVector();
};

template<typename T>
class CubicInterpolatorTest
{
	public:

		void testSimple();
		void testTyped();
		void testVector();
};

struct InterpolatorTestSuite : public boost::unit_test::test_suite
{

	InterpolatorTestSuite() : boost::unit_test::test_suite("InterpolatorTestSuite")
	{
		addLinearTest<float>();
		addLinearTest<double>();
		addLinearTest<Imath::V3f>();
		addLinearTest<Imath::V3d>();

		addCubicTest<float>();
		addCubicTest<double>();
		addCubicTest<Imath::V3f>();
		addCubicTest<Imath::V3d>();
	}

	template<typename T>
	void addLinearTest()
	{
		static boost::shared_ptr<LinearInterpolatorTest<T> > instance(new LinearInterpolatorTest<T>());

		add( BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testSimple, instance ) );
		add( BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testTyped, instance ) );
		add( BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testVector, instance ) );
	}

	template<typename T>
	void addCubicTest()
	{
		static boost::shared_ptr<CubicInterpolatorTest<T> > instance(new CubicInterpolatorTest<T>());

		add( BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testSimple, instance ) );
		add( BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testTyped, instance ) );
		add( BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testVector, instance ) );
	}
};
}

#include "InterpolatorTest.inl"

#endif

