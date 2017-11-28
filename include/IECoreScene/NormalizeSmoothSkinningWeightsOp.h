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

#ifndef IECORESCENE_NORMALIZESMOOTHSKINNINGWEIGHTSOP_H
#define IECORESCENE_NORMALIZESMOOTHSKINNINGWEIGHTSOP_H

#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

namespace IECoreScene
{

/// The NormalizeSmoothSkinningWeightsOp normalizes SmoothSkinningData weights between the existing influences for each point
/// Locks can be applied to the influences and the unlocked weights will be normalized accordingly.
/// \ingroup skinningGroup
class IECORESCENE_API NormalizeSmoothSkinningWeightsOp : public IECore::ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( NormalizeSmoothSkinningWeightsOp, NormalizeSmoothSkinningWeightsOpTypeId, IECore::ModifyOp );

		NormalizeSmoothSkinningWeightsOp();
		~NormalizeSmoothSkinningWeightsOp() override;

	protected :

		void modify( IECore::Object *object, const IECore::CompoundObject *operands ) override;

	private :

		IECore::BoolParameterPtr m_useLocksParameter;
		IECore::BoolVectorParameterPtr m_influenceLocksParameter;
};

IE_CORE_DECLAREPTR( NormalizeSmoothSkinningWeightsOp );

} // namespace IECoreScene

#endif // IECORESCENE_NORMALIZESMOOTHSKINNINGWEIGHTSOP_H
