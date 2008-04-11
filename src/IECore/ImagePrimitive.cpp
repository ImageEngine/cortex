//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

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

void ImagePrimitive::channelNames( vector<string> &names ) const
{
	// copy in the names of channels from the map
	names.clear();
	
	for ( PrimitiveVariableMap::const_iterator i = variables.begin(); i != variables.end(); ++i )
	{			
		const PrimitiveVariable &primVar = i->second;
		
		TypedDataSize func;
		size_t size = despatchTypedData< TypedDataSize >( primVar.data, func );
		
		if ( size == variableSize( primVar.interpolation ) )
		{		
			names.push_back( i->first );
		}
	}
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

void ImagePrimitive::render(RendererPtr renderer)
{
	assert( renderer );
	
	renderer->image(m_dataWindow, m_displayWindow, variables);
}

//
// handling for serialization
//
void ImagePrimitive::copyFrom(ConstObjectPtr rhs, IECore::Object::CopyContext *context )
{
	assert( rhs );
	assert( context );

	Primitive::copyFrom(rhs, context);
	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs.get());

	m_displayWindow = p_rhs->getDisplayWindow();
	m_dataWindow = p_rhs->getDataWindow();
}

void ImagePrimitive::save(IECore::Object::SaveContext *context) const
{
	assert( context );

	Primitive::save(context);
	IndexedIOInterfacePtr container = context->container(staticTypeName(), m_ioVersion);

	container->write("displayWindowMinX", m_displayWindow.min.x);
	container->write("displayWindowMinY", m_displayWindow.min.y);
	container->write("displayWindowMaxX", m_displayWindow.max.x);
	container->write("displayWindowMaxY", m_displayWindow.max.y);
	
	container->write("dataWindowMinX", m_dataWindow.min.x);
	container->write("dataWindowMinY", m_dataWindow.min.y);
	container->write("dataWindowMaxX", m_dataWindow.max.x);
	container->write("dataWindowMaxY", m_dataWindow.max.y);
}

void ImagePrimitive::load(IECore::Object::LoadContextPtr context)
{
	assert( context );

	Primitive::load(context);
	unsigned int v = m_ioVersion;

	IndexedIOInterfacePtr container = context->container(staticTypeName(), v);

	container->read("displayWindowMinX", m_displayWindow.min.x);
	container->read("displayWindowMinY", m_displayWindow.min.y);
	container->read("displayWindowMaxX", m_displayWindow.max.x);
	container->read("displayWindowMaxY", m_displayWindow.max.y);
	
	if ( v < 1 )
	{
		m_dataWindow = m_displayWindow;
	}
	else
	{
		container->read("dataWindowMinX", m_dataWindow.min.x);
		container->read("dataWindowMinY", m_dataWindow.min.y);
		container->read("dataWindowMaxX", m_dataWindow.max.x);
		container->read("dataWindowMaxY", m_dataWindow.max.y);
	}
}

bool ImagePrimitive::isEqualTo(ConstObjectPtr rhs) const
{
	assert( rhs );

	if (!Primitive::isEqualTo(rhs))
	{
		return false;
	}

	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs.get());

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
