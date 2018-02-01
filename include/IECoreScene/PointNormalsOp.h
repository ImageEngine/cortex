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

#ifndef IECORESCENE_POINTNORMALSOP_H
#define IECORESCENE_POINTNORMALSOP_H

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/NumericParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/Op.h"

namespace IECoreScene
{

/// The PointNormalsOp calculates normals for a cloud of points. It's designed
/// to work with a volume of points rather than a shell, so is of more use in calculating
/// normals for particle simulations and the like rather than surface reconstruction.
/// \todo This was written for a production and then never needed. The normals it produces
/// are somewhat noisy - we could probably do with improving the process.
/// \ingroup geometryProcessingGroup
class IECORESCENE_API PointNormalsOp : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PointNormalsOp, PointNormalsOpTypeId, IECore::Op );

		PointNormalsOp();
		~PointNormalsOp() override;

		/// The Parameter for the input point cloud.
		IECore::ObjectParameter * pointParameter();
		const IECore::ObjectParameter * pointParameter() const;

		/// The Parameter that specifies how many neighbours to use
		/// in estimating the density.
		IECore::IntParameter * numNeighboursParameter();
		const IECore::IntParameter * numNeighboursParameter() const;

	protected :

		IECore::ObjectPtr doOperation( const IECore::CompoundObject * operands ) override;

	private :

		IECore::ObjectParameterPtr m_pointParameter;
		IECore::IntParameterPtr m_numNeighboursParameter;

};

IE_CORE_DECLAREPTR( PointNormalsOp );

} // namespace IECoreScene

#endif // IECORESCENE_POINTNORMALSOP_H
