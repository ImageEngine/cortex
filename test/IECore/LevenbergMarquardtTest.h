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

#ifndef IE_CORE_LEVENBERGMARQUARDTTEST_H
#define IE_CORE_LEVENBERGMARQUARDTTEST_H

#include "IECore/Export.h"
#include "IECore/LevenbergMarquardt.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "OpenEXR/ImathRandom.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include <algorithm>
#include <iostream>
#include <boost/mpl/list.hpp>

namespace IECore
{

void addLevenbergMarquardtTest( boost::unit_test::test_suite* test );

template<typename T>
class LevenbergMarquardtTestSimple
{
		class Fn;

	public:
		LevenbergMarquardtTestSimple();
		virtual ~LevenbergMarquardtTestSimple();

		void test();

};

template<typename T>
class LevenbergMarquardtTestPolynomialFit
{
		template<int N>
		class Fn;

	public:
		LevenbergMarquardtTestPolynomialFit();
		virtual ~LevenbergMarquardtTestPolynomialFit();

		template<int N>
		void test();
};

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_Simple, T)
{
	static boost::shared_ptr< LevenbergMarquardtTestSimple<T> > instance( new LevenbergMarquardtTestSimple<T>() );
	instance->test();
}

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_CurveFit, T)
{
	static boost::shared_ptr< LevenbergMarquardtTestPolynomialFit<T> > instance( new LevenbergMarquardtTestPolynomialFit<T>() );
	instance->template test<1>();
	instance->template test<2>();
	instance->template test<3>();
	instance->template test<4>();
}

struct LevenbergMarquardtTestSuite : public boost::unit_test::test_suite
{

	LevenbergMarquardtTestSuite() : boost::unit_test::test_suite( "LevenbergMarquardtTestSuite" )
	{
		typedef boost::mpl::list<float,double> type_list;
		add( BOOST_TEST_CASE_TEMPLATE(test_Simple, type_list) );
		add( BOOST_TEST_CASE_TEMPLATE(test_CurveFit, type_list) );
	}

};

}

#include "LevenbergMarquardtTest.inl"

#endif // IE_CORE_LEVENBERGMARQUARDTTEST_H
