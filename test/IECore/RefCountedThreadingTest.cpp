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

#include "RefCountedThreadingTest.h"

#include "IECore/RefCounted.h"

#include "tbb/parallel_for.h"

#include <iostream>
#include <vector>

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct RefCountedThreadingTest
{

	struct TestRefCount
	{
		public :

			TestRefCount( int bufferSize, bool testAssign ) : m_testAssign( testAssign )
			{
				for ( int i = 0; i < bufferSize; i++ )
				{
					m_buffer.push_back( new RefCounted() );
				}
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				if ( m_testAssign )
				{
					// test assignment operator
					for( size_t i=r.begin(); i!=r.end(); ++i )
					{
						RefCountedPtr tmpPtr(0);
						tmpPtr = m_buffer[ i % m_buffer.size() ];
						tmpPtr->refCount();
					}
				}
				else
				{
					// test copy constructor
					for( size_t i=r.begin(); i!=r.end(); ++i )
					{
						RefCountedPtr tmpPtr = m_buffer[ i % m_buffer.size() ];
						tmpPtr->refCount();
					}
				}
			}

			void checkRefCount()
			{
				int error = 0;
				for ( std::vector< RefCountedPtr >::const_iterator it = m_buffer.begin(); it != m_buffer.end(); it++ )
				{
					if ( (*it)->refCount() != 1 )
					{
						error++;
					}
				}
				BOOST_CHECK( error == 0 );
			}

		private :
			bool m_testAssign;
			std::vector< RefCountedPtr > m_buffer;
	};

	void testCopyConstructor()
	{
		TestRefCount task( 10, false );
		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, 10000000 ), task, taskGroupContext );
		task.checkRefCount();
	}

	void testAssignment()
	{
		TestRefCount task( 10, true );
		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, 10000000 ), task, taskGroupContext );
		task.checkRefCount();
	}

};

struct RefCountedThreadingTestSuite : public boost::unit_test::test_suite
{

	RefCountedThreadingTestSuite() : boost::unit_test::test_suite( "RefCountedThreadingTestSuite" )
	{
		boost::shared_ptr<RefCountedThreadingTest> instance( new RefCountedThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &RefCountedThreadingTest::testCopyConstructor, instance ) );
		add( BOOST_CLASS_TEST_CASE( &RefCountedThreadingTest::testAssignment, instance ) );
	}
};

void addRefCountedThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new RefCountedThreadingTestSuite( ) );
}

} // namespace IECore
