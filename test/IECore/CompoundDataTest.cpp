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
		catch ( std::exception &e ) 
		{
		}
		try
		{
			StringData *s = c->member<StringData>( "iAmMissing", true, false );
			BOOST_CHECK( !"Exception not thrown during missing member retrieval." );
			BOOST_CHECK( !s );		
		}
		catch ( std::exception &e ) 
		{
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
};


struct CompoundDataTestSuite : public boost::unit_test::test_suite
{

	CompoundDataTestSuite() : boost::unit_test::test_suite( "CompoundDataTestSuite" )
	{
		boost::shared_ptr<CompoundDataTest> instance( new CompoundDataTest() );
		add( BOOST_CLASS_TEST_CASE( &CompoundDataTest::testMemberRetrieval, instance ) );
	}
};

void addCompoundDataTest(boost::unit_test::test_suite* test)
{
	test->add( new CompoundDataTestSuite( ) );
}

} // namespace IECore
