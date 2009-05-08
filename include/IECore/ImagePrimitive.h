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
///
/// \todo Define standard depth channel ("Z"? "depth"?)
class ImagePrimitive : public Primitive
{

	public:

		IE_CORE_DECLAREOBJECT( ImagePrimitive, Primitive );

		/// construct an ImagePrimitive with no area consumed
		/// \deprecated There is no default display window which makes sense for an image primitive. We only need this so that we can
		/// created an object during file reading, or for the default values of ImagePrimitiveParameters
		/// \todo Try and make this constructor protected so that only the Object loading can call it.
		ImagePrimitive();

		/// Construct an ImagePrimitive with the given data and display window dimensions. The constructed image will have
		/// no primitive variables.
		ImagePrimitive( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow );

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

		/// Returns 2-d image size for Vertex, Varying, and FaceVarying Interpolation, otherwise 1.
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;

		virtual void render( RendererPtr renderer ) const;

		//! @name Channels
		/// Channels of the image are just primitive variables with the following
		/// constraints :
		///
		///		* Data type must satisfy TypeTraits::IsNumericVectorTypedData - this
		///		  may well be restricted even further in the future, to accept only
		///		  FloatVectorData, UIntVectorData and possibly HalfVectorData. The
		///		  restrictions are currently somewhat relaxed as some ImageReaders
		///		  load the data from file without converting to float - this too may
		///		  change at some point.
		///		* Interpolation type must be Vertex, Varying or FaceVarying - these
		///		  all mean the same thing (that there are the same number of elements
		///		  as there are pixels).
		///		* Data must contain the same number of elements as there are pixels.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		/// Returns true if the PrimitiveVariable is a valid channel for this
		/// image - returns false otherwise. If false is returned and reason is
		/// passed, then a reason for invalidity is places in reason.
		bool channelValid( const PrimitiveVariable &pv, std::string *reason=0 ) const;
		/// As above but passes the name of a PrimitiveVariable.
		bool channelValid( const std::string &name, std::string *reason=0 ) const;
		/// Places the names of all valid channel into the given vector.
		void channelNames( std::vector<std::string> &names ) const;
		/// Returns the data for the named channel, or 0 if it
		/// doesn't exist or is invalid.
		template<typename T>
		typename TypedData<std::vector<T> >::Ptr getChannel( const std::string &name );
		template<typename T>
		typename TypedData<std::vector<T> >::ConstPtr getChannel( const std::string &name ) const;
		/// Convenience function to create a channel - this simply creates and adds a PrimitiveVariable of the appropriate
		/// size and returns a pointer to the data within it. The data is not initialized.
		template<typename T>
		typename TypedData<std::vector<T> >::Ptr createChannel( const std::string &name );
		//@}

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
