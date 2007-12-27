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

#ifndef IE_CORE_ParticleMeshOp_H
#define IE_CORE_ParticleMeshOp_H

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
				
		DoubleParameterPtr radiusParameter();
		ConstDoubleParameterPtr radiusParameter() const;
		
		DoubleParameterPtr radiusScaleParameter();
		ConstDoubleParameterPtr radiusScaleParameter() const;
				
		BoolParameterPtr useStrengthAttributeParameter();
		ConstBoolParameterPtr useStrengthAttributeParameter() const;
		
		StringParameterPtr strengthAttributeParameter();
		ConstStringParameterPtr strengthAttributeParameter() const;
		
		DoubleParameterPtr strengthParameter();
		ConstDoubleParameterPtr strengthParameter() const;
		
		DoubleParameterPtr strengthScaleParameter();
		ConstDoubleParameterPtr strengthScaleParameter() const;

		DoubleParameterPtr thresholdParameter();
		ConstDoubleParameterPtr thresholdParameter() const;
		
		V3iParameterPtr resolutionParameter();
		ConstV3iParameterPtr resolutionParameter() const;
		
		Box3dParameterPtr boundParameter();
		ConstBox3dParameterPtr boundParameter() const;
								
		
	protected :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );
	
	private :
		
		FileNameParameterPtr m_fileNameParameter;
		StringParameterPtr m_positionAttributeParameter;
		BoolParameterPtr m_useRadiusAttributeParameter;
		StringParameterPtr m_radiusAttributeParameter;
		DoubleParameterPtr m_radiusParameter;
		BoolParameterPtr m_useStrengthAttributeParameter;
		StringParameterPtr m_strengthAttributeParameter;
		DoubleParameterPtr m_strengthParameter;
		
		DoubleParameterPtr m_thresholdParameter;
		V3iParameterPtr m_resolutionParameter;
		Box3dParameterPtr m_boundParameter;
		
		DoubleParameterPtr m_radiusScaleParameter;
		DoubleParameterPtr m_strengthScaleParameter;

};

IE_CORE_DECLAREPTR( ParticleMeshOp );

} // namespace IECore

#endif // IE_CORE_ParticleMeshOp_H
