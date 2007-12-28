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

#ifndef IE_CORE_IMAGEREADER_H
#define IE_CORE_IMAGEREADER_H

#include "IECore/Reader.h"
#include "IECore/TypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );
	
/// The ImageReader class defines an abstract base class for reading sampled images
/// \todo Define and support a parameter for conversion of image data type during loading
class ImageReader : public Reader
{

	/// ImageReader's main purpose is to define a standard set of parameters
	/// which all concrete ImageReader implementations obey.  It also defines some pure virtual functions
	/// which allow interface implementors to focus on image-specific code for loading channels.
	
	public:

		IE_CORE_DECLARERUNTIMETYPED( ImageReader, Reader );

		ImageReader( const std::string name, const std::string description );
		/// place the user-requested channels into the given vector
		void imageChannels(std::vector<std::string> & names);
	
		/// replace the channel names into the given vector
		virtual void channelNames(std::vector<std::string> & names) = 0;

		/// give the user-requested data window
		Imath::Box2i dataWindow() const;
	
		/// give the user-requested display window
		Imath::Box2i displayWindow() const;

		/// return true iff all pixels from the data window reported by the file are present
		virtual bool isComplete() const { return true; };
	
	protected:
		
		/// a class implementing this method returns an ImagePrimitive.  this function
		/// is left open to derivation, but it is suggested to interface implementors that
		/// the protected methods be coded and to let this base class handle the logic
		/// for dealing with configuration of subimage, particular channel loading, etc.
		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );
	
		/// read the channel identified by the given name.  
		virtual void readChannel(std::string name, ImagePrimitivePtr image, const Imath::Box2i &dw) = 0;
	
		Box2iParameterPtr m_dataWindowParameter;
		Box2iParameterPtr m_displayWindowParameter;
		StringVectorParameterPtr m_channelNamesParameter;
	
};
	
IE_CORE_DECLAREPTR(ImageReader);
	
} // namespace IECore

#endif // IE_CORE_IMAGEREADER_H
