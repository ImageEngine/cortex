//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_POINTDISTRIBUTIONOP_H
#define IECORE_POINTDISTRIBUTIONOP_H

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveParameter.h"

namespace IECore
{

/// The PointDistributionOp distributes points over a mesh using an IECore::PointDistribution in UV space
/// and mapping it to 3d space. It gives a more even distribution than MappedRandomPointDistributionOp,
/// but requires UVs that are well layed out in order to work efficiently.
/// \ingroup geometryProcessingGroup
class IECORE_API PointDistributionOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( PointDistributionOp, Op );

		PointDistributionOp();
		virtual ~PointDistributionOp();

		MeshPrimitiveParameter *meshParameter();
		const MeshPrimitiveParameter *meshParameter() const;
		
		FloatParameter *densityParameter();
		const FloatParameter *densityParameter() const;

	protected :

		void processMesh( const IECore::MeshPrimitive *mesh );
		
		virtual ObjectPtr doOperation( const CompoundObject * operands );

	private :

		MeshPrimitiveParameterPtr m_meshParameter;
		FloatParameterPtr m_densityParameter;
		V2fParameterPtr m_offsetParameter;
		StringParameterPtr m_densityPrimVarNameParameter;
		StringParameterPtr m_pRefPrimVarNameParameter;
		StringParameterPtr m_uPrimVarNameParameter;
		StringParameterPtr m_vPrimVarNameParameter;
		
		struct Emitter;
		struct Generator;
		
		// filled in by processMesh()
		MeshPrimitivePtr m_mesh;
		MeshPrimitiveEvaluatorPtr m_meshEvaluator;

};

IE_CORE_DECLAREPTR( PointDistributionOp );

} // namespace IECore

#endif // IECORE_POINTDISTRIBUTIONOP_H
