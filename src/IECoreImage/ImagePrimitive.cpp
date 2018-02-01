//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreImage/ImagePrimitive.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/MurmurHash.h"
#include "IECore/TypeTraits.h"

#include "boost/static_assert.hpp"

#include <cassert>

using namespace std;
using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

static IndexedIO::EntryID g_channelsEntry("channels");
static IndexedIO::EntryID g_dataEntry("data");
// for backwards compatibility with m_ioVersion 1
static IndexedIO::EntryID g_variablesEntry("variables");

static IndexedIO::EntryID g_displayWindowMinXEntry("displayWindowMinX");
static IndexedIO::EntryID g_displayWindowMinYEntry("displayWindowMinY");
static IndexedIO::EntryID g_displayWindowMaxXEntry("displayWindowMaxX");
static IndexedIO::EntryID g_displayWindowMaxYEntry("displayWindowMaxY");
static IndexedIO::EntryID g_dataWindowMinXEntry("dataWindowMinX");
static IndexedIO::EntryID g_dataWindowMinYEntry("dataWindowMinY");
static IndexedIO::EntryID g_dataWindowMaxXEntry("dataWindowMaxX");
static IndexedIO::EntryID g_dataWindowMaxYEntry("dataWindowMaxY");

const unsigned int ImagePrimitive::m_ioVersion = 2;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(ImagePrimitive);

ImagePrimitive::ImagePrimitive()
{
}

ImagePrimitive::ImagePrimitive( const Box2i &dataWindow, const Box2i &displayWindow )
{
	setDataWindow( dataWindow );
	setDisplayWindow( displayWindow );
}

ImagePrimitive::~ImagePrimitive()
{
}

const Box2i &ImagePrimitive::getDataWindow() const
{
	return m_dataWindow;
}

void ImagePrimitive::setDataWindow( const Box2i &dataWindow )
{
	m_dataWindow = dataWindow;
}

const Box2i &ImagePrimitive::getDisplayWindow() const
{
	return m_displayWindow;
}

void ImagePrimitive::setDisplayWindow( const Box2i &displayWindow )
{
	if ( displayWindow.isEmpty() )
	{
		throw InvalidArgumentException( "ImagePrimitive: Cannot set displayWindow to the empty window" );
	}

	m_displayWindow = displayWindow;
}

//
// handling for serialization
//
void ImagePrimitive::copyFrom( const IECore::Object *rhs, IECore::Object::CopyContext *context )
{
	assert( rhs );
	assert( context );

	BlindDataHolder::copyFrom(rhs, context);
	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs);

	channels.clear();
	for( const auto &item : p_rhs->channels )
	{
		channels.insert( ChannelMap::value_type( item.first, context->copy<Data>( item.second.get() ) ) );
	}

	m_displayWindow = p_rhs->getDisplayWindow();
	m_dataWindow = p_rhs->getDataWindow();
}

void ImagePrimitive::save(IECore::Object::SaveContext *context) const
{
	assert( context );

	BlindDataHolder::save(context);
	IndexedIOPtr container = context->container(staticTypeName(), m_ioVersion);
	IndexedIOPtr ioChannels = container->subdirectory( g_channelsEntry, IndexedIO::CreateIfMissing );
	for( const auto &item : channels )
	{
		IndexedIOPtr ioChannel = ioChannels->subdirectory( item.first, IndexedIO::CreateIfMissing );
		context->save( item.second.get(), ioChannel.get(), g_dataEntry );
	}

	container->write(g_displayWindowMinXEntry, m_displayWindow.min.x);
	container->write(g_displayWindowMinYEntry, m_displayWindow.min.y);
	container->write(g_displayWindowMaxXEntry, m_displayWindow.max.x);
	container->write(g_displayWindowMaxYEntry, m_displayWindow.max.y);

	container->write(g_dataWindowMinXEntry, m_dataWindow.min.x);
	container->write(g_dataWindowMinYEntry, m_dataWindow.min.y);
	container->write(g_dataWindowMaxXEntry, m_dataWindow.max.x);
	container->write(g_dataWindowMaxYEntry, m_dataWindow.max.y);
}

