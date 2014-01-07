//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include <iostream>

#include "CompoundDataTest.h" 

#include "IECore/CompoundData.h" 
#include "IECore/TypedData.h" 
#include "IECore/SimpleTypedData.h" 
#include "IECore/MemoryIndexedIO.h"

using namespace boost;
using namespace boost::unit_test;

namespace IECore
{

struct CompoundDataTest
{
	void testMemberRetrieval()
	{
		
		CompoundDataPtr c = new CompoundData();
		
		c->writable()["floatElement"] = new FloatData( 42.0f );
		c->writable()["stringElement"] = new StringData( "cake" );
		
		try
		{
			FloatData *f = c->member<FloatData>( "floatElement", false );
			BOOST_CHECK( f );
			BOOST_CHECK( f->staticTypeId() == FloatData::staticTypeId() );
			
			IntData *i = c->member<IntData>( "floatElement", false );
			BOOST_CHECK( !i );
			
			StringData *s = c->member<StringData>( "iAmMissing", false );
			BOOST_CHECK( !s );
			
		}
		catch ( std::exception &e ) 
		{
			BOOST_WARN( !e.what() );
			BOOST_CHECK( !"Exception thrown during member retrieval with exceptions disabled." );
		}
		
		try
		{
			FloatData *f = c->member<FloatData>( "floatElement", true );
			BOOST_REQUIRE( f );
			BOOST_CHECK( f->staticTypeId() == FloatData::staticTypeId() );
			
			StringData *s = c->member<StringData>( "stringElement", true );
			BOOST_REQUIRE( s );
			BOOST_CHECK( s->staticTypeId() == StringData::staticTypeId() );
		}
		catch ( std::exception &e ) 
		{
			BOOST_WARN( !e.what() );
			BOOST_CHECK( !"Exception thrown during member retrieval." );
		}
		
		try
		{
			IntData *i = c->member<IntData>( "floatElement", true );
			BOOST_CHECK( !"Exception not thrown during invalid member retrieval." );		
			BOOST_CHECK( !i );
		}
		catch ( IECore::Exception &e )
		{
		}
		catch( ... )
		{
			BOOST_CHECK( !"Incorrect exception type thrown during invalid member retrieval." );
		}
		
		try
		{
			StringData *s = c->member<StringData>( "iAmMissing", true, false );
			BOOST_CHECK( !"Exception not thrown during missing member retrieval." );
			BOOST_CHECK( !s );		
		}
		catch ( IECore::Exception &e )
		{
		}
		catch( ... )
		{
			BOOST_CHECK( !"Incorrect exception type thrown during invalid member retrieval." );
		}
		
		try
		{
			StringData *s = c->member<StringData>( "iAmMissing", true, true );
			BOOST_REQUIRE( s );
			BOOST_CHECK( s->staticTypeId() == StringData::staticTypeId() );
		
			FloatData *f = c->member<CompoundData>( "newParent", true, true )->member<FloatData>( "newChild", true, true );
			BOOST_REQUIRE( f );
			BOOST_CHECK( f->staticTypeId() == FloatData::staticTypeId() );
		}
		catch ( std::exception &e ) 
		{
			BOOST_CHECK( !"Exception thrown during creation of member." );
		}
	}

	void testNullData()
	{
		CompoundDataPtr d = new CompoundData();
		d->writable()["floatElement"] = new FloatData( 42.0f );
		d->writable()["stringElement"] = new StringData( "cake" );

		// sanity check first.
		try
		{
			CompoundDataPtr dd = d->copy();
		}
		catch ( std::exception &e ) 
		{
			BOOST_CHECK( !"Exception thrown during CompoundData copy." );
		}

		CompoundDataPtr c = new CompoundData();
		c->writable()["nullElement"] = 0;

		// copy
		try
		{
			CompoundDataPtr d = c->copy();

			BOOST_CHECK( !"Exception not thrown during copy with invalid NULL data." );
		}
		catch ( std::exception &e )
		{
		}

		// save
		try
		{
			IndexedIOPtr io = new MemoryIndexedIO( NULL, IndexedIO::rootPath, IndexedIO::Write);
			IndexedIO::EntryID entryName( "test" );
			c->Object::save( io, entryName );

			BOOST_CHECK( !"Exception not thrown during save with invalid NULL data." );
		}
		catch ( std::exception &e )
		{
		}
		
		// memoryUsage
		try
		{
			c->Object::memoryUsage();
		}
		catch ( std::exception &e )
		{
			BOOST_CHECK( !"Exception thrown during memoryCopy with invalid NULL data." );
		}

		// isEqual
		try
		{
			CompoundDataPtr c2 = new CompoundData();
			c2->writable()["nullElement"] = 0;

			bool i = c->isEqualTo( c );
			BOOST_CHECK( i );

			i = c->isEqualTo( c2 );
			BOOST_CHECK( i );

			i = c2->isEqualTo( c );
			BOOST_CHECK( i );

			i = c->isEqualTo( d );
			BOOST_CHECK( !i );

			i = d->isEqualTo( c );
			BOOST_CHECK( !i );

		}
		catch ( std::exception &e )
		{
			BOOST_CHECK( !"Exception thrown during isEqual with invalid NULL data." );
		}

		// hash
		try
		{
			c->Object::hash();

			BOOST_CHECK( !"Exception not thrown during hash with invalid NULL data." );
		}
		catch ( std::exception &e )
		{
		}

	}

};


struct CompoundDataTestSuite : public boost::unit_test::test_suite
{

	CompoundDataTestSuite() : boost::unit_test::test_suite( "CompoundDataTestSuite" )
	{
		boost::shared_ptr<CompoundDataTest> instance( new CompoundDataTest() );
		add( BOOST_CLASS_TEST_CASE( &CompoundDataTest::testMemberRetrieval, instance ) );
		add( BOOST_CLASS_TEST_CASE( &CompoundDataTest::testNullData, instance ) );
	}
};

void addCompoundDataTest(boost::unit_test::test_suite* test)
{
	test->add( new CompoundDataTestSuite( ) );
}

} // namespace IECore
