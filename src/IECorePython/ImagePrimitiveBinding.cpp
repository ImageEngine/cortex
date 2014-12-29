//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ImagePrimitive.h"
#include "IECorePython/ImagePrimitiveBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "OpenEXR/half.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

/// \todo Rewrite the Parameter::valueValid bindings to follow this form? They currently always
/// return a tuple, which is causing lots of coding errors (the tuple is always true, and it's
/// easy to forget a tuple is being returned and expect a bool instead).
static object channelValid( ImagePrimitive &that, PrimitiveVariable &p, bool wantReason=false )
{
	if( wantReason )
	{
		std::string reason;
		bool v = that.channelValid( p, &reason );
		return (object)make_tuple( v, reason );
	}
	bool v = that.channelValid( p );
	return object( v );
}

static object channelValid2( ImagePrimitive &that, const char * n, bool wantReason=false )
{
	if( wantReason )
	{
		std::string reason;
		bool v = that.channelValid( n, &reason );
		return (object)make_tuple( v, reason );
	}
	bool v = that.channelValid( n );
	return object( v );
}

static DataPtr getChannel( ImagePrimitive &that, const char *name )
{
	std::string reason;
	if( that.channelValid( name, &reason ) )
	{
		return that.variables[name].data;
	}
	return 0;
}

static StringVectorDataPtr channelNames( ImagePrimitive &that )
{
	StringVectorDataPtr result( new StringVectorData );
	that.channelNames( result->writable() );
	return result;
}

template<typename T>
static DataPtr createChannel( ImagePrimitive &i, const std::string &name )
{
	return i.createChannel<T>( name );
}

void bindImagePrimitive()
{
	scope s = RunTimeTypedClass<ImagePrimitive>()
		.def( init<>() )
		.def( init<Imath::Box2i, Imath::Box2i>() )

		.add_property("dataWindow", make_function( &ImagePrimitive::getDataWindow,
	                	return_value_policy<copy_const_reference>() ), &ImagePrimitive::setDataWindow )

		.add_property( "displayWindow", make_function( &ImagePrimitive::getDisplayWindow,
	                	return_value_policy<copy_const_reference>() ), &ImagePrimitive::setDisplayWindow )

		.def( "objectToUVMatrix", &ImagePrimitive::objectToUVMatrix )
		.def( "uvToObjectMatrix", &ImagePrimitive::uvToObjectMatrix )

		.def( "objectToPixelMatrix", &ImagePrimitive::objectToPixelMatrix )
		.def( "pixelToObjectMatrix", &ImagePrimitive::pixelToObjectMatrix )
		
		.def( "pixelToUVMatrix", &ImagePrimitive::pixelToUVMatrix )
		.def( "uvToPixelMatrix", &ImagePrimitive::uvToPixelMatrix )

		.def( "matrix", &ImagePrimitive::matrix )

		.def( "channelValid", &channelValid, ( arg_( "image" ), arg_( "primVar" ), arg_( "wantReason" ) = false ) )
		.def( "channelValid", &channelValid2, ( arg_( "image" ), arg_( "primVarName" ), arg_( "wantReason" ) = false ) )
		.def( "getChannel", &getChannel )
		.def( "channelNames", &channelNames)

		.def( "createFloatChannel", &createChannel<float> )
		.def( "createHalfChannel", &createChannel<half> )
		.def( "createUIntChannel", &createChannel<unsigned int> )

		.def( "createRGBFloat", &ImagePrimitive::createRGB<float> )
		.staticmethod( "createRGBFloat" )
		
		.def( "createGreyscaleFloat", &ImagePrimitive::createGreyscale<float> )
		.staticmethod( "createGreyscaleFloat" )
		
	;
	
	enum_<ImagePrimitive::Space>( "Space" )
		.value( "Invalid", ImagePrimitive::Invalid )
		.value( "Pixel", ImagePrimitive::Pixel )
		.value( "UV", ImagePrimitive::UV )
		.value( "Object", ImagePrimitive::Object )
	;		

}

}
