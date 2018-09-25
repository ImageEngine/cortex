//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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


#include <IECoreScene/SceneInterface.h>
#include "PathMatcherTest.h"

#include "IECoreScene/SceneInterface.h"

#include "IECore/PathMatcher.h"

namespace IECore
{

struct PathMatcherTest
{
	void testPathMatcherIterator()
	{
		IECore::PathMatcher pathMatcher;

		pathMatcher.addPath("/a/b/c");
		pathMatcher.addPath("/a/b/c/d/e/f");
		pathMatcher.addPath("/a/b/c/d/g/h");

		BOOST_CHECK( pathMatcher.find({ IECore::InternedString( "a" ) } ) != pathMatcher.end() );
		BOOST_CHECK( pathMatcher.find({ IECore::InternedString( "a" ), IECore::InternedString( "b" ) } ) != pathMatcher.end() );
		BOOST_CHECK( pathMatcher.find({ IECore::InternedString( "a" ), IECore::InternedString( "b" ), IECore::InternedString( "c" ) } ) != pathMatcher.end() );

		IECore::PathMatcher::RawIterator x = pathMatcher.find(  { IECore::InternedString( "a" ),  IECore::InternedString( "b" ),  IECore::InternedString( "c" ),  IECore::InternedString( "d" ) }  );

		// Finds the next terminal node
		IECore::PathMatcher::Iterator y = IECore::PathMatcher::Iterator( x );

		// This check to see if the path we're querying has been explicitly added.
		BOOST_CHECK( y != x);
	}
};

struct PathMatcherTestSuite : public boost::unit_test::test_suite
{

	PathMatcherTestSuite() : boost::unit_test::test_suite( "PathMatcherTestSuite" )
	{
		boost::shared_ptr<PathMatcherTest> instance( new PathMatcherTest() );
		add( BOOST_CLASS_TEST_CASE( &PathMatcherTest::testPathMatcherIterator, instance ) );
	}
};

void addPathMatcherTest(boost::unit_test::test_suite* test)
{
	test->add( new PathMatcherTestSuite() );
}

} // IECore