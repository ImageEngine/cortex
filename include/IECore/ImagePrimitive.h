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
/// \todo Implement the bound() method.
/// \bug The bound() method is not implemented.
/// \todo Document the meaning of data and display windows and the use of primvars as channels.
/// \todo Establish whether or not image origin and orientation are appropriate for common uses, document it either way
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
		/// \todo Change parameters to const references
		ImagePrimitive( Imath::Box2i dataWindow, Imath::Box2i displayWindow );
		
		/// Returns the display window of the image on the XY-plane.
		virtual Imath::Box3f bound() const;

		/// Returns the data window.
		const Imath::Box2i &getDataWindow() const;
	
		/// Sets the data window - note that this doesn't modify the contents of primitive variables (channels)
		/// at all - it is the callers responsibilty to keep any data valid.
		void setDataWindow( const Imath::Box2i &dw );
	
		/// Returns the display window.
		const Imath::Box2i &getDisplayWindow() const;
	
		/// Sets the display window.
		/// \todo Throw on empty windows
		void setDisplayWindow( const Imath::Box2i &dw );
	
		/// give the data window x origin
		/// \deprecated It's unclear whether this should reference the data window or display window.
		/// Just use those windows directly instead.
		const int x() const;
	
		/// compute the data window y origin
		/// \deprecated It's unclear whether this should reference the data window or display window.
		/// Just use those windows directly instead.
		const int y() const;
	
		/// compute the data window width
		/// \deprecated It's unclear whether this should reference the data window or display window.
		/// Just use those windows directly instead.
		const int width() const;
	
		/// compute the data window height
		/// \deprecated It's unclear whether this should reference the data window or display window.
		/// Just use those windows directly instead.
		const int height() const;
	
		/// return the data window area
		/// \deprecated It's unclear whether this should reference the data window or display window.
		/// Just use those windows directly instead.
		const int area() const;
	
		/// Returns 2-d image size for Vertex, Varying, and FaceVarying Interpolation, otherwise 1.
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation );
	
		/// Renders the image.
		virtual void render(RendererPtr renderer);
	
		/// Places the channel names for this image into the given vector
		/// \bug this just copies the primitive variable names - it should also check that
		/// the number of elements and interpolation makes the primvars suitable for
		/// use as channels.
		void channelNames(std::vector<std::string> & names) const;
	
		/// Convenience function to create a channel - this simply creates and adds a PrimitiveVariable of the appropriate
		/// size and returns a pointer to the data within it.
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
