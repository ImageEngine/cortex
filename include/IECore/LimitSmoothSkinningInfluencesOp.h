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

#ifndef IECORE_LIMITSMOOTHSKINNINGINFLUENCESOP_H
#define IECORE_LIMITSMOOTHSKINNINGINFLUENCESOP_H

#include "IECore/Export.h"
#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/FrameListParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// The LimitSmoothSkinningInfluencesOp zeros the weight values of SmoothSkinningData for certain influences.
/// This op has several modes of operation. It can zero influences below a minimum weight value,
/// it can impose a maximum number of influences per point, or it can zero specific influences for all points.
/// There is a parameter to control whether the result should be compressed or left as is. Locks can be applied
/// in WeightLimit and MaxInfluences mode to control which influences are affected.
/// \ingroup skinningGroup
class IECORE_API LimitSmoothSkinningInfluencesOp : public ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( LimitSmoothSkinningInfluencesOp, ModifyOp );

		LimitSmoothSkinningInfluencesOp();
		virtual ~LimitSmoothSkinningInfluencesOp();
		
		enum Mode
		{
			WeightLimit = 0,
			MaxInfluences,
			Indexed,
		};
	
	protected :

		virtual void modify( Object *object, const CompoundObject *operands );

	private :

		IntParameterPtr m_modeParameter;
		BoolParameterPtr m_compressionParameter;
		FloatParameterPtr m_minWeightParameter;
		IntParameterPtr m_maxInfluencesParameter;
		FrameListParameterPtr m_influenceIndicesParameter;
		BoolParameterPtr m_useLocksParameter;
		BoolVectorParameterPtr m_influenceLocksParameter;
};

IE_CORE_DECLAREPTR( LimitSmoothSkinningInfluencesOp );

} // namespace IECore

#endif // IECORE_LIMITSMOOTHSKINNINGINFLUENCESOP_H
