//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MESHPRIMITIVEIMPLICITSURFACEOP_H
#define IE_CORE_MESHPRIMITIVEIMPLICITSURFACEOP_H

#include <vector>

#include "IECore/TypedPrimitiveOp.h"
#include "IECore/PrimitiveImplicitSurfaceFunction.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"


namespace IECore
{

/// A MeshPrimitiveOp to create an implicit surface from a MeshPrimitive,
/// then remesh it using the MarchingCubes algorithm over the given
/// domain at a specified threshold (iso-value)
class MeshPrimitiveImplicitSurfaceOp : public TypedPrimitiveOp<MeshPrimitive>
{
	public:

		typedef enum
		{
			Resolution = 0,
			DivisionSize = 1,
		} GridMethod;

		MeshPrimitiveImplicitSurfaceOp();
		virtual ~MeshPrimitiveImplicitSurfaceOp();

		IE_CORE_DECLARERUNTIMETYPED( MeshPrimitiveImplicitSurfaceOp, MeshPrimitiveOp );

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

		FloatParameterPtr boundExtendParameter();
		FloatParameterPtr boundExtendParameter() const;


	protected:

		virtual void modifyTypedPrimitive( MeshPrimitivePtr typedPrimitive, ConstCompoundObjectPtr operands );

	private:

		FloatParameterPtr m_thresholdParameter;
		V3iParameterPtr m_resolutionParameter;
		Box3fParameterPtr m_boundParameter;

		BoolParameterPtr m_automaticBoundParameter;
		IntParameterPtr m_gridMethodParameter;
		V3fParameterPtr m_divisionSizeParameter;
		FloatParameterPtr m_boundExtendParameter;

};

IE_CORE_DECLAREPTR( MeshPrimitiveImplicitSurfaceOp );


} // namespace IECore

#endif // IE_CORE_MESHPRIMITIVEIMPLICITSURFACEOP_H
