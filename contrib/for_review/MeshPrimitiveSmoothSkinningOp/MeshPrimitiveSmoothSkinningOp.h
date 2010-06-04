//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MESHPRIMITIVESMOOTHSKINNINGOP_H
#define IE_CORE_MESHPRIMITIVESMOOTHSKINNINGOP_H

#include <vector>

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// A MeshPrimitiveOp to deform a mesh based on a pose defined by a matrixVector and SmoothSkinningData.
/// This Op can be used to generate smooth deformation based on influence objects like joint hierachies.
class MeshPrimitiveSmoothSkinningOp : public TypedPrimitiveOp<MeshPrimitive>
{
	public:

		// defines what algorithm to use when calculating the deformation
		typedef enum
		{
			Linear = 0,
			// todo: DualQuaternion = 1
			// todo: LinearDualQuaternionMix = 2
		} Blend;

		typedef enum
		{
			Off = 0,
			On = 1
		} DeformNormals;

		MeshPrimitiveSmoothSkinningOp();
		virtual ~MeshPrimitiveSmoothSkinningOp();

		IE_CORE_DECLARERUNTIMETYPED( MeshPrimitiveSmoothSkinningOp, MeshPrimitiveOp );

		SmoothSkinningDataParameter * smoothSkinningDataParameter();
		const SmoothSkinningDataParameter * smoothSkinningDataParameter() const;

		M44fVectorParameter * deformationPoseParameter();
		const M44fVectorParameter * deformationPoseParameter() const;

		/// parameter to control if the normals are deformed as well by the op
		IntParameter * deformNormalsParameter();
		const IntParameter * deformNormalsParameter() const;

		/// parameter that controls which algorithm is used for the deformation of the mesh
		IntParameter * blendParameter();
		const IntParameter * blendParameter() const;

		/// parameter that controls which PrimVar is used for the deformation
		StringParameter * pointPrimVarParameter();
		const StringParameter * pointPrimVarParameter() const;

		StringParameter * normalPrimVarParameter();
		const StringParameter * normalPrimVarParameter() const;


	protected:

		virtual void modifyTypedPrimitive( MeshPrimitive * typedPrimitive, const CompoundObject * operands );

	private:

		SmoothSkinningDataParameterPtr m_smoothSkinningDataParameter;
		IntParameterPtr m_blendParameter;
		IntParameterPtr m_deformNormalsParameter;
		M44fVectorParameterPtr m_deformationPoseParameter;
		StringParameterPtr m_pointPrimVarParameter;
		StringParameterPtr m_normalPrimVarParameter;

};

IE_CORE_DECLAREPTR( MeshPrimitiveSmoothSkinningOp );


} // namespace IECore

#endif // IE_CORE_MESHPRIMITIVESMOOTHSKINNINGOP_H
