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

#include "IECore/ImagePrimitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/Renderer.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

const unsigned int ImagePrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(ImagePrimitive);

ImagePrimitive::ImagePrimitive()
{
}

ImagePrimitive::ImagePrimitive(Box2i datawindow, Box2i displaywindow)
		: m_dataWindow(datawindow), m_displayWindow(displaywindow)
{
}

Box3f ImagePrimitive::bound() const
{
	/// \todo We might need to include any pixel aspect ratio in this bound

	/// We add one here because the displayWindow is measured in pixels, and is inclusive. That is, an image
	/// which has a displayWindow of (0,0)->(0,0) contains exactly one pixel.
	return Box3f(
	               V3f( m_displayWindow.min.x, m_displayWindow.min.y, 0.0 ),
	               V3f( 1.0f + m_displayWindow.max.x, 1.0f + m_displayWindow.max.y, 0.0 )
	       );
}

const Box2i &ImagePrimitive::getDataWindow() const
{
	return m_dataWindow;
}

void ImagePrimitive::setDataWindow(const Box2i &dw)
{
	m_dataWindow = dw;
}

const Box2i &ImagePrimitive::getDisplayWindow() const
{
	return m_displayWindow;
}

void ImagePrimitive::setDisplayWindow(const Box2i &dw)
{
	m_displayWindow = dw;
}

const int ImagePrimitive::width() const
{
	return 1 + m_dataWindow.max.x - m_dataWindow.min.x;
}

const int ImagePrimitive::height() const
{
	return 1 + m_dataWindow.max.y - m_dataWindow.min.y;
}

const int ImagePrimitive::area() const
{
	return width() * height();
}

const int ImagePrimitive::x() const
{
	return m_dataWindow.min.x;
}

const int ImagePrimitive::y() const
{
	return m_dataWindow.min.y;
}

void ImagePrimitive::channelNames(vector<string> &names) const
{
	// copy in the names of channels from the map
	names.clear();
	PrimitiveVariableMap::const_iterator i = variables.begin();
	while (i != variables.end())
	{
		names.push_back(i->first);
		++i;
	}
}

// give the size of the image
size_t ImagePrimitive::variableSize(PrimitiveVariable::Interpolation interpolation)
{
	switch (interpolation)
	{

	case PrimitiveVariable::Vertex:
	case PrimitiveVariable::Varying:
	case PrimitiveVariable::FaceVarying:
		return area();

	default:
		return 1;

	}
}

void ImagePrimitive::render(RendererPtr renderer)
{
	renderer->image(m_dataWindow, m_displayWindow, variables);
}

//
// handling for serialization
//
void ImagePrimitive::copyFrom(ConstObjectPtr rhs, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom(rhs, context);
	const ImagePrimitive *p_rhs = static_cast<const ImagePrimitive *>(rhs.get());

	m_displayWindow = p_rhs->getDisplayWindow();
	m_dataWindow = p_rhs->getDataWindow();
}

void ImagePrimitive::save(IECore::Object::SaveContext *context) const
{
	Primitive::save(context);
	IndexedIOInterfacePtr container = context->container(staticTypeName(), m_ioVersion);

	container->write("displayWindowMinX", m_displayWindow.min.x);
	container->write("displayWindowMinY", m_displayWindow.min.y);
	container->write("displayWindowMaxX", m_displayWindow.max.x);
	container->write("displayWindowMaxY", m_displayWindow.max.y);
}

void ImagePrimitive::load(IECore::Object::LoadContextPtr context)
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;

	IndexedIOInterfacePtr container = context->container(staticTypeName(), v);

	container->read("displayWindowMinX", m_displayWindow.min.x);
	container->read("displayWindowMinY", m_displayWindow.min.y);
	container->read("displayWindowMaxX", m_displayWindow.max.x);
	container->read("displayWindowMaxY", m_displayWindow.max.y);
}

bool ImagePrimitive::isEqualTo(ConstObjectPtr rhs) const
{
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
