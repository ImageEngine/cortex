//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "ComputationCacheTest.h"

#include "IECore/ComputationCache.h"
#include "IECore/SimpleTypedData.h"

#include "tbb/tbb.h"

#include <iostream>

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct ComputationCacheTest
{

	static int getCount;
	typedef int ComputationParams;
	typedef ComputationCache< ComputationParams > Cache;

	static MurmurHash hash( const ComputationParams &params )
	{
		int id = params;
		MurmurHash h;
		h.append(id);
		return h;
	}

	static IntDataPtr get( const ComputationParams &params )
	{
		int id = params;
		getCount++;
		return new IntData( id );
	}

	void test()
	{
		ConstObjectPtr res;
		IntDataPtr v = new IntData(1);

		/// limit the pool to fit only one integer.
		ObjectPoolPtr pool = new ObjectPool( v->Object::memoryUsage() );

		Cache cache( get, hash, 1000, pool );

		BOOST_CHECK_EQUAL( pool.get(), cache.objectPool() );

		BOOST_CHECK_EQUAL( size_t(1000), cache.getMaxComputations() );
		cache.setMaxComputations( 100 );
		BOOST_CHECK_EQUAL( size_t(100), cache.getMaxComputations() );
		BOOST_CHECK_EQUAL( size_t(0), cache.cachedComputations() );

		/// cache should return NULL on never computed values (hash is unknown)
		res = cache.get( ComputationParams(2), Cache::NullIfMissing );
		BOOST_CHECK( !res );
		// this is weird, but because we are using LRUCache with a dummie and we don't
		/// want to do two queries, it ends up registering that computation with the default hash...
		BOOST_CHECK_EQUAL( size_t(1), cache.cachedComputations() );

		/// computes a value (default value)
		res = cache.get( ComputationParams(2) );
		BOOST_CHECK( res );
		BOOST_CHECK_EQUAL( size_t(1), cache.cachedComputations() );

		/// computes a value (explicit)
		res = cache.get( ComputationParams(3), Cache::ComputeIfMissing );
		BOOST_CHECK( res );
		BOOST_CHECK_EQUAL( size_t(2), cache.cachedComputations() );

		/// Cache should have evicted one value due to the memory limit
		/// of the ObjectPool.
		ConstObjectPtr res2 = cache.get( ComputationParams( 2 ), Cache::NullIfMissing );
		ConstObjectPtr res3 = cache.get( ComputationParams( 3 ), Cache::NullIfMissing );
		BOOST_CHECK( !(res2 && res3) );
		BOOST_CHECK( res2 || res3 );

		/// now increase memory limit to two IntData objects
		pool->setMaxMemoryUsage( v->Object::memoryUsage() * 2 );

		/// computes two new values
		cache.get( ComputationParams(4) );
		cache.get( ComputationParams(5) );
		BOOST_CHECK_EQUAL( size_t(4), cache.cachedComputations() );
		BOOST_CHECK( cache.get( ComputationParams(4), Cache::NullIfMissing ) );
		BOOST_CHECK( cache.get( ComputationParams(5), Cache::NullIfMissing ) );

		/// clears the all values
		cache.clear();
		pool->clear();
		BOOST_CHECK_EQUAL( size_t(0), cache.cachedComputations() );

		// set some values on the cache
		cache.set( ComputationParams(1), v.get(), ObjectPool::StoreReference );
		BOOST_CHECK_EQUAL( size_t(1), cache.cachedComputations() );
		BOOST_CHECK_EQUAL( v, cache.get( ComputationParams(1), Cache::NullIfMissing ) );
		cache.set( ComputationParams(1), v.get(), ObjectPool::StoreCopy );
		BOOST_CHECK_EQUAL( size_t(1), cache.cachedComputations() );
		BOOST_CHECK_EQUAL( v, cache.get( ComputationParams(1), Cache::NullIfMissing ) );
		cache.clear();
		v = new IntData(41);
		cache.set( ComputationParams(1), v.get(), ObjectPool::StoreCopy );
		BOOST_CHECK_EQUAL( size_t(1), cache.cachedComputations() );
		BOOST_CHECK( *v == *cache.get( ComputationParams(1), Cache::NullIfMissing ) );
		v->writable() = 42;
		BOOST_CHECK( *v != *cache.get( ComputationParams(1), Cache::NullIfMissing ) );

		// test when the computation function does not match the already registered computation hash....
		IntDataPtr weirdValue = new IntData(666);
		cache.clear();
		pool->clear();
		cache.set( ComputationParams(1), weirdValue.get(), ObjectPool::StoreReference );
		ConstObjectPtr v0 = cache.get( ComputationParams(1) );
		BOOST_CHECK( *weirdValue == *cache.get( ComputationParams(1), Cache::NullIfMissing ) );
		pool->clear();
		int c1 = ComputationCacheTest::getCount;
		ConstObjectPtr v1 = cache.get( ComputationParams(1) );
		int c2 = ComputationCacheTest::getCount;
		BOOST_CHECK_EQUAL( 1, static_cast< const IntData * >(v1.get())->readable() );
		BOOST_CHECK_EQUAL( c1 + 1, c2 );
		ConstObjectPtr v2 = cache.get( ComputationParams(1) );
		int c3 = ComputationCacheTest::getCount;
		BOOST_CHECK_EQUAL( 1, static_cast< const IntData * >(v2.get())->readable() );
		/// garantee that there was no recomputation.
		BOOST_CHECK_EQUAL( c2, c3 );
	}

	struct GetFromCache
	{
		public :

			GetFromCache( Cache &cache )
				:	m_cache( cache )
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				for( size_t i=r.begin(); i!=r.end(); ++i )
				{
					int value = int(i) % 500;
					ConstObjectPtr r = m_cache.get( ComputationParams(value) );
					ConstIntDataPtr k = runTimeCast< const IntData >(r);
					// can't use boost unit test assertions from threads
					assert( k.get() );
					assert( k->readable() == value );
				}
			}

		private :

			Cache &m_cache;

	};

	void testThreadedGet()
	{

		Cache cache( get, hash, 10000, new ObjectPool(10000) );

		parallel_for( blocked_range<size_t>( 0, 10000 ), GetFromCache( cache ) );

		BOOST_CHECK_EQUAL( size_t(500), cache.cachedComputations() );
	}

};

int ComputationCacheTest::getCount(0);

struct ComputationCacheTestSuite : public boost::unit_test::test_suite
{

	ComputationCacheTestSuite() : boost::unit_test::test_suite( "ComputationCacheTestSuite" )
	{
		boost::shared_ptr<ComputationCacheTest> instance( new ComputationCacheTest() );

		add( BOOST_CLASS_TEST_CASE( &ComputationCacheTest::test, instance ) );
		add( BOOST_CLASS_TEST_CASE( &ComputationCacheTest::testThreadedGet, instance ) );
	}
};

void addComputationCacheTest(boost::unit_test::test_suite* test)
{
	test->add( new ComputationCacheTestSuite( ) );
}

} // namespace IECore
