//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_POINTREPULSIONOP_H
#define IE_CORE_POINTREPULSIONOP_H

#include "IECore/Export.h"
#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/Random.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )
IE_CORE_FORWARDDECLARE( ImagePrimitiveEvaluator )
IE_CORE_FORWARDDECLARE( MeshPrimitiveEvaluator )

/// \todo Class docs
/// \ingroup geometryProcessingGroup
class IECORE_API PointRepulsionOp : public ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( PointRepulsionOp, ModifyOp );

		PointRepulsionOp();
		virtual ~PointRepulsionOp();

		MeshPrimitiveParameter * meshParameter();
		const MeshPrimitiveParameter * meshParameter() const;

		ImagePrimitiveParameter * imageParameter();
		const ImagePrimitiveParameter * imageParameter() const;

		StringParameter * channelNameParameter();
		const StringParameter * channelNameParameter() const;

		IntParameter * numIterationsParameter();
		const IntParameter * numIterationsParameter() const;

		FloatParameter * magnitudeParameter();
		const FloatParameter * magnitudeParameter() const;

		StringParameter * weightsNameParameter();
		const StringParameter * weightsNameParameter() const;


	protected :

		void getNearestPointsAndDensities( ImagePrimitiveEvaluator *, const PrimitiveVariable &density, MeshPrimitiveEvaluator *, const PrimitiveVariable &s, const PrimitiveVariable &t, std::vector<Imath::V3f> &points, std::vector<float> &densities );
		void calculateForces( std::vector<Imath::V3f> &points, std::vector<float> &radii, std::vector<Imath::Box3f> &bounds, std::vector<Imath::V3f> &forces, Imath::Rand48 &generator, std::vector<float> &densities, float densityInv );

		virtual void modify( Object * object, const CompoundObject * operands );

		MeshPrimitiveParameterPtr m_meshParameter;
		ImagePrimitiveParameterPtr m_imageParameter;
		StringParameterPtr m_channelNameParameter;
		IntParameterPtr m_numIterationsParameter;
		FloatParameterPtr m_magnitudeParameter;
		StringParameterPtr m_weightsNameParameter;

};

IE_CORE_DECLAREPTR( PointRepulsionOp );

} // namespace IECore

#endif // IE_CORE_POINTREPULSIONOP_H
