//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IMAGECOMPOSITEOP_H
#define IE_CORE_IMAGECOMPOSITEOP_H

#include "OpenEXR/ImathVec.h"

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/ImagePrimitive.h"

namespace IECore
{

class ImageCompositeOp : public ImagePrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( ImageCompositeOp, ImagePrimitiveOp );

		typedef enum
		{
			Over,
			Max,
			Min,
			Multiply,
		} Operation;

		typedef enum
		{
			Premultiplied,
			Unpremultiplied,
		} InputMode;

		ImageCompositeOp();
		virtual ~ImageCompositeOp();

		/// \todo Create a new base class containing a channelNames parameter, shared with ChannelOp?
		StringVectorParameterPtr channelNamesParameter();
		ConstStringVectorParameterPtr channelNamesParameter() const;

		StringParameterPtr alphaChannelNameParameter();
		ConstStringParameterPtr alphaChannelNameParameter() const;

		ImagePrimitiveParameterPtr imageAParameter();
		ConstImagePrimitiveParameterPtr imageAParameter() const;

		IntParameterPtr operationParameter();
		ConstIntParameterPtr operationParameter() const;

		IntParameterPtr inputModeParameter();
		ConstIntParameterPtr inputModeParameter() const;

	protected :

		typedef enum
		{
			Union,
			Intersection
		} DataWindowResult;

		typedef float (*CompositeFn)( float, float, float, float );

		void composite( CompositeFn fn, DataWindowResult dwr, ImagePrimitivePtr imageB, ConstCompoundObjectPtr operands );

		virtual void modifyTypedPrimitive( ImagePrimitivePtr imageB, ConstCompoundObjectPtr operands );

		StringVectorParameterPtr m_channelNamesParameter;
		StringParameterPtr m_alphaChannelNameParameter;
		ImagePrimitiveParameterPtr m_imageAParameter;
		IntParameterPtr m_operationParameter;
		IntParameterPtr m_inputModeParameter;

	private :
		struct ChannelConverter;

		FloatVectorDataPtr getChannelData( ImagePrimitivePtr image, const std::string &channelName, bool mustExist = true );
		float readChannelData( ConstImagePrimitivePtr image, ConstFloatVectorDataPtr data, const Imath::V2i &pixel );

};

IE_CORE_DECLAREPTR( ImageCompositeOp );

} // namespace IECore

#endif // IE_CORE_IMAGECOMPOSITEOP_H
