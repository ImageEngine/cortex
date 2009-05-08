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

#ifndef IE_CORE_IMAGECROPOP_H
#define IE_CORE_IMAGECROPOP_H

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// The ImageCropOp performs cropping over ImagePrimitive objects.
/// The operation results on an ImagePrimitive with displayWindow equal to the given crop box.
/// If matchDataWindow if On then the dataWindow will match displayWindow. Otherwise it will be intersected against the given crop box.
class ImageCropOp : public ImagePrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( ImageCropOp, ImagePrimitiveOp );

		ImageCropOp();
		virtual ~ImageCropOp();

		Box2iParameterPtr cropBoxParameter();
		ConstBox2iParameterPtr cropBoxParameter() const;

		BoolParameterPtr matchDataWindowParameter();
		ConstBoolParameterPtr matchDataWindowParameter() const;

		BoolParameterPtr resetOriginParameter();
		ConstBoolParameterPtr resetOriginParameter() const;

		BoolParameterPtr intersectParameter();
		ConstBoolParameterPtr intersectParameter() const;

	protected :

		virtual void modifyTypedPrimitive( ImagePrimitivePtr image, ConstCompoundObjectPtr operands );

	private :

		struct ImageCropFn;

		Box2iParameterPtr m_cropBoxParameter;
		BoolParameterPtr m_matchDataWindowParameter;
		BoolParameterPtr m_resetOriginParameter;
		BoolParameterPtr m_intersectParameter;
};

IE_CORE_DECLAREPTR( ImageCropOp );

} // namespace IECore

#endif // IE_CORE_IMAGECROPOP_H
