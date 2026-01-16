//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023, Image Engine Design Inc. All rights reserved.
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

#include "MurmurHashTest.h"

#include "IECore/MurmurHash.h"

#include <unordered_set>

using namespace boost;
using namespace boost::unit_test;

namespace IECore
{

struct MurmurHashTest
{

	void testUnorderedSet()
	{
		std::unordered_set< IECore::MurmurHash > set;
		for( size_t i = 0; i < 1000000; i++ )
		{
			IECore::MurmurHash h;
			h.append( i );
			set.insert( h );
		}

		BOOST_CHECK( set.size() == 1000000 );

		size_t maxBucketOccupancy = 0;
		for( size_t i = 0; i < set.bucket_count(); i++ )
		{
			maxBucketOccupancy = std::max( maxBucketOccupancy, set.bucket_size( i ) );
		}

		// If our hash function is good, then there shouldn't be any bucket that gets way too
		// many elements in it - currently, I'm seeing a max occupancy of 8.
		BOOST_CHECK( maxBucketOccupancy < 16 );
	}

	void testAllElementsOfImathBoxes()
	{
		Imath::Box3i ref( Imath::V3i( 123, 456, 789 ), Imath::V3i( 10123, 10456, 10789 ) );
		std::unordered_set< IECore::MurmurHash > set;

		for( int i = 0; i < 6; i++ )
		{
			for( int j = 0; j < 32; j++ )
			{
				int bitToFlip = 1 << j;
				Imath::Box3i q = ref;
				if( i >= 3 )
				{
					q.max[i - 3] = bitToFlip ^ q.max[i - 3];
				}
				else
				{
					q.min[i] = bitToFlip ^ q.min[i];
				}

				IECore::MurmurHash h;
				h.append( q );
				set.insert( h );
			}
		}

		BOOST_CHECK( set.size() == ( 6 * 32 ) );
	}
};


struct MurmurHashTestSuite : public boost::unit_test::test_suite
{

	MurmurHashTestSuite() : boost::unit_test::test_suite( "MurmurHashTestSuite" )
	{
		boost::shared_ptr<MurmurHashTest> instance( new MurmurHashTest() );

		add( BOOST_CLASS_TEST_CASE( &MurmurHashTest::testUnorderedSet, instance ) );
		add( BOOST_CLASS_TEST_CASE( &MurmurHashTest::testAllElementsOfImathBoxes, instance ) );
	}
};

void addMurmurHashTest(boost::unit_test::test_suite* test)
{
	test->add( new MurmurHashTestSuite( ) );
}

} // namespace IECore
