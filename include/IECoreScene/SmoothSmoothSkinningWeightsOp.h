//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_SMOOTHSMOOTHSKINNINGWEIGHTSOP_H
#define IECORESCENE_SMOOTHSMOOTHSKINNINGWEIGHTSOP_H

#include "boost/graph/adjacency_list.hpp"

#include "IECore/ModifyOp.h"
#include "IECore/FrameListParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECoreScene/Export.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/TypedPrimitiveParameter.h"

namespace IECoreScene
{

/// The SmoothSmoothSkinningWeightsOp smooths the weights of SmoothSkinningData using the average weights from
/// connected vertices. The connectivity information is calculated from a MeshPrimitive and the neighbouring weights
/// are averaged and interpolated using the smoothingRatio over any number of iterations. Locks can be applied to the
/// influences per iteration and the unlocked weights will be normalized accordingly. There is an optional vertexIndices
/// parameter which applies smoothing to user chosen vertices only. In this case, the smoothing weights will still be
/// interpolated from all connected vertices, regardless of which vertices have been selected.
/// \ingroup skinningGroup
class IECORESCENE_API SmoothSmoothSkinningWeightsOp : public IECore::ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SmoothSmoothSkinningWeightsOp, SmoothSmoothSkinningWeightsOpTypeId, IECore::ModifyOp );

		SmoothSmoothSkinningWeightsOp();
		~SmoothSmoothSkinningWeightsOp() override;

	protected :

		void modify( IECore::Object *object, const IECore::CompoundObject *operands ) override;

	private :

		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS > Graph;
		typedef boost::property_map<Graph, boost::vertex_index_t>::type VertexIdMap;
		typedef Graph::vertex_descriptor Vertex;
		typedef Graph::adjacency_iterator NeighbourIterator;
		typedef std::pair<NeighbourIterator, NeighbourIterator> NeighbourIteratorRange;

		MeshPrimitiveParameterPtr m_meshParameter;
		IECore::FrameListParameterPtr m_vertexIdsParameter;
		IECore::FloatParameterPtr m_smoothingRatioParameter;
		IECore::IntParameterPtr m_iterationsParameter;
		IECore::BoolParameterPtr m_useLocksParameter;
		IECore::BoolVectorParameterPtr m_influenceLocksParameter;
};

IE_CORE_DECLAREPTR( SmoothSmoothSkinningWeightsOp );

} // namespace IECoreScene

#endif // IECORESCENE_SMOOTHSMOOTHSKINNINGWEIGHTSOP_H
