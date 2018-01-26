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

#include "StringAlgoTest.h"

#include "IECore/StringAlgo.h"

#include <iostream>
#include <set>

using namespace boost;
using namespace boost::unit_test;

namespace IECore
{

struct StringAlgoTest
{

	void noElements()
	{
		std::vector<std::string> v;
		std::string sep = " ";

		std::string result = StringAlgo::join( v.begin(), v.end(), sep );

		BOOST_CHECK( result == "" );
	}

	void singleElement()
	{
		std::vector<std::string> v;
		v.push_back( "tif" );
		std::string sep = " ";

		std::string result = StringAlgo::join( v.begin(), v.end(), sep );

		BOOST_CHECK( result == "tif" );
	}

	void severalElements()
	{
		std::vector<std::string> v;
		v.push_back( "tif" );
		v.push_back( "exr" );
		v.push_back( "foobar" );
		v.push_back( "string with spaces" );
		std::string sep = " ";

		std::string result = StringAlgo::join( v.begin(), v.end(), sep );

		BOOST_CHECK( result == "tif exr foobar string with spaces"  );
	}

	void partial()
	{
		std::vector<std::string> v;
		v.push_back( "tif" );
		v.push_back( "exr" );
		v.push_back( "foobar" );
		v.push_back( "string with spaces" );
		std::string sep = " ";

		std::vector<std::string>::const_iterator start = v.begin();
		std::vector<std::string>::const_iterator end = v.end();
		start++;
		end--;

		std::string result = StringAlgo::join( start, end, sep );

		BOOST_CHECK( result == "exr foobar" );
	}

	void separator()
	{
		std::vector<std::string> v;
		v.push_back( "tif" );
		v.push_back( "exr" );
		v.push_back( "foobar" );
		v.push_back( "string with spaces" );
		std::string sep = ".";

		std::string result = StringAlgo::join( v.begin(), v.end(), sep );

		BOOST_CHECK( result == "tif.exr.foobar.string with spaces" );
	}

	void containers()
	{
		std::list<std::string> l;
		l.push_back( "tif" );
		l.push_back( "exr" );
		l.push_back( "foobar" );
		l.push_back( "string with spaces" );
		std::string sep = ".";

		std::set<std::string> s;
		s.insert( "tif" );
		s.insert( "exr" );
		s.insert( "foobar" );
		s.insert( "tif" );
		s.insert( "string with spaces" );

		std::string lResult = StringAlgo::join( l.begin(), l.end(), sep );
		std::string sResult = StringAlgo::join( s.begin(), s.end(), sep );

		BOOST_CHECK( lResult == "tif.exr.foobar.string with spaces" );
		BOOST_CHECK( sResult == "exr.foobar.string with spaces.tif" );
	}

	void notJustForStrings()
	{
		std::vector<float> v( 4, 2.5f );
		float sep = 0.0f;

		float result = StringAlgo::join( v.begin(), v.end(), sep );

		BOOST_CHECK( result == 10.0f );
	}
};


struct StringAlgoTestSuite : public boost::unit_test::test_suite
{

	StringAlgoTestSuite() : boost::unit_test::test_suite( "StringAlgoTestSuite" )
	{
		boost::shared_ptr<StringAlgoTest> instance( new StringAlgoTest() );

		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::noElements, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::singleElement, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::severalElements, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::partial, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::separator, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::containers, instance ) );
		add( BOOST_CLASS_TEST_CASE( &StringAlgoTest::notJustForStrings, instance ) );
	}
};

void addStringAlgoTest(boost::unit_test::test_suite* test)
{
	test->add( new StringAlgoTestSuite( ) );
}

} // namespace IECore
