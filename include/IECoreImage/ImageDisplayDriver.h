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

//! \file ImageDisplayDriver.h
/// Defines the ImageDisplayDriver class.

#ifndef IECOREIMAGE_IMAGEDISPLAYDRIVER
#define IECOREIMAGE_IMAGEDISPLAYDRIVER

#include "IECore/DisplayDriver.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

/// Display driver that creates an ImagePrimitive object held
/// in memory.
/// \ingroup renderingGroup
class IECOREIMAGE_API ImageDisplayDriver : public IECore::DisplayDriver
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImageDisplayDriver, ImageDisplayDriverTypeId, IECore::DisplayDriver );

		/// Initializes the internal ImagePrimitive.
		/// The image's blindData will keep the values given on the parameters CompoundData.
		ImageDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters );
		virtual ~ImageDisplayDriver();

		virtual bool scanLineOrderOnly() const;
		virtual bool acceptsRepeatedData() const;
		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize );
		virtual void imageClose();

		/// Access to the image being created. This should always be valid for reading, even
		/// before imageClose() has been called.
		ConstImagePrimitivePtr image() const;

		//! @name Image pool
		/// It can be useful to store the images created by ImageDisplayDrivers for
		/// later retrieval. Images can be stored by passing a StringData
		/// "handle" parameter to the constructor or to the
		/// DisplayDriver::create() method. The resulting image will then be
		/// stored and can be retrieved using the methods below.
		///////////////////////////////////////////////////////////////////////
		//@{
		/// Returns the image stored with the specified handle, or 0 if no
		/// such image exists.
		static ConstImagePrimitivePtr storedImage( const std::string &handle );
		/// Removes the image stored with the specified handle from the pool. Returns
		/// the image, or 0 if no such image existed.
		static ConstImagePrimitivePtr removeStoredImage( const std::string &handle );
		//@}

	private:

		static const DisplayDriverDescription<ImageDisplayDriver> g_description;

		ImagePrimitivePtr m_image;

};

IE_CORE_DECLAREPTR( ImageDisplayDriver )

} // namespace IECore

#endif // IE_CORE_IMAGEDISPLAYDRIVER
