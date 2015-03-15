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

#ifndef IE_CORE_POINTDENSITIESOP_H
#define IE_CORE_POINTDENSITIESOP_H

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The PointDensitiesOp calculates densities from a cloud of points.
/// \ingroup geometryProcessingGroup
class IECORE_API PointDensitiesOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( PointDensitiesOp, Op );

		PointDensitiesOp();
		virtual ~PointDensitiesOp();

		/// The Parameter for the input point cloud.
		ObjectParameter * pointParameter();
		const ObjectParameter * pointParameter() const;

		/// The Parameter that specifies how many neighbours to use
		/// in estimating the density.
		IntParameter * numNeighboursParameter();
		const IntParameter * numNeighboursParameter() const;

		/// The Parameter that specifies a simple multiplier on
		/// the density value.
		DoubleParameter * multiplierParameter();
		const DoubleParameter * multiplierParameter() const;

	protected :

		virtual ObjectPtr doOperation( const CompoundObject * operands );

	private :

		ObjectParameterPtr m_pointParameter;
		IntParameterPtr m_numNeighboursParameter;
		DoubleParameterPtr m_multiplierParameter;

};

IE_CORE_DECLAREPTR( PointDensitiesOp );

} // namespace IECore

#endif // IE_CORE_POINTDENSITIESOP_H
