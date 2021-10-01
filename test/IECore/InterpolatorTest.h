//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Export.h"
#include "IECore/Interpolator.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "OpenEXR/ImathMatrixAlgo.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/test/floating_point_comparison.hpp"

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



template<typename T>
class MatrixLinearInterpolatorTest
{
	public:
		void testSimple();
		void testTyped();
		void testVector();
};

template<typename T>
class MatrixCubicInterpolatorTest
{
	public:

		void testSimple();
		void testTyped();
		void testVector();
};




struct InterpolatorTestSuite : public boost::unit_test::test_suite
{

	InterpolatorTestSuite()
		: boost::unit_test::test_suite("InterpolatorTestSuite")
	{
		addLinearTest<float>( "float" );
		addLinearTest<double>( "double" );
		addLinearTest<Imath::V3f>( "V3f" );
		addLinearTest<Imath::V3d>( "V3d" );

		addLinearMatrixTest<float>( "float" );
		addLinearMatrixTest<double>( "double" );

		addCubicTest<float>( "float" );
		addCubicTest<double>( "double" );
		addCubicTest<Imath::V3f>( "V3f" );
		addCubicTest<Imath::V3d>( "V3d" );

		addCubicMatrixTest<float>( "float" );
		addCubicMatrixTest<double>( "double" );
	}

	template<typename T>
	void addLinearTest( const std::string &nameSuffix )
	{
		static boost::shared_ptr<LinearInterpolatorTest<T> > instance(new LinearInterpolatorTest<T>());

		auto test = BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testSimple, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testTyped, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &LinearInterpolatorTest<T>::testVector, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );
	}

	template<typename T>
	void addCubicTest( const std::string &nameSuffix )
	{
		static boost::shared_ptr<CubicInterpolatorTest<T> > instance(new CubicInterpolatorTest<T>());

		auto test = BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testSimple, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testTyped, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &CubicInterpolatorTest<T>::testVector, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );
	}

	template<typename T>
	void addLinearMatrixTest( const std::string &nameSuffix )
	{
		static boost::shared_ptr<MatrixLinearInterpolatorTest<T> > instance(new MatrixLinearInterpolatorTest<T>());

		auto test = BOOST_CLASS_TEST_CASE( &MatrixLinearInterpolatorTest<T>::testSimple, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &MatrixLinearInterpolatorTest<T>::testTyped, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &MatrixLinearInterpolatorTest<T>::testVector, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );
	}

	template<typename T>
	void addCubicMatrixTest( const std::string &nameSuffix )
	{
		static boost::shared_ptr<MatrixCubicInterpolatorTest<T> > instance(new MatrixCubicInterpolatorTest<T>());

		auto test = BOOST_CLASS_TEST_CASE( &MatrixCubicInterpolatorTest<T>::testSimple, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &MatrixCubicInterpolatorTest<T>::testTyped, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &MatrixCubicInterpolatorTest<T>::testVector, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );
	}
};
}

#include "InterpolatorTest.inl"

#endif