void ImagePrimitive::load(IECore::Object::LoadContextPtr context)
{
	assert( context );

	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container(staticTypeName(), v);

	BlindDataHolder::load( context );

	// we changed the inheritance hierarchy at io version 2
	ConstIndexedIOPtr ioChannels = ( v < 2 ) ? container->subdirectory( g_variablesEntry ) : container->subdirectory( g_channelsEntry );

	channels.clear();
	IndexedIO::EntryIDList names;
	ioChannels->entryIds( names, IndexedIO::Directory );
	for( const auto &name : names )
	{
		ConstIndexedIOPtr ioChannel = ioChannels->subdirectory( name );
		channels.insert(
			ChannelMap::value_type( name, context->load<Data>( ioChannel.get(), g_dataEntry ) )
		);
	}

	container->read(g_displayWindowMinXEntry, m_displayWindow.min.x);
	container->read(g_displayWindowMinYEntry, m_displayWindow.min.y);
	container->read(g_displayWindowMaxXEntry, m_displayWindow.max.x);
	container->read(g_displayWindowMaxYEntry, m_displayWindow.max.y);

	if ( v < 1 )
	{
		m_dataWindow = m_displayWindow;
	}
	else
	{
		container->read(g_dataWindowMinXEntry, m_dataWindow.min.x);
		container->read(g_dataWindowMinYEntry, m_dataWindow.min.y);
		container->read(g_dataWindowMaxXEntry, m_dataWindow.max.x);
		container->read(g_dataWindowMaxYEntry, m_dataWindow.max.y);
	}
}

bool ImagePrimitive::isEqualTo( const IECore::Object *other ) const
{
	if( !BlindDataHolder::isEqualTo( other ) )
	{
		return false;
	}

	const ImagePrimitive *tOther = static_cast<const ImagePrimitive *>( other );

	if( m_dataWindow != tOther->getDataWindow() || m_displayWindow != tOther->getDisplayWindow() )
	{
		return false;
	}

	for( const auto &channel : channels )
	{
		const auto otherIt = tOther->channels.find( channel.first );
		if( otherIt == tOther->channels.end() )
		{
			return false;
		}

		if( ( channel.second && !otherIt->second ) || ( !channel.second && otherIt->second ) )
		{
			return false;
		}

		if( channel.second && otherIt->second && !channel.second->isEqualTo( otherIt->second.get() ) )
		{
			return false;
		}
	}

	return true;
}

void ImagePrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	BlindDataHolder::memoryUsage( a );

	for( const auto &channel : channels )
	{
		a.accumulate( channel.second.get() );
	}

	a.accumulate( sizeof(m_displayWindow) );
	a.accumulate( sizeof(m_dataWindow) );
}

void ImagePrimitive::hash( MurmurHash &h ) const
{
	BlindDataHolder::hash( h );

	for( const auto &channel : channels )
	{
		h.append( channel.first );
		channel.second->hash( h );
	}

	h.append( m_dataWindow );
	h.append( m_displayWindow );
}

///////////////////////////////////////////////////////////////////////////////////////////
// Space methods
///////////////////////////////////////////////////////////////////////////////////////////

M33f ImagePrimitive::objectToUVMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size() + V2i( 1 );
	result.translate( V2f( 0.5f ) );
	result.scale( V2f( 1.0f ) / V2f( size.x, -size.y ) );
	return result;
}

M33f ImagePrimitive::uvToObjectMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size() + V2i( 1 );
	result.scale( V2f( size.x, -size.y ) );
	result.translate( V2f( -0.5f ) );
	return result;
}

M33f ImagePrimitive::objectToPixelMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size();
	result.translate( V2f( displayWindow.min.x, displayWindow.min.y ) + V2f( size.x, size.y ) / 2.0f );
	result.scale( V2f( 1.0f, -1.0f ) );
	return result;
}

