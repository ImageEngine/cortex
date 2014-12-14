//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IE_CORE_POINTSMOOTHSKINNINGOP_H
#define IE_CORE_POINTSMOOTHSKINNINGOP_H

#include <vector>

#include "IECore/Export.h"
#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// A PointPrimitiveOp to deform points and normals based on a pose defined by a matrixVector and SmoothSkinningData.
/// This Op can be used to generate smooth deformation effects based on influence objects like joint hierachies.
///
/// The input Primitive should have a V3fVectorData primitive variable for positions as specified by the positionVar
/// parameter (which defaults to "P"). Optionally one can also deform a normal V3fVectorData primitive variable (which
/// defaults to "N"). These variables must have the same number of elements and must match the number of points in the
/// SmoothSkinningData.
/// \ingroup geometryProcessingGroup
/// \ingroup skinningGroup
class IECORE_API PointSmoothSkinningOp : public ModifyOp
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( PointSmoothSkinningOp, ModifyOp );


		// defines what algorithm to use when calculating the deformation
		typedef enum
		{
			Linear = 0,
			// todo: DualQuaternion = 1
			// todo: LinearDualQuaternionMix = 2
		} Blend;

		PointSmoothSkinningOp();
		virtual ~PointSmoothSkinningOp();

		/// parameter for the 'position' primvar to be deformed in the input primitive,
		/// defaults to "P"
		StringParameter * positionVarParameter();
        const StringParameter * positionVarParameter() const;

		/// parameter for the 'normal' primvar to be deformed in the input primitive,
		/// defaults to "N"
        StringParameter * normalVarParameter();
        const StringParameter * normalVarParameter() const;

		/// parameter for the smooth skinning data used in the deformation
		SmoothSkinningDataParameter * smoothSkinningDataParameter();
		const SmoothSkinningDataParameter * smoothSkinningDataParameter() const;

		/// parameter for the pose that deforms the points, this array of matrices is assumed to be in
		/// world space and match the length in the SmoothSkinningData parameter's influencePose
		M44fVectorParameter * deformationPoseParameter();
		const M44fVectorParameter * deformationPoseParameter() const;

		/// parameter to control if the normals are deformed by the op
		BoolParameter * deformNormalsParameter();
		const BoolParameter * deformNormalsParameter() const;

		/// parameter that controls which algorithm is used for the deformation of the mesh
		IntParameter * blendParameter();
		const IntParameter * blendParameter() const;

		/// parameter to map each input vertex index to an index in the smooth skinning data
		IntVectorParameter * refIndicesParameter();
		const IntVectorParameter * refIndicesParameter() const;

	protected:

        virtual void modify( Object *object, const CompoundObject * operands );

	private:

        StringParameterPtr m_positionVarParameter;
        StringParameterPtr m_normalVarParameter;
		SmoothSkinningDataParameterPtr m_smoothSkinningDataParameter;
		IntParameterPtr m_blendParameter;
		BoolParameterPtr m_deformNormalsParameter;
		M44fVectorParameterPtr m_deformationPoseParameter;
		IntVectorParameterPtr m_refIndicesParameter;

		ConstSmoothSkinningDataPtr m_prevSmoothSkinningData;
		
		struct DeformPositions;
		struct DeformNormals;
};

IE_CORE_DECLAREPTR( PointSmoothSkinningOp );


} // namespace IECore

#endif // IE_CORE_POINTSMOOTHSKINNINGOP_H
