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

#ifndef IECORESCENE_POINTSMOOTHSKINNINGOP_H
#define IECORESCENE_POINTSMOOTHSKINNINGOP_H

#include <vector>

#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "IECoreScene/Export.h"
#include "IECoreScene/TypedPrimitiveParameter.h"
#include "IECoreScene/TypedObjectParameter.h"
#include "IECoreScene/TypeIds.h"

namespace IECoreScene
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
class IECORESCENE_API PointSmoothSkinningOp : public IECore::ModifyOp
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PointSmoothSkinningOp, PointSmoothSkinningOpTypeId, IECore::ModifyOp );


		// defines what algorithm to use when calculating the deformation
		typedef enum
		{
			Linear = 0,
			// todo: DualQuaternion = 1
			// todo: LinearDualQuaternionMix = 2
		} Blend;

		PointSmoothSkinningOp();
		~PointSmoothSkinningOp() override;

		/// parameter for the 'position' primvar to be deformed in the input primitive,
		/// defaults to "P"
		IECore::StringParameter * positionVarParameter();
        const IECore::StringParameter * positionVarParameter() const;

		/// parameter for the 'normal' primvar to be deformed in the input primitive,
		/// defaults to "N"
        IECore::StringParameter * normalVarParameter();
        const IECore::StringParameter * normalVarParameter() const;

		/// parameter for the smooth skinning data used in the deformation
		SmoothSkinningDataParameter * smoothSkinningDataParameter();
		const SmoothSkinningDataParameter * smoothSkinningDataParameter() const;

		/// parameter for the pose that deforms the points, this array of matrices is assumed to be in
		/// world space and match the length in the SmoothSkinningData parameter's influencePose
		IECore::M44fVectorParameter * deformationPoseParameter();
		const IECore::M44fVectorParameter * deformationPoseParameter() const;

		/// parameter to control if the normals are deformed by the op
		IECore::BoolParameter * deformNormalsParameter();
		const IECore::BoolParameter * deformNormalsParameter() const;

		/// parameter that controls which algorithm is used for the deformation of the mesh
		IECore::IntParameter * blendParameter();
		const IECore::IntParameter * blendParameter() const;

		/// parameter to map each input vertex index to an index in the smooth skinning data
		IECore::IntVectorParameter * refIndicesParameter();
		const IECore::IntVectorParameter * refIndicesParameter() const;

	protected:

        void modify( IECore::Object *object, const IECore::CompoundObject * operands ) override;

	private:

        IECore::StringParameterPtr m_positionVarParameter;
        IECore::StringParameterPtr m_normalVarParameter;
		SmoothSkinningDataParameterPtr m_smoothSkinningDataParameter;
		IECore::IntParameterPtr m_blendParameter;
		IECore::BoolParameterPtr m_deformNormalsParameter;
		IECore::M44fVectorParameterPtr m_deformationPoseParameter;
		IECore::IntVectorParameterPtr m_refIndicesParameter;

		ConstSmoothSkinningDataPtr m_prevSmoothSkinningData;

		struct DeformPositions;
		struct DeformNormals;
};

IE_CORE_DECLAREPTR( PointSmoothSkinningOp );


} // namespace IECoreScene

#endif // IECORESCENE_POINTSMOOTHSKINNINGOP_H
