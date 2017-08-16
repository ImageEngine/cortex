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

#include "OpenEXR/half.h"

#include "IECoreImage/ImagePrimitive.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreImageBindings/ImagePrimitiveBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreImage;

namespace
{

size_t numChannels( ImagePrimitive &i )
{
	return i.channels.size();
}

static DataPtr getItem( ImagePrimitive &i, const std::string &n )
{
	auto channel = i.channels.find( n );
	if( channel == i.channels.end() )
	{
		throw std::out_of_range( "Bad index" );
	}

	return channel->second;
}

static void setItem( ImagePrimitive &i, const std::string &n, const DataPtr &d )
{
	i.channels[n] = d;
}

static bool contains( ImagePrimitive &i, const std::string &n )
{
	return i.channels.find( n ) != i.channels.end();
}

static boost::python::list keys( ImagePrimitive &i )
{
	boost::python::list result;
	for( const auto &channel : i.channels )
	{
		result.append( channel.first );
	}
	return result;
}

static boost::python::list values( ImagePrimitive &i )
{
	boost::python::list result;
	for( const auto &channel : i.channels )
	{
		result.append( channel.second );
	}
	return result;
}

static void delItem( ImagePrimitive &i, const std::string &n )
{
	const auto it = i.channels.find( n );
	if( it==i.channels.end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	i.channels.erase( it );
}

/// \todo Rewrite the Parameter::valueValid bindings to follow this form? They currently always
/// return a tuple, which is causing lots of coding errors (the tuple is always true, and it's
/// easy to forget a tuple is being returned and expect a bool instead).
static object channelValid( ImagePrimitive &that, Data &d, bool wantReason = false )
{
	if( wantReason )
	{
		std::string reason;
		bool v = that.channelValid( &d, &reason );
		return boost::python::make_tuple( v, reason );
	}
	bool v = that.channelValid( &d );
	return object( v );
}

static object channelValid2( ImagePrimitive &that, const char *n, bool wantReason = false )
{
	if( wantReason )
	{
		std::string reason;
		bool v = that.channelValid( n, &reason );
		return boost::python::make_tuple( v, reason );
	}
	bool v = that.channelValid( n );
	return object( v );
}

static object channelsValid( ImagePrimitive &that, bool wantReason = false )
{
	if( wantReason )
	{
		std::string reason;
		bool v = that.channelsValid( &reason );
		return boost::python::make_tuple( v, reason );
	}
	bool v = that.channelsValid();
	return object( v );
}

static StringVectorDataPtr channelNames( ImagePrimitive &that )
{
	StringVectorDataPtr result( new StringVectorData );
	that.channelNames( result->writable() );
	return result;
}

static DataPtr getChannel( ImagePrimitive &that, const char *name )
{
	std::string reason;
	if( that.channelValid( name, &reason ) )
	{
		return that.channels[name];
	}
	return nullptr;
}

template<typename T>
static DataPtr createChannel( ImagePrimitive &i, const std::string &name )
{
	return i.createChannel<T>( name );
}

} // namespace

namespace IECoreImageBindings
{

void bindImagePrimitive()
{
	scope s = RunTimeTypedClass<ImagePrimitive>()
		.def( init<>() )
		.def( init<Imath::Box2i, Imath::Box2i>() )
		.def( "__len__", &numChannels )
		.def( "__getitem__", &getItem, "Returns a shallow copy of the requested channel data." )
		.def( "__setitem__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "keys", &keys )
		.def( "values", &values, "Returns a list containing shallow copies of all channel data." )

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

		.def( "channelSize", &ImagePrimitive::channelSize )
		.def( "channelValid", &channelValid, ( arg_( "image" ), arg_( "channel" ), arg_( "wantReason" ) = false ) )
		.def( "channelValid", &channelValid2, ( arg_( "image" ), arg_( "channelName" ), arg_( "wantReason" ) = false ) )
		.def( "channelsValid", &channelsValid, ( arg_( "image" ), arg_( "wantReason" ) = false ) )
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
