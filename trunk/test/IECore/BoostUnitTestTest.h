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

#ifndef IE_CORE_BOOSTUNITTESTTEST_H
#define IE_CORE_BOOSTUNITTESTTEST_H

#include "boost/test/unit_test.hpp"

namespace IECore
{

void addBoostUnitTestTest(boost::unit_test::test_suite* test);

/// A test case to trap situations where reference count to test's
/// shared_ptr decrements to zero before test is run when optimisations are
/// enabled on osx104.ppc platform. Appears to be fixed in boost's CVS trunk.
struct BoostUnitTestTest
{
		bool m_testRun;

		BoostUnitTestTest()
		{
			m_testRun = false;
		}

		~BoostUnitTestTest()
		{
			if (!m_testRun)
			{
				std::cerr << "Test cases should be compiled without optimisations on this platform" << std::endl;
				exit(EXIT_FAILURE);
			}
		}

		void runTest()
		{
			m_testRun = true;
		}
};


struct BoostUnitTestTestSuite : public boost::unit_test::test_suite
{

	BoostUnitTestTestSuite() : boost::unit_test::test_suite("BoostUnitTestTestSuite")
	{
		boost::shared_ptr<BoostUnitTestTest> instance(new BoostUnitTestTest());

		add( BOOST_CLASS_TEST_CASE( &BoostUnitTestTest::runTest, instance ) );
	}
};

}

#endif

