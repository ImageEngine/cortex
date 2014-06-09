//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PARTICLEMESHOP_H
#define IE_CORE_PARTICLEMESHOP_H

#include "IECore/Op.h"
#include "IECore/FileNameParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The ParticleMeshOp calculates a mesh from an isosurface defined by a point cloud
/// \ingroup geometryProcessingGroup
class ParticleMeshOp : public Op
{
	public :

		typedef enum
		{
			Resolution = 0,
			DivisionSize = 1,
		} GridMethod;

		IE_CORE_DECLARERUNTIMETYPED( ParticleMeshOp, Op );

		ParticleMeshOp();
		virtual ~ParticleMeshOp();

		FileNameParameter * fileNameParameter();
		const FileNameParameter * fileNameParameter() const;

		StringParameter * positionAttributeParameter();
		const StringParameter * positionAttributeParameter() const;

		BoolParameter * useRadiusAttributeParameter();
		const BoolParameter * useRadiusAttributeParameter() const;

		StringParameter * radiusAttributeParameter();
		const StringParameter * radiusAttributeParameter() const;

		FloatParameter * radiusParameter();
		const FloatParameter * radiusParameter() const;

		FloatParameter * radiusScaleParameter();
		const FloatParameter * radiusScaleParameter() const;

		BoolParameter * useStrengthAttributeParameter();
		const BoolParameter * useStrengthAttributeParameter() const;

		StringParameter * strengthAttributeParameter();
		const StringParameter * strengthAttributeParameter() const;

		FloatParameter * strengthParameter();
		const FloatParameter * strengthParameter() const;

		FloatParameter * strengthScaleParameter();
		const FloatParameter * strengthScaleParameter() const;

		FloatParameter * thresholdParameter();
		const FloatParameter * thresholdParameter() const;

		V3iParameter * resolutionParameter();
		const V3iParameter * resolutionParameter() const;

		Box3fParameter * boundParameter();
		const Box3fParameter * boundParameter() const;

		BoolParameter * automaticBoundParameter();
		const BoolParameter * automaticBoundParameter() const;

		IntParameter * gridMethodParameter();
		const IntParameter * gridMethodParameter() const;

		V3fParameter * divisionSizeParameter();
		const V3fParameter * divisionSizeParameter() const;

		FloatParameter * boundExtendParameter();
		const FloatParameter * boundExtendParameter() const;

	protected :

		virtual ObjectPtr doOperation( const CompoundObject * operands );

	private :

		FileNameParameterPtr m_fileNameParameter;
		StringParameterPtr m_positionAttributeParameter;
		BoolParameterPtr m_useRadiusAttributeParameter;
		StringParameterPtr m_radiusAttributeParameter;
		FloatParameterPtr m_radiusParameter;
		BoolParameterPtr m_useStrengthAttributeParameter;
		StringParameterPtr m_strengthAttributeParameter;
		FloatParameterPtr m_strengthParameter;

		FloatParameterPtr m_thresholdParameter;
		V3iParameterPtr m_resolutionParameter;
		Box3fParameterPtr m_boundParameter;

		FloatParameterPtr m_radiusScaleParameter;
		FloatParameterPtr m_strengthScaleParameter;

		BoolParameterPtr m_automaticBoundParameter;
		IntParameterPtr m_gridMethodParameter;
		V3fParameterPtr m_divisionSizeParameter;
		FloatParameterPtr m_boundExtendParameter;

};

IE_CORE_DECLAREPTR( ParticleMeshOp );

} // namespace IECore

#endif // IE_CORE_PARTICLEMESHOP_H
