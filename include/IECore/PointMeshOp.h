//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_POINTMESHOP_H
#define IE_CORE_POINTMESHOP_H

#include "IECore/Op.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/TypedObjectParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The PointMeshOp calculates a mesh from an isosurface defined by a point cloud
class PointMeshOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( PointMeshOp, Op );

		PointMeshOp();
		virtual ~PointMeshOp();

		/// The Parameter for the input point cloud.
		ObjectParameterPtr pointParameter();
		ConstObjectParameterPtr pointParameter() const;

		/// The Parameter that specifies the radius of each point-centered sphere
		DoubleVectorParameterPtr radiusParameter();
		ConstDoubleVectorParameterPtr radiusParameter() const;

		/// The Parameter that specifies the strength fo each sphere
		DoubleVectorParameterPtr strengthParameter();
		ConstDoubleVectorParameterPtr strengthParameter() const;

		/// The Parameter that specifies the threshold at which to build the mesh
		FloatParameterPtr thresholdParameter();
		ConstFloatParameterPtr thresholdParameter() const;

		V3iParameterPtr resolutionParameter();
		ConstV3iParameterPtr resolutionParameter() const;

		Box3fParameterPtr boundParameter();
		ConstBox3fParameterPtr boundParameter() const;

	protected :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

	private :

		ObjectParameterPtr m_pointParameter;
		DoubleVectorParameterPtr m_radiusParameter;
		DoubleVectorParameterPtr m_strengthParameter;
		FloatParameterPtr m_thresholdParameter;
		V3iParameterPtr m_resolutionParameter;
		Box3fParameterPtr m_boundParameter;


};

IE_CORE_DECLAREPTR( PointMeshOp );

} // namespace IECore

#endif // IE_CORE_POINTMESHOP_H
