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
//	     other contributors to this software may be used to endorse or
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

#ifndef IECOREIMAGE_IMAGEPRIMITIVE_INL
#define IECOREIMAGE_IMAGEPRIMITIVE_INL

#include "boost/format.hpp"
#include "boost/static_assert.hpp"

#include "OpenEXR/half.h"

#include "IECore/Exception.h"
#include "IECore/TypeTraits.h"

namespace IECoreImage
{

template<typename T>
IECore::TypedData<std::vector<T> > *ImagePrimitive::getChannel( const std::string &name )
{
	std::string reason = "";
	if( channelValid( name, &reason ) )
	{
		return IECore::runTimeCast<IECore::TypedData<std::vector<T> > >( channels.find( name )->second.get() );
	}
	return nullptr;
}

template<typename T>
const IECore::TypedData<std::vector<T> > *ImagePrimitive::getChannel( const std::string &name ) const
{
	std::string reason = "";
	if( channelValid( name, &reason ) )
	{
		return IECore::runTimeCast<IECore::TypedData<std::vector<T> > >( channels.find( name )->second.get() );
	}
	return nullptr;
}

template<typename T>
IECore::TypedData<std::vector<T> > *ImagePrimitive::createChannel( const std::string &name )
{
	/// This assert enforces the comments regarding permissible channel types in ImagePrimitive.h
	BOOST_STATIC_ASSERT( (
		boost::mpl::or_<
			boost::is_same< T, float >,
			boost::is_same< T, unsigned int >,
			boost::is_same< T, half >
		>::value
	) );
	typename IECore::TypedData<std::vector<T> >::Ptr channel = new IECore::TypedData<std::vector<T> >;

	typename std::vector<T>::size_type area = ( 1 + m_dataWindow.max.x - m_dataWindow.min.x ) * ( 1 + m_dataWindow.max.y - m_dataWindow.min.y );

	channel->writable().resize( area );

	channels.insert( ChannelMap::value_type( name, channel ) );

	return channel.get();
}


//! @name Creation
/// Functinos to assist with creation of common types of ImagePrimitives
template<typename T>
ImagePrimitivePtr ImagePrimitive::createRGB( const Imath::Color3<T> &fillColor, const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow )
{

	ImagePrimitivePtr result = new ImagePrimitive( dataWindow, displayWindow );

	typename IECore::TypedData<std::vector<T> >::Ptr rData = result->createChannel<T>( "R" );
	typename IECore::TypedData<std::vector<T> >::Ptr gData = result->createChannel<T>( "G" );
	typename IECore::TypedData<std::vector<T> >::Ptr bData = result->createChannel<T>( "B" );

	std::fill( rData->writable().begin(), rData->writable().end(), fillColor[0] );
	std::fill( gData->writable().begin(), gData->writable().end(), fillColor[1] );
	std::fill( bData->writable().begin(), bData->writable().end(), fillColor[2] );

	return result;
}

template<typename T>
ImagePrimitivePtr ImagePrimitive::createGreyscale( const T fillValue, const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow )
{
	ImagePrimitivePtr result = new ImagePrimitive( dataWindow, displayWindow );

	typename IECore::TypedData<std::vector<T> >::Ptr yData = result->createChannel<T>( "Y" );

	std::fill( yData->writable().begin(), yData->writable().end(), fillValue );

	return result;
}


}

#endif // IECOREIMAGE_IMAGEPRIMITIVE_INL
