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

#ifndef IE_CORE_IMAGEWRITER_H
#define IE_CORE_IMAGEWRITER_H

#include <vector>
#include <string>

#include "IECore/Writer.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// Abstract base class for serializing images
class ImageWriter : public Writer
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( ImageWriter, Writer )

		/// Checks that object is an ImagePrimitive instance
		static bool canWrite( ConstObjectPtr object, const std::string &fileName );

		/// Convenience function to access the channels specified in parameters
		void imageChannels( std::vector<std::string> &names ) const;
		
		/// Returns the name of default colorspace in which the Writer expects to receive images.
		virtual std::string destinationColorSpace() const = 0;

	protected:
	
		ImageWriter( const std::string &name, const std::string &description );	

		/// Return the image object to write
		ConstImagePrimitivePtr getImage() const;

		/// Write the image. Subclasses implement this method.
		virtual void writeImage( const std::vector<std::string> &names,
		                         ConstImagePrimitivePtr image,
		                         const Imath::Box2i &dataWindow	) const = 0;

	private :
		
		/// Implementation of Writer::doWrite(). Calls through to writeImage()
		virtual void doWrite();

		StringVectorParameterPtr m_channelsParameter;

};

IE_CORE_DECLAREPTR(ImageWriter);

} // namespace IECore

#endif // IE_CORE_IMAGEWRITER_H
