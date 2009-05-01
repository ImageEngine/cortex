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

#include "boost/python.hpp"

#include "IECore/ColorSpaceTransformOp.h"
#include "IECore/bindings/ColorSpaceTransformOpBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;

namespace IECore
{

template<typename T>
static list vectorToList( const std::vector<T> &v )
{
	list r;
	for ( typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it )
	{
		r.append( *it );
	}
	return r;
}

static list inputColorSpaces()
{
	std::vector<std::string> x;
	ColorSpaceTransformOp::inputColorSpaces( x );
	return vectorToList( x );
}

static list outputColorSpaces()
{
	std::vector<std::string> x;
	ColorSpaceTransformOp::outputColorSpaces( x );
	return vectorToList( x );
}

static list colorSpaces()
{
	std::vector<std::string> x;
	ColorSpaceTransformOp::colorSpaces( x );
	return vectorToList( x );
}


static ImagePrimitiveOpPtr creator( const std::string &inputColorSpace, const std::string &outputColorSpace, void *data )
{
        assert( data );
        PyObject *d = (PyObject *)( data );
        
        ImagePrimitiveOpPtr r = call< ImagePrimitiveOpPtr >( d, inputColorSpace, outputColorSpace );
        return r;
}

static void registerConversion( const std::string &inputColorSpace, const std::string &outputColorSpace, PyObject *createFn )
{
	Py_INCREF( createFn );
	ColorSpaceTransformOp::registerConversion( inputColorSpace, outputColorSpace, &creator, (void*)createFn );
}

void bindColorSpaceTransformOp()
{

	RunTimeTypedClass<ColorSpaceTransformOp>()
		.def( init<>() )
		
		.def( "registerConversion", registerConversion ).staticmethod( "registerConversion" )
		
		.def( "inputColorSpaces", inputColorSpaces ).staticmethod( "inputColorSpaces" )
		.def( "outputColorSpaces", outputColorSpaces ).staticmethod( "outputColorSpaces" )
		.def( "colorSpaces", colorSpaces ).staticmethod( "colorSpaces" )				
	;

}

} // namespace IECore