M33f ImagePrimitive::pixelToObjectMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size();
	result.scale( V2f( 1.0f, -1.0f ) );
	result.translate( -V2f( displayWindow.min.x, displayWindow.min.y ) - V2f( size.x, size.y ) / 2.0f );
	return result;
}

M33f ImagePrimitive::pixelToUVMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size() + V2i( 1 );
	result.scale( V2f( 1.0f ) / V2f( size.x, size.y ) );
	result.translate( V2f( 0.5f ) - V2f( displayWindow.min.x, displayWindow.min.y ) );
	return result;
}

M33f ImagePrimitive::uvToPixelMatrix() const
{
	M33f result;
	const Box2i &displayWindow = getDisplayWindow();
	V2i size = displayWindow.size() + V2i( 1 );
	result.translate( V2f( displayWindow.min.x, displayWindow.min.y ) - V2f( 0.5f ) );
	result.scale( V2f( size.x, size.y ) );
	return result;
}

Imath::M33f ImagePrimitive::matrix( Space inputSpace, Space outputSpace ) const
{
	switch( inputSpace )
	{
		case Pixel :

			switch( outputSpace )
			{
				case Pixel :
					return M33f();
				case UV :
					return pixelToUVMatrix();
				case Object :
					return pixelToObjectMatrix();
				default :
					throw Exception( "Unknown output space" );
			}

		case UV :

			switch( outputSpace )
			{
				case Pixel :
					return uvToPixelMatrix();
				case UV :
					return M33f();
				case Object :
					return uvToObjectMatrix();
				default :
					throw Exception( "Unknown output space" );
			}

		case Object :

			switch( outputSpace )
			{
				case Pixel :
					return objectToPixelMatrix();
				case UV :
					return objectToUVMatrix();
				case Object :
					return M33f();
				default :
					throw Exception( "Unknown output space" );
			}

		default :

			throw Exception( "Unknown input space" );

	};
}

///////////////////////////////////////////////////////////////////////////////////////////
// Channel methods
///////////////////////////////////////////////////////////////////////////////////////////

size_t ImagePrimitive::channelSize() const
{
	return ( 1 + m_dataWindow.max.x - m_dataWindow.min.x ) * ( 1 + m_dataWindow.max.y - m_dataWindow.min.y );
}

bool ImagePrimitive::channelValid( const IECore::Data *data, std::string *reason ) const
{
	if( !data )
	{
		if( reason )
		{
			*reason = "Channel has no data.";
		}
		return false;
	}

	if( !despatchTraitsTest<TypeTraits::IsNumericVectorTypedData>( data ) )
	{
		if( reason )
		{
			*reason = "Channel data has inappropriate type.";
		}
		return false;
	}

	size_t size = despatchTypedData<TypedDataSize>( const_cast<Data *>( data ) );
	size_t numPixels = channelSize();
	if( size!=numPixels )
	{
		if( reason )
		{
			*reason = str( format( "Channel has wrong size (%d but should be %d)." ) % size % numPixels );
		}
		return false;
	}

	return true;
}

bool ImagePrimitive::channelValid( const std::string &name, std::string *reason ) const
{
	ChannelMap::const_iterator it = channels.find( name );
	if( it==channels.end() )
	{
		if( reason )
		{
			*reason = str( format( "Channel \"%s\" does not exist." ) % name );
		}
		return false;
	}
	return channelValid( it->second.get(), reason );
}

bool ImagePrimitive::channelsValid( std::string *reason ) const
{
	for( const auto &channel : channels )
	{
		if( !channelValid( channel.second.get(), reason ) )
		{
			return false;
		}
	}

	return true;
}

void ImagePrimitive::channelNames( vector<string> &names ) const
{
	// copy in the names of channels from the map
	names.clear();

	for( const auto &channel : channels )
	{
		if( channelValid( channel.second.get() ) )
		{
			names.push_back( channel.first );
		}
	}
}

