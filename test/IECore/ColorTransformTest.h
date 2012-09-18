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

#ifndef IE_CORE_COLORTRANSFORMTEST_H
#define IE_CORE_COLORTRANSFORMTEST_H

#include <cassert>
#include <limits.h>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/random.hpp"

#include "IECore/RGBToXYZColorTransform.h"
#include "IECore/XYZToRGBColorTransform.h"

#include "IECore/XYZToXYYColorTransform.h"

using namespace Imath;

namespace IECore
{

void addColorTransformTest(boost::unit_test::test_suite* test);

struct ColorTransformTest
{
	template<typename F, typename T>
	void testRGBXYZ()
	{
		/// \todo More rigorous testing
		typedef RGBToXYZColorTransform< F, T > Func;
		typedef typename RGBToXYZColorTransform< F, T >::InverseType InvFunc;

		Func f;

		Imath::M33f expected(
			0.4124, 0.2126, 0.0193,
			0.3575, 0.715, 0.1191,
			0.1804, 0.072, 0.950 );

		BOOST_CHECK( f.matrix().equalWithAbsError( expected, 0.01 ) );

		InvFunc fi;

		Imath::Color3f input( 0.5, 0.5, 0.5 );
		Imath::Color3f output = f( input );
		BOOST_CHECK( !output.equalWithAbsError( input, 0.01 ) );
		output = fi( output );
		BOOST_CHECK( output.equalWithAbsError( input, 0.01 ) );
	}

	template<typename F, typename T>
	void testXYYXYZ()
	{
		/// \todo More rigorous testing
		typedef XYYToXYZColorTransform< F, T > Func;
		typedef typename XYYToXYZColorTransform< F, T >::InverseType InvFunc;

		Func f;
		InvFunc fi;

		Imath::Color3f input( 0.5, 0.5, 0.5 );
		Imath::Color3f output = f( input );
		BOOST_CHECK( !output.equalWithAbsError( input, 0.01 ) );
		output = fi( output );
		BOOST_CHECK( output.equalWithAbsError( input, 0.01 ) );
	}
};

struct ColorTransformTestSuite : public boost::unit_test::test_suite
{

	ColorTransformTestSuite() : boost::unit_test::test_suite( "ColorTransformTestSuite" )
	{
		static boost::shared_ptr<ColorTransformTest> instance( new ColorTransformTest() );

		testRGBXYZ( instance );
		testXYYXYZ( instance );
	}

	void testRGBXYZ( boost::shared_ptr<ColorTransformTest> instance )
	{
		void (ColorTransformTest::*fn)() = 0;

		fn = &ColorTransformTest::testRGBXYZ<Imath::Color3f, Imath::Color3f>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );
	}

	void testXYYXYZ( boost::shared_ptr<ColorTransformTest> instance )
	{
		void (ColorTransformTest::*fn)() = 0;

		fn = &ColorTransformTest::testXYYXYZ<Imath::Color3f, Imath::Color3f>;
		add( BOOST_CLASS_TEST_CASE( fn, instance ) );

	}
};

}

#endif // IE_CORE_COLORTRANSFORMTEST_H

