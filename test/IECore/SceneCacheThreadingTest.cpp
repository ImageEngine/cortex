//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

#include <vector>
#include <iostream>

#include "tbb/tbb.h"

#include "IECore/SharedSceneInterfaces.h"

#include "SceneCacheThreadingTest.h"

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct SceneCacheThreadingTest
{

	struct TestSceneCache
	{
		public :

			TestSceneCache( const char *attribute ) : m_errors( 0 ), m_attribute( attribute )
			{
			}

			TestSceneCache( TestSceneCache &that, tbb::split ) : m_errors( 0 ), m_attribute( that.m_attribute )
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				for ( size_t i = r.begin(); i != r.end(); ++i )
				{
					for ( size_t j = 0; j < 1000; j++ )
					{
						if ( ( i + j ) % 7 == 0 )
						{
							SharedSceneInterfaces::clear();
						}

						ConstSceneInterfacePtr scene = SharedSceneInterfaces::get("test/IECore/data/sccFiles/attributeAtRoot.scc");

						try
						{
							scene->readAttribute( m_attribute, 0 );
						}
						catch( Exception &e )
						{
							m_errors++;
						}
					}
				}
			}

			void join( const TestSceneCache &that )
			{
				m_errors += that.m_errors;
			}

			size_t errors() const
			{
				return m_errors;
			}

		private :
			mutable size_t m_errors;
			SceneInterface::Name m_attribute;
	};

	void testAttributeRead()
	{
		task_scheduler_init scheduler( 100 );

		TestSceneCache task( "w" );

		parallel_reduce( blocked_range<size_t>( 0, 100 ), task );
 		BOOST_CHECK( task.errors() == 0 );
	}

	void testFakeAttributeRead()
	{
		task_scheduler_init scheduler( 100 );

		TestSceneCache task( "fake" );

		parallel_reduce( blocked_range<size_t>( 0, 100 ), task );
 		BOOST_CHECK( task.errors() == 100000 );
	}

};

struct SceneCacheThreadingTestSuite : public boost::unit_test::test_suite
{

	SceneCacheThreadingTestSuite() : boost::unit_test::test_suite( "SceneCacheThreadingTestSuite" )
	{
		boost::shared_ptr<SceneCacheThreadingTest> instance( new SceneCacheThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &SceneCacheThreadingTest::testAttributeRead, instance ) );
		add( BOOST_CLASS_TEST_CASE( &SceneCacheThreadingTest::testFakeAttributeRead, instance ) );
	}
};

void addSceneCacheThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new SceneCacheThreadingTestSuite( ) );
}

} // namespace IECore
