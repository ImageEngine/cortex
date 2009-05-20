//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SPACETRANSFORMTEST_H
#define IE_CORE_SPACETRANSFORMTEST_H

#include <cassert>
#include <limits.h>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include <OpenEXR/ImathRandom.h>

#include "OpenEXR/ImathVec.h"
#include "IECore/EuclidianToSphericalTransform.h"
#include "IECore/SphericalToEuclidianTransform.h"

using namespace Imath;

namespace IECore
{

void addSpaceTransformTest(boost::unit_test::test_suite* test);

struct EuclidianSphericalTest
{
	void test()
	{
		Imath::Rand32 r( 88 );
		SphericalToEuclidianTransform< Imath::V3f, Imath::V3f > sph2euc;
		EuclidianToSphericalTransform< Imath::V3f, Imath::V3f > euc2sph;

		BOOST_CHECK( Imath::V3f( 0, 0, 2 ).equalWithAbsError( sph2euc( Imath::V3f( 0, 0, 2 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( 2, 0, 0 ).equalWithAbsError( sph2euc( Imath::V3f( 0, M_PI * 0.5, 2 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( 0, 2, 0 ).equalWithAbsError( sph2euc( Imath::V3f( M_PI * 0.5, M_PI * 0.5, 2 ) ), 0.01 ) );

		BOOST_CHECK( Imath::V3f( 0, 0, 2 ).equalWithAbsError( euc2sph( Imath::V3f( 0, 0, 2 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( 0, M_PI * 0.5, 2 ).equalWithAbsError( euc2sph( Imath::V3f( 2, 0, 0 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( M_PI * 0.5, M_PI * 0.5, 2 ).equalWithAbsError( euc2sph( Imath::V3f( 0, 2, 0 ) ), 0.01 ) );

		for ( int i = 0; i < 500; i++ )
		{
			Imath::V3f pos( 10 * r.nextf(), 10*r.nextf(), 10*r.nextf() );
			Imath::V3f sph( euc2sph( pos ) );
			BOOST_CHECK( pos.equalWithAbsError( sph2euc( sph ), 0.01 ) );
		}
	}

	void testNormalized()
	{
		Imath::Rand32 r( 88 );
		SphericalToEuclidianTransform< Imath::V2f, Imath::V3f > sph2euc;
		EuclidianToSphericalTransform< Imath::V3f, Imath::V2f > euc2sph;

		BOOST_CHECK( Imath::V3f( 0, 0, 1 ).equalWithAbsError( sph2euc( Imath::V2f( 0, 0 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( 1, 0, 0 ).equalWithAbsError( sph2euc( Imath::V2f( 0, M_PI * 0.5 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V3f( 0, 1, 0 ).equalWithAbsError( sph2euc( Imath::V2f( M_PI * 0.5, M_PI * 0.5 ) ), 0.01 ) );

		BOOST_CHECK( Imath::V2f( 0, 0 ).equalWithAbsError( euc2sph( Imath::V3f( 0, 0, 1 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V2f( 0, M_PI * 0.5 ).equalWithAbsError( euc2sph( Imath::V3f( 1, 0, 0 ) ), 0.01 ) );
		BOOST_CHECK( Imath::V2f( M_PI * 0.5, M_PI * 0.5 ).equalWithAbsError( euc2sph( Imath::V3f( 0, 1, 0 ) ), 0.01 ) );

		Imath::V3f pos;
		Imath::V2f sph;
		for ( int i = 0; i < 500; i++ )
		{
			pos = Imath::V3f( r.nextf(), r.nextf(), r.nextf() );
			pos.normalize();
			sph = euc2sph( pos );
			BOOST_CHECK( pos.equalWithAbsError( sph2euc( sph ), 0.01 ) );
		}
	}
};

struct SpaceTransformTestSuite : public boost::unit_test::test_suite
{

	SpaceTransformTestSuite() : boost::unit_test::test_suite( "SpaceTransformTestSuite" )
	{
		static boost::shared_ptr<EuclidianSphericalTest> instance( new EuclidianSphericalTest() );

		testEuclidianSpherical( instance );
	}

	void testEuclidianSpherical( boost::shared_ptr<EuclidianSphericalTest> instance )
	{
		add( BOOST_CLASS_TEST_CASE( &EuclidianSphericalTest::test, instance ) );
		add( BOOST_CLASS_TEST_CASE( &EuclidianSphericalTest::testNormalized, instance ) );
	}

};

}

#endif // IE_CORE_SPACETRANSFORMTEST_H

