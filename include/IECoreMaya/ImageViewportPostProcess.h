//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_IMAGEVIEWPORTPOSTPROCESS_H
#define IE_COREMAYA_IMAGEVIEWPORTPOSTPROCESS_H

#include <string>

#include "maya/MString.h"
#include "maya/MImage.h"

#include "IECore/ImagePrimitive.h"

#include "IECoreMaya/ViewportPostProcess.h"

namespace IECoreMaya
{

/// An abstract ViewportPostProcess suitable for subclassing in Python which operates on ImagePrimitive objects. This is much
/// slower than ViewportPostProcess due to the MImage<->ImagePrimitive conversions which need to occur.
class ImageViewportPostProcess : public ViewportPostProcess
{
	public:

		IE_CORE_DECLAREMEMBERPTR( ImageViewportPostProcess )

		ImageViewportPostProcess();
		virtual ~ImageViewportPostProcess();

	protected:

		/// Performs in-place modification of the given ImagePrimitive. Derived classes need to implement this.
		virtual void postRender( const std::string &panelName, IECore::ImagePrimitivePtr image ) = 0;

	private :

		/// Performs the appropriate MImage<->ImagePrimitive conversions around call through to abstract ImageViewportPostProcess::postRender()
		virtual void postRender( const std::string &panelName, MImage &image );
};


IE_CORE_DECLAREPTR( ImageViewportPostProcess );

} // namespace IECoreMaya

#endif // IE_COREMAYA_IMAGEVIEWPORTPOSTPROCESS_H
