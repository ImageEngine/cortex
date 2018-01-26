//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "LRUCacheThreadingTest.h"

#include "IECore/LRUCache.h"
#include "IECore/SimpleTypedData.h"

#include "tbb/tbb.h"

#include <iostream>

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct LRUCacheThreadingTest
{

	struct GetFromCache
	{
		public :

			GetFromCache( LRUCache<int, IntDataPtr> &cache )
				:	m_cache( cache )
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				for( size_t i=r.begin(); i!=r.end(); ++i )
				{
					IntDataPtr k = m_cache.get( i );
					// can't use boost unit test assertions from threads
					assert( k->readable() == (int)i );
				}
			}

		private :

			LRUCache<int, IntDataPtr> &m_cache;

	};

	static IntDataPtr get( int key, size_t &cost )
	{
		cost = 10;
		return new IntData( key );
	}

	void test()
	{
		LRUCache<int, IntDataPtr> cache( get, 1000 );

		parallel_for( blocked_range<size_t>( 0, 10000 ), GetFromCache( cache ) );
	}
};


struct LRUCacheThreadingTestSuite : public boost::unit_test::test_suite
{

	LRUCacheThreadingTestSuite() : boost::unit_test::test_suite( "LRUCacheThreadingTestSuite" )
	{
		boost::shared_ptr<LRUCacheThreadingTest> instance( new LRUCacheThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &LRUCacheThreadingTest::test, instance ) );
	}
};

void addLRUCacheThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new LRUCacheThreadingTestSuite( ) );
}

} // namespace IECore
