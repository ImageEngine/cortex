//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_IMAGEPRIMITIVE_H
#define IECORE_IMAGEPRIMITIVE_H

#include <string>
#include <vector>

#include "IECore/Primitive.h"
#include "IECore/Data.h"
#include "IECore/VectorTypedData.h"

#include "CompoundData.h"

namespace IECore
{

/// ImagePrimitive represents an axis-aligned collection of raster data in the form of channels.
class ImagePrimitive : public Primitive
{	  

	/// A channel may contain data of half (16-bit float), unsigned int (32-bit integer),
	/// or float (32-bit float) type.  Typically, data is arranged into three colour channels
	/// named "R" (red), "G" (green), and "B" (blue), however, arbitrary channel names are allowed,
	/// and their interpretation is left to the application.
	
	public:
	  
		IE_CORE_DECLAREOBJECT( ImagePrimitive, Primitive );
	
		/// construct an ImagePrimitive with no area consumed
		ImagePrimitive();
	
		/// construct an ImagePrimitive with the given data and display window dimensions
		ImagePrimitive(Imath::Box2i datawindow, Imath::Box2i displaywindow);

		/// return the image data window
		const Imath::Box2i & getDataWindow() const;
	
		/// get the data window
		void setDataWindow(const Imath::Box2i & dw);
	
		/// return the image display window
		const Imath::Box2i & getDisplayWindow() const;
	
		/// set the display window
		void setDisplayWindow(const Imath::Box2i & dw);
	
		/// give the data window x origin
		const int x() const;
	
		/// compute the data window y origin
		const int y() const;
	
		/// compute the data window width
		const int width() const;
	
		/// compute the data window height
		const int height() const;
	
		/// return the data window area
		const int area() const;
	
		/// returns 2-d image size for Vertex, Varying, and FaceVarying Interpolation, 1 otherwise
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation );
	
		/// render the image
		virtual void render(RendererPtr renderer);
	
		/// place the channel names for this image into the given vector
		void channelNames(std::vector<std::string> & names) const;
	
		/// create a channel
		template<typename T>
		boost::intrusive_ptr<TypedData<std::vector<T> > > createChannel(std::string name);
		
	private:
	
		/// the full parameters for image position and dimension
		Imath::Box2i m_dataWindow;
	
		/// a sub-rectangle of the full image
		Imath::Box2i m_displayWindow;
	
		static const unsigned int m_ioVersion;

};  
	
IE_CORE_DECLAREPTR(ImagePrimitive);

}

#include "IECore/ImagePrimitive.inl"

#endif
