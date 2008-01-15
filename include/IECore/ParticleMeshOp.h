//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
#include "IECore/TypedParameter.h"
#include "IECore/TypedObjectParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The ParticleMeshOp calculates a mesh from an isosurface defined by a point cloud
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
		
		FileNameParameterPtr fileNameParameter();
		ConstFileNameParameterPtr fileNameParameter() const;
				
		StringParameterPtr positionAttributeParameter();
		ConstStringParameterPtr positionAttributeParameter() const;
				
		BoolParameterPtr useRadiusAttributeParameter();
		ConstBoolParameterPtr useRadiusAttributeParameter() const;
				
		StringParameterPtr radiusAttributeParameter();
		ConstStringParameterPtr radiusAttributeParameter() const;
				
		FloatParameterPtr radiusParameter();
		ConstFloatParameterPtr radiusParameter() const;
		
		FloatParameterPtr radiusScaleParameter();
		ConstFloatParameterPtr radiusScaleParameter() const;
				
		BoolParameterPtr useStrengthAttributeParameter();
		ConstBoolParameterPtr useStrengthAttributeParameter() const;
		
		StringParameterPtr strengthAttributeParameter();
		ConstStringParameterPtr strengthAttributeParameter() const;
		
		FloatParameterPtr strengthParameter();
		ConstFloatParameterPtr strengthParameter() const;
		
		FloatParameterPtr strengthScaleParameter();
		ConstFloatParameterPtr strengthScaleParameter() const;

		FloatParameterPtr thresholdParameter();
		ConstFloatParameterPtr thresholdParameter() const;
		
		V3iParameterPtr resolutionParameter();
		ConstV3iParameterPtr resolutionParameter() const;
				
		Box3fParameterPtr boundParameter();
		ConstBox3fParameterPtr boundParameter() const;
		
		BoolParameterPtr automaticBoundParameter();
		BoolParameterPtr automaticBoundParameter() const;
						
		IntParameterPtr gridMethodParameter();
		IntParameterPtr gridMethodParameter() const;
		
		V3fParameterPtr divisionSizeParameter();
		ConstV3fParameterPtr divisionSizeParameter() const;								
		
	protected :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );
	
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

};

IE_CORE_DECLAREPTR( ParticleMeshOp );

} // namespace IECore

#endif // IE_CORE_PARTICLEMESHOP_H
