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

#ifndef IE_CORE_IMAGEWRITER_H
#define IE_CORE_IMAGEWRITER_H

#include "IECore/Writer.h"
#include "IECore/TypedParameter.h"

#include <vector>
#include <string>

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// abstract base class for serializing images
class ImageWriter : public Writer
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( ImageWriter, Writer )

		ImageWriter( const std::string name, const std::string description );

		/// checks that object is an ImagePrimitive instance
		static bool canWrite( ConstObjectPtr object, const std::string &fileName );

		/// convenience function to access the channels specified in parameters
		void imageChannels( std::vector<std::string> &names );

	protected:

		/// Writes the associated ImagePrimitive. There's no need to reimplement
		/// this in subclasses - implement writeImage() instead.
		virtual void doWrite();

		/// return the image object to write
		ConstImagePrimitivePtr getImage();

		/// write the image. subclasses implement this method.
		/// \todo make "names" const
		virtual void writeImage( std::vector<std::string> &names,
		                         ConstImagePrimitivePtr image, const Imath::Box2i &dataWindow ) = 0;

	private :

		void constructParameters();

		StringVectorParameterPtr m_channelsParameter;

};

IE_CORE_DECLAREPTR(ImageWriter);

} // namespace IECore

#endif // IE_CORE_IMAGEWRITER_H
