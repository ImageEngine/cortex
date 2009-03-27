//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECore/bindings/ImagePrimitiveBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

#include "OpenEXR/half.h"

using namespace boost::python;

namespace IECore
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
		return make_tuple( v, reason );
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
		return make_tuple( v, reason );
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

void bindImagePrimitive()
{
	typedef class_<ImagePrimitive, ImagePrimitivePtr, bases<Primitive>, boost::noncopyable> ImagePrimitivePyClass;
	ImagePrimitivePyClass("ImagePrimitive")
		.def( init<Imath::Box2i, Imath::Box2i>() )

		.add_property("dataWindow", make_function( &ImagePrimitive::getDataWindow,
	                	return_value_policy<copy_const_reference>() ), &ImagePrimitive::setDataWindow )

		.add_property( "displayWindow", make_function( &ImagePrimitive::getDisplayWindow,
	                	return_value_policy<copy_const_reference>() ), &ImagePrimitive::setDisplayWindow )

		.def( "channelValid", &channelValid, ( arg_( "image" ), arg_( "primVar" ), arg_( "wantReason" ) = false ) )
		.def( "channelValid", &channelValid2, ( arg_( "image" ), arg_( "primVarName" ), arg_( "wantReason" ) = false ) )
		.def( "getChannel", &getChannel )
		.def( "channelNames", &channelNames)

		/// \todo Why are we allowing people to create channels of types that are invalid according to our own spec?
		/// Are we relying on this somewhere?
		.def( "createFloatChannel", &ImagePrimitive::createChannel<float> )
		.def( "createHalfChannel", &ImagePrimitive::createChannel<half> )
		.def( "createDoubleChannel", &ImagePrimitive::createChannel<double> )
		.def( "createIntChannel", &ImagePrimitive::createChannel<int> )
		.def( "createUIntChannel", &ImagePrimitive::createChannel<unsigned int> )
		.def( "createShortChannel", &ImagePrimitive::createChannel<short> )
		.def( "createUShortChannel", &ImagePrimitive::createChannel<unsigned short> )
		.def( "createCharChannel", &ImagePrimitive::createChannel<char> )
		.def( "createUCharChannel", &ImagePrimitive::createChannel<unsigned char> )

		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( ImagePrimitive )
	;

	INTRUSIVE_PTR_PATCH( ImagePrimitive, ImagePrimitivePyClass );
	implicitly_convertible<ImagePrimitivePtr, PrimitivePtr>();
}

}
