//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CURVEEXTRUDEOP_H
#define IE_CORE_CURVEEXTRUDEOP_H

#include <vector>

#include "IECore/Op.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/PatchMeshPrimitive.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The CurveExtrudeOp lofts RiCurves into RiPatchMesh cylinders, obeying any width primvars present.
class CurveExtrudeOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( CurveExtrudeOp, Op );

		CurveExtrudeOp();
		virtual ~CurveExtrudeOp();

		CurvesPrimitiveParameterPtr curvesParameter();
		ConstCurvesPrimitiveParameterPtr curvesParameter() const;

		V2iParameterPtr resolutionParameter();
		ConstV2iParameterPtr resolutionParameter() const;

	protected :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

		void buildReferenceFrames( const std::vector< Imath::V3f > &points, std::vector< Imath::V3f > &tangents, std::vector< Imath::M44f > &frames ) const;

		PatchMeshPrimitivePtr buildPatchMesh( ConstCurvesPrimitivePtr curves, unsigned curveIndex, unsigned vertexOffset, unsigned varyingOffset ) const;

	private :

		CurvesPrimitiveParameterPtr m_curvesParameter;
		V2iParameterPtr m_resolutionParameter;

		struct VaryingFn;
		struct VertexFn;
		struct UniformFn;

};

IE_CORE_DECLAREPTR( CurveExtrudeOp );

} // namespace IECore

#endif // IE_CORE_CURVEEXTRUDEOP_H
