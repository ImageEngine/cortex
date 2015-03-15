//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DEEPIMAGEREADER_H
#define IECORE_DEEPIMAGEREADER_H

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/Export.h"
#include "IECore/DeepPixel.h"
#include "IECore/Reader.h"

namespace IECore
{

/// The DeepImageReader class defines an abstract base class for reading deep images.
/// DeepImageReader defines some pure virtual functions which allow interface implementors
/// to focus on format-specific code for loading pixels. As with DeepPixel, DeepImageReader
/// only supports float channels.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class IECORE_API DeepImageReader : public Reader
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( DeepImageReader, Reader );

		DeepImageReader( const std::string &description );

		virtual CompoundObjectPtr readHeader();

		/// Fills the passed vector with the names of all channels within the file.
		virtual void channelNames( std::vector<std::string> &names ) = 0;
		
		/// Returns true if the file is complete. Implementations of this function should
		/// be quick - it's intended as a cheaper alternative to loading the whole file to
		/// determine completeness.
		virtual bool isComplete() = 0;
		
		/// Returns the dataWindow contained in the file.
		virtual Imath::Box2i dataWindow() = 0;
		/// Returns the displayWindow contained in the file.
		virtual Imath::Box2i displayWindow() = 0;
		
		/// Returns the world to camera space transformation matrix contained in the file.
		virtual Imath::M44f worldToCameraMatrix() = 0;
		/// Returns the world to screen space projection matrix contained in the file.
		virtual Imath::M44f worldToNDCMatrix() = 0;
		
		/// Reads the specified pixel. Note that regardless of image format, x and y should
		/// be specified as if the origin is in the upper left corner of the displayWindow.
		/// It is up to the derived classes to account for that fact if necessary.
		DeepPixelPtr readPixel( int x, int y );

	protected :

		/// Returns an ImagePrimitive, having composited all the DeepPixels into flat pixels
		virtual ObjectPtr doOperation( const CompoundObject *operands );

		/// Read the specified pixel. This is called by the public readPixel() method and must
		/// be implemented in all derived classes. It is guaranteed that this function will not
		/// be called with invalid coordinates which are not within the dataWindow in the file.
		/// However, as with the public method, x and y are specified as if the origin is in the
		/// upper left corner of the displayWindow. It is up to the derived classes to account
		/// for that fact if necessary.
		virtual DeepPixelPtr doReadPixel( int x, int y ) = 0;

};

IE_CORE_DECLAREPTR( DeepImageReader );

} // namespace IECore

#endif // IECORE_DEEPIMAGEREADER_H
