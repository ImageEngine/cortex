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
			
			TestSceneCache() : m_errors( 0 )
			{
			}

			void operator() ( int instance ) const
			{
				for ( size_t i = 0; i < 1000; i++ )
				{
					if ( (instance + i) % 7 == 0 )
					{
						SharedSceneInterfaces::clear();
					}
					ConstSceneInterfacePtr scene = SharedSceneInterfaces::get("test/IECore/data/sccFiles/attributeAtRoot.scc");
					try 
					{
						scene->readAttribute("w", 0);
					}
					catch( Exception e )
					{
						m_errors++;
					}
				}
			}

			size_t errors() const
			{
				return m_errors;
			}
		
		private :
			mutable size_t m_errors;
			SceneInterface::Path m_attributePath;
	};

	void testAttributeRead()
	{
		const int numThreads = 100;

		TestSceneCache task;
		std::vector< tbb::tbb_thread *> threads;

		for ( int i = 0; i < numThreads; i++ )
		{
			threads.push_back( new tbb::tbb_thread( task, i ) );
		}
		for ( int i = 0; i < numThreads; i++ )
		{
			threads[i]->join();
		}
		for ( int i = 0; i < numThreads; i++ )
		{
			delete threads[i];
		}
		BOOST_CHECK( task.errors() == 0 );

	}

};

struct SceneCacheThreadingTestSuite : public boost::unit_test::test_suite
{

	SceneCacheThreadingTestSuite() : boost::unit_test::test_suite( "SceneCacheThreadingTestSuite" )
	{
		boost::shared_ptr<SceneCacheThreadingTest> instance( new SceneCacheThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &SceneCacheThreadingTest::testAttributeRead, instance ) );
	}
};

void addSceneCacheThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new SceneCacheThreadingTestSuite( ) );
}

} // namespace IECore
