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

#ifndef IECORESCENE_SPHEREPRIMITIVEEVALUATOR_H
#define IECORESCENE_SPHEREPRIMITIVEEVALUATOR_H

#include <vector>

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveEvaluator.h"
#include "IECoreScene/SpherePrimitive.h"

namespace IECoreScene
{

/// An implementation of PrimitiveEvaluator to allow spatial queries to be performed on spheres.
/// \todo Currently ignores zMin, zMax, thetaMax parameters, instead assuming that there sphere is whole and facing outwards.
/// \ingroup geometryProcessingGroup
class IECORESCENE_API SpherePrimitiveEvaluator : public PrimitiveEvaluator
{
	public:

		typedef SpherePrimitive PrimitiveType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SpherePrimitiveEvaluator, SpherePrimitiveEvaluatorTypeId, PrimitiveEvaluator );

		class Result : public PrimitiveEvaluator::Result
		{
			friend class SpherePrimitiveEvaluator;

			public:

				IE_CORE_DECLAREMEMBERPTR( Result );

				Result();

				Imath::V3f point() const override;
				Imath::V3f normal() const override;
				Imath::V2f uv() const override;
				Imath::V3f uTangent() const override;
				Imath::V3f vTangent() const override;

				Imath::V3f vectorPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::V2f vec2PrimVar( const PrimitiveVariable &pv ) const override;
				float floatPrimVar( const PrimitiveVariable &pv ) const override;
				int intPrimVar( const PrimitiveVariable &pv ) const override;
				const std::string &stringPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::Color3f colorPrimVar( const PrimitiveVariable &pv ) const override;
				half halfPrimVar( const PrimitiveVariable &pv ) const override;

			protected:

				Imath::V3f m_p;

				template<typename T>
				T getPrimVar( const PrimitiveVariable &pv ) const;


		};
		IE_CORE_DECLAREPTR( Result );

		SpherePrimitiveEvaluator( ConstSpherePrimitivePtr sphere );

		~SpherePrimitiveEvaluator() override;

		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );

		ConstPrimitivePtr primitive() const override;

		PrimitiveEvaluator::ResultPtr createResult() const override;

		void validateResult( PrimitiveEvaluator::Result *result ) const override;

		bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const override;

		bool pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const override;

		bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			PrimitiveEvaluator::Result *result, float maxDistance = Imath::limits<float>::max() ) const override;

		int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const override;

		float volume() const override;

		Imath::V3f centerOfGravity() const override;

		float surfaceArea() const override;

	protected:

		ConstSpherePrimitivePtr m_sphere;
};

IE_CORE_DECLAREPTR( SpherePrimitiveEvaluator );

} // namespace IECoreScene

#endif // IECORESCENE_SPHEREPRIMITIVEEVALUATOR_H
