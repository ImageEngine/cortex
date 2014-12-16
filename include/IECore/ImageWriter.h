//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Export.h"
#include "IECore/Writer.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// Abstract base class for serializing images
/// \ingroup ioGroup
class IECORE_API ImageWriter : public Writer
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( ImageWriter, Writer )

		/// Checks that object is an ImagePrimitive instance
		static bool canWrite( ConstObjectPtr object, const std::string &fileName );

		/// The parameter specifying the channels to write.
		StringVectorParameter * channelNamesParameter();
		const StringVectorParameter * channelNamesParameter() const;
		/// The parameter specifying the colorspace that the given image will be when stored in the file.
		/// If autoDetect is chosen than it will use the colorspace returned by destinationColorSpace().
		/// The input image is assumed to be in linear colorspace.
		StringParameter * colorspaceParameter();
		const StringParameter * colorspaceParameter() const;
		/// The parameter specifying if the image channels should be 
		/// written as is to the file, keeping the same data type if possible. 
		/// If True, then color space settings will not take effect.
		/// Otherwise the 
		BoolParameter * rawChannelsParameter();
		const BoolParameter * rawChannelsParameter() const;

		/// Convenience function to access the channels specified in parameters
		void imageChannels( std::vector<std::string> &names ) const;

		/// Returns the name of default colorspace in which the Writer expects to receive images.
		/// The base class is responsible for making sure it will happen.
		virtual std::string destinationColorSpace() const = 0;

	protected:

		ImageWriter( const std::string &description );

		/// Return the image object to write
		const ImagePrimitive *getImage() const;

		/// Write the image. Subclasses implement this method.
		virtual void writeImage( const std::vector<std::string> &names,
		                         const ImagePrimitive * image,
		                         const Imath::Box2i &dataWindow	) const = 0;

	private :

		/// Implementation of Writer::doWrite(). Calls through to writeImage()
		virtual void doWrite( const CompoundObject *operands );

		StringVectorParameterPtr m_channelsParameter;
		BoolParameterPtr m_rawChannelsParameter;
		StringParameterPtr m_colorspaceParameter;

};

IE_CORE_DECLAREPTR(ImageWriter);

} // namespace IECore

#endif // IE_CORE_IMAGEWRITER_H
