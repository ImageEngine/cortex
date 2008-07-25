//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_OBJECTDATATEST_H
#define IE_COREMAYA_OBJECTDATATEST_H

#include <iostream>

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/operations.hpp"

#include "maya/MGlobal.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MFnPluginData.h"
#include "maya/MString.h"
#include "maya/MSelectionList.h"

#include "IECore/Object.h"
#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ObjectData.h"

using namespace IECore;
namespace fs = boost::filesystem;

namespace IECoreMaya
{

void addObjectDataTest( boost::unit_test::test_suite* test );

struct ObjectDataTest
{	
	void testConstruction()
	{
		MStatus s;
		s = MGlobal::executeCommand( "loadPlugin \"ObjectDataTestNode.py\"" );
		BOOST_CHECK( s );
		
		MFnDependencyNode fnDN;
		
		MObject node = fnDN.create( "ieObjectDataTestNode", &s );
		BOOST_CHECK( s );
		
		MPlug plug = fnDN.findPlug( "objectData", &s );
		BOOST_CHECK( s );		
		BOOST_CHECK( !plug.isNull() );
		
		MObject data;
		s = plug.getValue( data );
		BOOST_CHECK( s );		
		
		MFnPluginData fnData( data, &s );
		BOOST_CHECK( s );		
		
		BOOST_CHECK( fnData.data() );
		
		ObjectData* objectData = dynamic_cast< ObjectData * >( fnData.data() );
		BOOST_CHECK( objectData );
		
		BOOST_CHECK( objectData->getObject() == 0 );				
	}
	
	void testReadWrite()
	{
		MString sceneName;
	
		MStatus s;
		s = MGlobal::executeCommand( "loadPlugin \"ObjectDataTestNode.py\"" );
		BOOST_CHECK( s );
		
		MFnDependencyNode fnDN;
		
		MObject node = fnDN.create( "ieObjectDataTestNode", &s );
		BOOST_CHECK( s );
		
		MString nodeName = fnDN.name();
		
		MPlug plug = fnDN.findPlug( "objectData", &s );
		BOOST_CHECK( s );		
		BOOST_CHECK( !plug.isNull() );
		
		MObject data;
		s = plug.getValue( data );
		BOOST_CHECK( s );		
		
		MFnPluginData fnData( data, &s );
		BOOST_CHECK( s );		
		
		ObjectData* objectData = dynamic_cast< ObjectData * >( fnData.data() );
		BOOST_CHECK( objectData );
				
		CompoundDataPtr testCompoundData = new CompoundData();				
		testCompoundData->writable()["val1"] = new FloatData( 1.0f );
		testCompoundData->writable()["val2"] = new StringData( "val2Data" );
		testCompoundData->writable()["val3"] = new CompoundData();		
		runTimeCast<CompoundData>(testCompoundData->writable()["val3"])->writable()["val3.val1"] = new IntData(100);				
		
		data = fnData.create( ObjectDataId, &s );
		BOOST_CHECK( s );
		objectData = dynamic_cast< ObjectData * >( fnData.data() );		
		BOOST_CHECK( objectData );		
		objectData->setObject( testCompoundData );
		
		s = plug.setValue( data );
		BOOST_CHECK( s );		
								
		s = MGlobal::executeCommand( "file -rename \"ObjectDataTest.ma\"" );
		BOOST_CHECK( s );				
		s = MGlobal::executeCommand( "file -type \"mayaAscii\" -save", sceneName );
		BOOST_CHECK( s );
		s = MGlobal::executeCommand( "file -force -new" );
		BOOST_CHECK( s );		
		s = MGlobal::executeCommand( "file -force -open \"" + sceneName + "\"" );
		BOOST_CHECK( s );
		
		MSelectionList list;
		list.add( nodeName + ".objectData" );		
		
		s = list.getPlug( 0, plug );
		BOOST_CHECK( s );
		s = plug.getValue( data );
		BOOST_CHECK( s );
		s = fnData.setObject( data );	
		BOOST_CHECK( s );
		objectData = dynamic_cast< ObjectData * >( fnData.data() );		
		BOOST_CHECK( objectData );		
		BOOST_CHECK( testCompoundData->isEqualTo( objectData->getObject() ) );
		
		fs::remove( fs::path( sceneName.asChar() ) );		
								
		s = MGlobal::executeCommand( "file -rename \"ObjectDataTest.mb\"" );
		BOOST_CHECK( s );
		s = MGlobal::executeCommand( "file -type \"mayaBinary\" -save", sceneName );
		BOOST_CHECK( s );
		s = MGlobal::executeCommand( "file -force -new" );
		BOOST_CHECK( s );		
		s = MGlobal::executeCommand( "file -force -open \"" + sceneName + "\"" );
		BOOST_CHECK( s );
		
		list.clear();
		list.add( nodeName + ".objectData" );		
		
		s = list.getPlug( 0, plug );
		BOOST_CHECK( s );
		s = plug.getValue( data );
		BOOST_CHECK( s );
		s = fnData.setObject( data );	
		BOOST_CHECK( s );
		objectData = dynamic_cast< ObjectData * >( fnData.data() );		
		BOOST_CHECK( objectData );		
		BOOST_CHECK( testCompoundData->isEqualTo( objectData->getObject() ) );
		
		fs::remove( fs::path( sceneName.asChar() ) );		
	}
};

struct ObjectDataTestSuite : public boost::unit_test::test_suite
{

	ObjectDataTestSuite() : boost::unit_test::test_suite( "ObjectDataTestSuite" )
	{
		static boost::shared_ptr<ObjectDataTest> instance( new ObjectDataTest() );

		add( BOOST_CLASS_TEST_CASE( &ObjectDataTest::testConstruction, instance ) );
		add( BOOST_CLASS_TEST_CASE( &ObjectDataTest::testReadWrite, instance ) );
	}

};

} // namespace IECoreMaya

#endif // IE_COREMAYA_OBJECTDATATEST_H
