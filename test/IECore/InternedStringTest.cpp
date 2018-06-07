//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "InternedStringTest.h"

#include "IECore/InternedString.h"

#include "OpenEXR/ImathRandom.h"

#include "boost/lexical_cast.hpp"

#include "tbb/tbb.h"

#include <iostream>

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct InternedStringTest
{

	struct Constructor
	{
		public :

			Constructor()
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				Imath::Rand32 rand;
				for( size_t i=r.begin(); i!=r.end(); ++i )
				{
					std::string s = lexical_cast<std::string>( rand.nexti() % 1000 );
					InternedString ss( s );
				}
			}

	};

	void testConcurrentConstruction()
	{
		size_t numIterations = 10000000;
		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, numIterations ), Constructor(), taskGroupContext );
	}

	void testRangeConstruction()
	{

		const char *aa = "aa";
		const char *aabb = "aabb";
		const char *aabbaa = "aabbaa";

		BOOST_CHECK_EQUAL( InternedString( aa ), InternedString( aa, 2 ) );
		BOOST_CHECK_EQUAL( InternedString( aa, 2 ), InternedString( aa, 2 ) );
		BOOST_CHECK_EQUAL( InternedString( aa, 1 ), InternedString( aa, 1 ) );
		BOOST_CHECK_EQUAL( InternedString( aa, 2 ), InternedString( aabb, 2 ) );
		BOOST_CHECK_EQUAL( InternedString( aabb ), InternedString( aabbaa, 4 ) );

	};

};


struct InternedStringTestSuite : public boost::unit_test::test_suite
{

	InternedStringTestSuite() : boost::unit_test::test_suite( "InternedStringTestSuite" )
	{
		boost::shared_ptr<InternedStringTest> instance( new InternedStringTest() );

		add( BOOST_CLASS_TEST_CASE( &InternedStringTest::testConcurrentConstruction, instance ) );
		add( BOOST_CLASS_TEST_CASE( &InternedStringTest::testRangeConstruction, instance ) );

	}
};

void addInternedStringTest(boost::unit_test::test_suite* test)
{
	test->add( new InternedStringTestSuite( ) );
}

} // namespace IECore
