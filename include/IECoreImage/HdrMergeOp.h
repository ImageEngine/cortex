//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_HDRMERGEOP_H
#define IECOREIMAGE_HDRMERGEOP_H

#include "IECore/Op.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedObjectParameter.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

/// The HdrMergeOp merges a set of images with different exposures into a single HDR image.
/// \todo Take in consideration Alpha channel from input images.
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API HdrMergeOp : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( HdrMergeOp, HdrMergeOpTypeId, IECore::Op );

		HdrMergeOp();
		virtual ~HdrMergeOp();

		/// The Parameter for the vector of input images.
		IECore::ObjectVectorParameter *inputImagesParameter();
		const IECore::ObjectVectorParameter *inputImagesParameter() const;

		IECore::FloatParameter *exposureStepParameter();
		const IECore::FloatParameter *exposureStepParameter() const;

		IECore::FloatParameter *exposureAdjustmentParameter();
		const IECore::FloatParameter *exposureAdjustmentParameter() const;

		IECore::Box2fParameter *windowingParameter();
		const IECore::Box2fParameter *windowingParameter() const;

	protected :

		virtual IECore::ObjectPtr doOperation( const IECore::CompoundObject *operands );

	private :

		IECore::ObjectVectorParameterPtr m_inputImagesParameter;
		IECore::FloatParameterPtr m_exposureStepParameter;
		IECore::FloatParameterPtr m_exposureAdjustmentParameter;
		IECore::Box2fParameterPtr m_windowingParameter;

};

IE_CORE_DECLAREPTR( HdrMergeOp );

} // namespace IECoreImage

#endif // IECOREIMAGE_HDRMERGEOP_H
