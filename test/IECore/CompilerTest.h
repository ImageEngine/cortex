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

#ifndef IE_CORE_COMPILERTEST_H
#define IE_CORE_COMPILERTEST_H

#include "boost/test/unit_test.hpp"

#include "OpenEXR/ImathLineAlgo.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "IECore/BoxOps.h"

namespace IECore
{

void addCompilerTest(boost::unit_test::test_suite* test);

struct CompilerTest
{

	void runTest1()
	{
		Imath::Box3f b1( Imath::V3f( -1 ), Imath::V3f( 1 ) );
		Imath::Box3f b2( Imath::V3f( 0, -0.5, 0.5 ), Imath::V3f( 0.1, 0, 0.9 ) );

		/// This seems to be failing on gcc4.2.3 with optimisations -O2, and -O3. -O1 seems fine.
		BOOST_CHECK( boxContains( b1, b2 ) );
	}

	void runTest2()
	{
		Imath::V3f p0 = Imath::V3f( 0.587785,         0, 0.809017 );
		Imath::V3f p1 = Imath::V3f( 0.799057, -0.156434, 0.580549 );
		Imath::V3f p2 = Imath::V3f( 0.580549, -0.156434, 0.799057 );

		Imath::Line3f ln;
		ln.pos = Imath::V3f( -0.289445, -0.0803292, 0.295812 );
		ln.dir = Imath::V3f(  0.898071, -0.0705415, 0.434157 );


		Imath::V3f hitPoint, bary;
		bool front;

		/// This seems to be failing on gcc4.2.3 with optimisations -O2, and -O3. -O1 seems fine.
		BOOST_CHECK( intersect( ln, p0, p1, p2, hitPoint, bary, front ) );
	}
};


struct CompilerTestSuite : public boost::unit_test::test_suite
{

	CompilerTestSuite() : boost::unit_test::test_suite("CompilerTestSuite")
	{
		boost::shared_ptr<CompilerTest> instance(new CompilerTest());

		add( BOOST_CLASS_TEST_CASE( &CompilerTest::runTest1, instance ) );
		add( BOOST_CLASS_TEST_CASE( &CompilerTest::runTest2, instance ) );
	}
};

}

#endif

