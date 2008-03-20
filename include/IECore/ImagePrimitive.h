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

/// ImagePrimitive represents a 2D bitmap in the form of individual channels, which are stored as primitive variables.
/// A channel may contain data of half (16-bit float), unsigned int (32-bit integer),
/// or float (32-bit float) type. The interpretation of these channels broadly matches the EXR
/// specification - the following channel names have the specified special meanings, but arbitrary channel names
/// are permitted and their interpretation is left to the application :
///
///    "R"                      Red<br>
///    "G"                      Green<br>
///    "B"                      Blue<br>
///    "A"                      Alpha<br>
///    "Y"                      Luminance for greyscale images<br>
///
/// Within the channel's data buffers themselves the pixel values are stored in row major order,
/// that is to say pixels which are adjacent in X (which runs along the "width" of the image) are also
/// adjacent in memory.
///
/// An ImagePrimitive defines both a "display window" and a "data window", both of which are specified in
/// pixel space. The display window defines the overall size of the image, whereas the data window defines
/// the specific region for which we hold data. In most cases the data window will be equal to, or a sub-region
/// of the display window. Outside of the data window the values of the ImagePrimitive's channels are defined
/// to be zero (i.e. black/transparent). This means that the number of data elements stored in each
/// channel should equal to the area of the data window.
///
/// In object-space, the ImagePrimitive is represented as a unit plane centered on the origin, with scale (width, height) in axes (X, Y).
/// The normal is pointing down the negative Z-axis.
///
/// Pixel-space runs from the display window origin in the top-left corner, to the display window's maximum in
/// the bottom-right corner. Pixels of ascending X coordinate therefore run left-right, and pixels of ascending
/// Y coordinate run top-bottom.
///
/// UV-space has the same orientation as pixel-space, and is defined to be (0,0) at the origin of the display window
/// and (1,1) at the maximum of the display window.
class ImagePrimitive : public Primitive
{

	public:

		IE_CORE_DECLAREOBJECT( ImagePrimitive, Primitive );


		/// construct an ImagePrimitive with no area consumed		
		/// \deprecated There is no default display window which makes sense for an image primitive. We only need this so that we can
		/// created an object during file reading
		/// \todo Try and make this constructor protected so that only the Object loading can call it.

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

		void setDataWindow( const Imath::Box2i &dataWindow );

		/// Returns the display window.
		const Imath::Box2i &getDisplayWindow() const;

		/// Sets the display window. Throws if an empty window is passed.
		void setDisplayWindow( const Imath::Box2i &displayWindow );

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


		virtual void render( RendererPtr renderer );


		/// Places the channel names for this image into the given vector
		/// \bug this just copies the primitive variable names - it should also check that
		/// the number of elements and interpolation makes the primvars suitable for
		/// use as channels.
		void channelNames( std::vector<std::string> &names ) const;

		/// Convenience function to create a channel - this simply creates and adds a PrimitiveVariable of the appropriate
		/// size and returns a pointer to the data within it. The data is not initialized.
		/// \todo Use typename TypedData<std::vector<T> >::Ptr as return type
		/// \todo Make channel name a const reference
		template<typename T>
		boost::intrusive_ptr<TypedData<std::vector<T> > > createChannel( std::string name );

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
