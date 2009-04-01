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

#ifndef IE_CORE_SPHERICALHARMONICSTEST_H
#define IE_CORE_SPHERICALHARMONICSTEST_H

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <IECore/SphericalHarmonics.h>
#include <IECore/SphericalHarmonicsSampler.h>

namespace IECore
{
void addSphericalHarmonicsTest(boost::unit_test::test_suite* test);

template<typename T >
class SphericalHarmonicsFunctionTest
{
	public:

		void testEvaluation();
};

template<typename T, int bands, unsigned int samples >
class SphericalHarmonicsSamplerTest
{
	public:

		// uses a light functor we know the answer.
		void testProjection();

		// tests 1D functor with polar coordinates
		void testPolarProjection1D();

		// tests 3D functor with polar coordinates
		void testPolarProjection3D();

		// tests 1D functor with euclidian coordinates
		void testEuclidianProjection1D();

		// tests 3D functor with euclidian coordinates
		void testEuclidianProjection3D();

	private:

		static T lightFunctor( const Imath::V2f &polar );
		static T polar1DFunctor( const Imath::V2f &polar );
		static T euclidian1DFunctor( const Imath::V3f &pos );
		static Imath::Vec3<T> polar3DFunctor( const Imath::V2f &polar );
		static Imath::Vec3<T> euclidian3DFunctor( const Imath::V3f &pos );
};

struct SphericalHarmonicsTestSuite : public boost::unit_test::test_suite
{	
	
	SphericalHarmonicsTestSuite() : boost::unit_test::test_suite("SphericalHarmonicsTestSuite")
	{
		addSphericalHarmonicsFunctionTest< float >();
		addSphericalHarmonicsFunctionTest< double >();
		addSphericalHarmonicsSamplerTest< double,10,20000 >();
	}

	template< typename T >
	void addSphericalHarmonicsFunctionTest()
	{
		static boost::shared_ptr< SphericalHarmonicsFunctionTest< T > > instance(new SphericalHarmonicsFunctionTest<T>());
		
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsFunctionTest< T >::testEvaluation), instance ) );
	}

	template< typename T, int bands, int samples >
	void addSphericalHarmonicsSamplerTest()
	{
		static boost::shared_ptr< SphericalHarmonicsSamplerTest< T,bands,samples > > instance(new SphericalHarmonicsSamplerTest<T,bands,samples>());
		
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsSamplerTest< T,bands,samples >::testProjection), instance ) );
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsSamplerTest< T,bands,samples >::testPolarProjection1D), instance ) );
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsSamplerTest< T,bands,samples >::testPolarProjection3D), instance ) );
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsSamplerTest< T,bands,samples >::testEuclidianProjection1D), instance ) );
		add( BOOST_CLASS_TEST_CASE( &(SphericalHarmonicsSamplerTest< T,bands,samples >::testEuclidianProjection3D), instance ) );
	}
	
};
}

#endif

