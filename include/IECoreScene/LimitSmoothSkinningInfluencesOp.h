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

#ifndef IECORESCENE_LIMITSMOOTHSKINNINGINFLUENCESOP_H
#define IECORESCENE_LIMITSMOOTHSKINNINGINFLUENCESOP_H

#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/FrameListParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

namespace IECoreScene
{

/// The LimitSmoothSkinningInfluencesOp zeros the weight values of SmoothSkinningData for certain influences.
/// This op has several modes of operation. It can zero influences below a minimum weight value,
/// it can impose a maximum number of influences per point, or it can zero specific influences for all points.
/// There is a parameter to control whether the result should be compressed or left as is. Locks can be applied
/// in WeightLimit and MaxInfluences mode to control which influences are affected.
/// \ingroup skinningGroup
class IECORESCENE_API LimitSmoothSkinningInfluencesOp : public IECore::ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LimitSmoothSkinningInfluencesOp, LimitSmoothSkinningInfluencesOpTypeId, IECore::ModifyOp );

		LimitSmoothSkinningInfluencesOp();
		~LimitSmoothSkinningInfluencesOp() override;

		enum Mode
		{
			WeightLimit = 0,
			MaxInfluences,
			Indexed,
		};

	protected :

		void modify( IECore::Object *object, const IECore::CompoundObject *operands ) override;

	private :

		IECore::IntParameterPtr m_modeParameter;
		IECore::BoolParameterPtr m_compressionParameter;
		IECore::FloatParameterPtr m_minWeightParameter;
		IECore::IntParameterPtr m_maxInfluencesParameter;
		IECore::FrameListParameterPtr m_influenceIndicesParameter;
		IECore::BoolParameterPtr m_useLocksParameter;
		IECore::BoolVectorParameterPtr m_influenceLocksParameter;
};

IE_CORE_DECLAREPTR( LimitSmoothSkinningInfluencesOp );

} // namespace IECoreScene

#endif // IECORESCENE_LIMITSMOOTHSKINNINGINFLUENCESOP_H
