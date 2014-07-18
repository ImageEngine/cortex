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

#include <cassert>

#include "boost/static_assert.hpp"

#include "IECore/ImagePrimitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/Renderer.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MurmurHash.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

static IndexedIO::EntryID g_displayWindowMinXEntry("displayWindowMinX");
static IndexedIO::EntryID g_displayWindowMinYEntry("displayWindowMinY");
static IndexedIO::EntryID g_displayWindowMaxXEntry("displayWindowMaxX");
static IndexedIO::EntryID g_displayWindowMaxYEntry("displayWindowMaxY");
static IndexedIO::EntryID g_dataWindowMinXEntry("dataWindowMinX");
static IndexedIO::EntryID g_dataWindowMinYEntry("dataWindowMinY");
static IndexedIO::EntryID g_dataWindowMaxXEntry("dataWindowMaxX");
static IndexedIO::EntryID g_dataWindowMaxYEntry("dataWindowMaxY");

const unsigned int ImagePrimitive::m_ioVersion = 1;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(ImagePrimitive);

ImagePrimitive::ImagePrimitive()
{
}

ImagePrimitive::ImagePrimitive( const Box2i &dataWindow, const Box2i &displayWindow )
{
	setDataWindow( dataWindow );
	setDisplayWindow( displayWindow );
}

Box3f ImagePrimitive::bound() const
{
	assert( ! m_displayWindow.isEmpty() );

	/// \todo We might need to include any pixel aspect ratio in this bound

	V3f boxMin( m_displayWindow.min.x, m_displayWindow.min.y, 0.0 );

	/// We add one here because the displayWindow is measured in pixels, and is inclusive. That is, an image
	/// which has a displayWindow of (0,0)->(0,0) contains exactly one pixel.

	V3f boxMax( 1.0f + m_displayWindow.max.x, 1.0f + m_displayWindow.max.y, 0.0 );

	V3f center = (boxMin + boxMax) / 2.0;

	return Box3f( boxMin - center, boxMax - center );
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

// give the size of the image
size_t ImagePrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch (interpolation)
	{

	case PrimitiveVariable::Vertex:
	case PrimitiveVariable::Varying:
	case PrimitiveVariable::FaceVarying:
		return ( 1 + m_dataWindow.max.x - m_dataWindow.min.x ) * ( 1 + m_dataWindow.max.y - m_dataWindow.min.y );

	default:
		return 1;

	}
}

void ImagePrimitive::render( Renderer *renderer ) const
{
	assert( renderer );

	renderer->image(m_dataWindow, m_displayWindow, variables);
}

//
// handling for serialization
//
void ImagePrimitive::copyFrom( const IECore::Object *rhs, IECore::Object::CopyContext *context )
{
	assert( rhs );
	assert( context );

	Primitive::copyFrom(rhs, context);
	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs);

	m_displayWindow = p_rhs->getDisplayWindow();
	m_dataWindow = p_rhs->getDataWindow();
}

void ImagePrimitive::save(IECore::Object::SaveContext *context) const
{
	assert( context );

	Primitive::save(context);
	IndexedIOPtr container = context->container(staticTypeName(), m_ioVersion);

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

	Primitive::load(context);
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container(staticTypeName(), v);

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

bool ImagePrimitive::isEqualTo(const IECore::Object *rhs) const
{
	assert( rhs );

	if (!Primitive::isEqualTo(rhs))
	{
		return false;
	}

	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs);

	// return true iff we have the same data window.
	// this is not complete
	return m_dataWindow == p_rhs->getDataWindow() && m_displayWindow == p_rhs->getDisplayWindow();
}

void ImagePrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof(m_displayWindow) );
	a.accumulate( sizeof(m_dataWindow) );
}

void ImagePrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void ImagePrimitive::topologyHash( MurmurHash &h ) const
{
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

bool ImagePrimitive::channelValid( const PrimitiveVariable &pv, std::string *reason ) const
{
	if( pv.interpolation!=PrimitiveVariable::Vertex &&
		pv.interpolation!=PrimitiveVariable::Varying &&
		pv.interpolation!=PrimitiveVariable::FaceVarying )
	{
		if( reason )
		{
			*reason = "Primitive variable has inappropriate interpolation.";
		}
		return false;
	}
	if( !pv.data )
	{
		if( reason )
		{
			*reason = "Primitive variable has no data.";
		}
		return false;
	}

	if( !despatchTraitsTest<TypeTraits::IsNumericVectorTypedData>( pv.data.get() ) )
	{
		if( reason )
		{
			*reason = "Primitive variable has inappropriate type.";
		}
		return false;
	}

	size_t size = despatchTypedData<TypedDataSize>( pv.data.get() );
	size_t numPixels = variableSize( PrimitiveVariable::Vertex );
	if( size!=numPixels )
	{
		if( reason )
		{
			*reason = str( format( "Primitive variable has wrong size (%d but should be %d)." ) % size % numPixels );
		}
		return false;
	}

	return true;
}

bool ImagePrimitive::channelValid( const std::string &name, std::string *reason ) const
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it==variables.end() )
	{
		if( reason )
		{
			*reason = str( format( "Primitive variable \"%s\" does not exist." ) % name );
		}
		return false;
	}
	return channelValid( it->second, reason );
}

void ImagePrimitive::channelNames( vector<string> &names ) const
{
	// copy in the names of channels from the map
	names.clear();

	for ( PrimitiveVariableMap::const_iterator i = variables.begin(); i != variables.end(); ++i )
	{
		if( channelValid( i->second ) )
		{
			names.push_back( i->first );
		}
	}
}

