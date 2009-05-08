//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "maya/MLibrary.h"
#include "maya/MGlobal.h"

#include <boost/test/unit_test.hpp>

#include "ObjectDataTest.h"
#include "MDataHandleAccessorTest.h"
#include "ImageConverterTest.h"

using namespace boost::unit_test;

using namespace IECoreMaya;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	test_suite* test = BOOST_TEST_SUITE( "IECoreMaya unit test" );

	/// \todo Try and find a way of calling MLibrary::cleanup when Boost.Test exits
	/// I think we'd have to rebuild the boost test library with BOOST_TEST_NO_MAIN defined,
	/// then implement our own main() here which simply calls :
	/// int exitStatus = unit_test_main( &init_unit_test_suite, argc, argv );
	/// We can then put our MLibrary calls on either side of that.
	/// While we don't have BOOST_TEST_NO_MAIN a main() gets automatically generated in
	/// libboost_test_exec_monitor
	MStatus s = MLibrary::initialize( argv[0], false );
	if ( !s )
	{
		std::cerr << "Could not initialize Maya standalone application: " << s << std::endl;
		exit( 1 );
        }

	MGlobal::executeCommand( "loadPlugin \"ieCore\"" );

	try
	{
		addObjectDataTest(test);
		addImageConverterTest(test);
	}
	catch ( std::exception &ex )
	{
		std::cerr << "Failed to create test suite: " << ex.what() << std::endl;
		throw;
	}

	return test;
}
