//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_IMAGETHINNER_H
#define IECOREIMAGE_IMAGETHINNER_H

#include "IECore/NumericParameter.h"

#include "IECoreImage/ChannelOp.h"
#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

/// The ImageThinner class performs thinning of binary images. The code is
/// derived from the example code accompanying the following article :
///
/// "Efficient Binary Image Thinning using Neighborhood Maps"
/// by Joseph M. Cychosz, 3ksnn64@ecn.purdue.edu
/// in "Graphics Gems IV", Academic Press, 1994
///
/// The original code can be found here :
///
/// http://tog.acm.org/resources/GraphicsGems/gemsiv/thin_image.c
///
/// And this page indicates that it can be used without restriction :
///
/// http://tog.acm.org/resources/GraphicsGems/
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API ImageThinner : public ChannelOp
{
	public:

		ImageThinner();
		virtual ~ImageThinner();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImageThinner, ImageThinnerTypeId, ChannelOp );

		/// The transform considers images to be binary, whereby each pixel is
		/// either a member of the foreground or background. Pixels with a value
		/// below the threshold are considered to be background, and those above
		/// the threshold are considered to be foreground.
		IECore::FloatParameter * thresholdParameter();
		const IECore::FloatParameter * thresholdParameter() const;

	protected :

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

};

IE_CORE_DECLAREPTR( ImageThinner );

} // namespace IECoreImage

#endif // IECOREIMAGE_IMAGETHINNER_H

