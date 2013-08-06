//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SPHERICALHARMONICSTEST_H
#define IE_CORE_SPHERICALHARMONICSTEST_H

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "IECore/SphericalHarmonics.h"
#include "IECore/SphericalHarmonicsProjector.h"
#include "IECore/SphericalHarmonicsRotationMatrix.h"
#include "IECore/SphericalHarmonicsAlgo.h"
#include "IECore/SphericalHarmonicsTransferMatrix.h"

namespace IECore
{
void addSphericalHarmonicsTest(boost::unit_test::test_suite* test);

template<typename T >
class SphericalHarmonicsTest
{
	public:

		void testFunctionEvaluation();
		void testConstruction();
		void testDotProduct();
		void testArithmeticOperations();
		void testSHProduct();
};

template<typename T, int bands, unsigned int samples >
class SphericalHarmonicsProjectorTest
{
	public:

		// uses a light functor we know the answer.
		void testProjection();

		// tests 1D functor with polar coordinates
		void testPolarProjection1D();

		// tests 3D functor with polar coordinates
		void testPolarProjection3D();

		// tests 1D functor with euclidean coordinates
		void testEuclideanProjection1D();

		// tests 3D functor with euclidean coordinates
		void testEuclideanProjection3D();


		static T euclidean1DFunctor( const Imath::Vec3<T> &pos );
		static Imath::Vec3<T> euclidean3DFunctor( const Imath::Vec3<T> &pos );

	private:

		static T lightFunctor( const Imath::Vec2<T> &polar );
		static T polar1DFunctor( const Imath::Vec2<T> &polar );
		static Imath::Vec3<T> polar3DFunctor( const Imath::Vec2<T> &polar );
};

template< typename T >
class SphericalHarmonicsRotationMatrixTest
{
	public:

		void testRotation();
		void testRotation3D();

	private:

		static Imath::Euler<T> rotation();
		static T normalFunctor( const Imath::Vec3<T> &pos );
		static T rotatedFunctor( const Imath::Vec3<T> &pos );

		static Imath::Vec3<T> normal3dFunctor( const Imath::Vec3<T> &pos );
		static Imath::Vec3<T> rotated3dFunctor( const Imath::Vec3<T> &pos );

};

template< typename T >
class SphericalHarmonicsTransferMatrixTest
{
	public:

		template< typename S > void testTransfer();

};

struct SphericalHarmonicsTestSuite : public boost::unit_test::test_suite
{

	SphericalHarmonicsTestSuite() : boost::unit_test::test_suite("SphericalHarmonicsTestSuite")
	{
		addSphericalHarmonicsTest< float >();
		addSphericalHarmonicsTest< double >();
		addSphericalHarmonicsProjectorTest< double,10,20000 >();
		addSphericalHarmonicsRotationMatrixTest< double >();

	}

	template< typename T >
	void addSphericalHarmonicsTest()
	{
		static boost::shared_ptr< SphericalHarmonicsTest< T > > instance(new SphericalHarmonicsTest<T>());

		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsTest<T>::testFunctionEvaluation, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsTest<T>::testConstruction, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsTest<T>::testDotProduct, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsTest<T>::testArithmeticOperations, instance ) );
	}

	template< typename T, int bands, int samples >
	void addSphericalHarmonicsProjectorTest()
	{
		typedef SphericalHarmonicsProjectorTest<T,bands,samples> TestType;
		static boost::shared_ptr<TestType> instance(new TestType());

		add( BOOST_CLASS_TEST_CASE( &TestType::testProjection, instance ) );
		add( BOOST_CLASS_TEST_CASE( &TestType::testPolarProjection1D, instance ) );
		add( BOOST_CLASS_TEST_CASE( &TestType::testPolarProjection3D, instance ) );
		add( BOOST_CLASS_TEST_CASE( &TestType::testEuclideanProjection1D, instance ) );
		add( BOOST_CLASS_TEST_CASE( &TestType::testEuclideanProjection3D, instance ) );
	}

	template< typename T >
	void addSphericalHarmonicsRotationMatrixTest()
	{
		static boost::shared_ptr< SphericalHarmonicsRotationMatrixTest< T > > instance(new SphericalHarmonicsRotationMatrixTest<T>());

		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsRotationMatrixTest<T>::testRotation, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SphericalHarmonicsRotationMatrixTest<T>::testRotation3D, instance ) );
	}

	template< typename T >
	void addSphericalHarmonicsTransferMatrixTest()
	{
		static boost::shared_ptr< SphericalHarmonicsTransferMatrixTest< T > > instance(new SphericalHarmonicsTransferMatrixTest<T>());

		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsTransferMatrixTest< T >::template testTransfer< T >), instance ) );
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsTransferMatrixTest< T >::template testTransfer< Imath::Vec3< T > > ), instance ) );
	}

};
}

#endif

