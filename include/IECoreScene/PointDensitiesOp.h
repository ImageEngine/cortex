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

#ifndef IECORESCENE_POINTDENSITIESOP_H
#define IECORESCENE_POINTDENSITIESOP_H

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/NumericParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/Op.h"

namespace IECoreScene
{

/// The PointDensitiesOp calculates densities from a cloud of points.
/// \ingroup geometryProcessingGroup
class IECORESCENE_API PointDensitiesOp : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PointDensitiesOp, PointDensitiesOpTypeId, IECore::Op );

		PointDensitiesOp();
		~PointDensitiesOp() override;

		/// The Parameter for the input point cloud.
		IECore::ObjectParameter * pointParameter();
		const IECore::ObjectParameter * pointParameter() const;

		/// The Parameter that specifies how many neighbours to use
		/// in estimating the density.
		IECore::IntParameter * numNeighboursParameter();
		const IECore::IntParameter * numNeighboursParameter() const;

		/// The Parameter that specifies a simple multiplier on
		/// the density value.
		IECore::DoubleParameter * multiplierParameter();
		const IECore::DoubleParameter * multiplierParameter() const;

	protected :

		IECore::ObjectPtr doOperation( const IECore::CompoundObject * operands ) override;

	private :

		IECore::ObjectParameterPtr m_pointParameter;
		IECore::IntParameterPtr m_numNeighboursParameter;
		IECore::DoubleParameterPtr m_multiplierParameter;

};

IE_CORE_DECLAREPTR( PointDensitiesOp );

} // namespace IECoreScene

#endif // IECORESCENE_POINTDENSITIESOP_H
