//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_IMAGECONVERTERTEST_H
#define IE_COREMAYA_IMAGECONVERTERTEST_H

#include <iostream>

#include "boost/test/unit_test.hpp"
#include "boost/filesystem/operations.hpp"

#include "maya/MImage.h"

#include "IECore/ImagePrimitive.h"
#include "IECore/ImageDiffOp.h"
#include "IECore/Reader.h"

#include "IECoreMaya/FromMayaImageConverter.h"
#include "IECoreMaya/ToMayaImageConverter.h"

using namespace IECore;
namespace fs = boost::filesystem;

namespace IECoreMaya
{

void addImageConverterTest( boost::unit_test::test_suite* test );

struct ImageConverterTest
{
	/// \todo Test more than this
	void test()
	{
		ImagePrimitivePtr imageA = runTimeCast<ImagePrimitive>( Reader::create( "test/IECore/data/exrFiles/colorBarsWithAlpha.exr")->read() );
		BOOST_CHECK( imageA );

		MImage mimage;

		ToMayaImageConverterPtr toMaya = ToMayaImageConverter::create( imageA );
		BOOST_CHECK( toMaya );

		MStatus s = toMaya->convert( mimage );
		BOOST_CHECK( s );

		FromMayaImageConverterPtr fromMaya  = new FromMayaImageConverter( mimage );
		BOOST_CHECK( fromMaya );

		ImagePrimitivePtr imageB = runTimeCast<ImagePrimitive>( fromMaya->convert() );
		BOOST_CHECK( imageB );

		ImageDiffOpPtr diffOp = new ImageDiffOp();

		diffOp->imageAParameter()->setValue( imageA );
		diffOp->imageBParameter()->setValue( imageB );
		diffOp->maxErrorParameter()->setNumericValue( 1.0 / 256.0 );

		bool result = runTimeCast< BoolData >( diffOp->operate() )->readable();
		BOOST_CHECK( !result );
	}
};

struct ImageConverterTestSuite : public boost::unit_test::test_suite
{

	ImageConverterTestSuite() : boost::unit_test::test_suite( "ImageConverterTestSuite" )
	{
		static boost::shared_ptr<ImageConverterTest> instance( new ImageConverterTest() );

		add( BOOST_CLASS_TEST_CASE( &ImageConverterTest::test, instance ) );
	}

};

} // namespace IECoreMaya

#endif // IE_COREMAYA_IMAGECONVERTERTEST_H
