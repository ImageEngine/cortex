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

#ifndef IECORESCENE_POINTSPRIMITIVEEVALUATOR_H
#define IECORESCENE_POINTSPRIMITIVEEVALUATOR_H

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveEvaluator.h"

#include "IECore/KDTree.h"

#include "tbb/mutex.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( PointsPrimitive )

/// The PointsPrimitiveEvaluator implements the PrimitiveEvaluator interface for
/// PointsPrimitives.
/// \ingroup geometryProcessingGroup
class IECORESCENE_API PointsPrimitiveEvaluator : public PrimitiveEvaluator
{

	public :

		typedef PointsPrimitive PrimitiveType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PointsPrimitiveEvaluator, PointsPrimitiveEvaluatorTypeId, PrimitiveEvaluator );

		class IECORESCENE_API Result : public PrimitiveEvaluator::Result
		{
			public :

				IE_CORE_DECLAREMEMBERPTR( Result );

				Imath::V3f point() const override;
				/// Not yet implemented.
				Imath::V3f normal() const override;
				/// Not yet implemented.
				Imath::V2f uv() const override;
				/// Not yet implemented.
				Imath::V3f uTangent() const override;
				/// Not yet implemented.
				Imath::V3f vTangent() const override;
				size_t pointIndex() const ;

				Imath::V3f vectorPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::V2f vec2PrimVar( const PrimitiveVariable &pv ) const override;
				float floatPrimVar( const PrimitiveVariable &pv ) const override;
				int intPrimVar( const PrimitiveVariable &pv ) const override;
				const std::string &stringPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::Color3f colorPrimVar( const PrimitiveVariable &pv ) const override;
				half halfPrimVar( const PrimitiveVariable &pv ) const override;

			private :

				friend class PointsPrimitiveEvaluator;

				Result( PointsPrimitiveEvaluator::ConstPtr evaluator );

				template<typename T>
				const T &primVar( const PrimitiveVariable &pv ) const;

				size_t m_pointIndex;
				PointsPrimitiveEvaluator::ConstPtr m_evaluator;

		};
		IE_CORE_DECLAREPTR( Result );

		PointsPrimitiveEvaluator( ConstPointsPrimitivePtr points );
		~PointsPrimitiveEvaluator() override;

		ConstPrimitivePtr primitive() const override;

		PrimitiveEvaluator::ResultPtr createResult() const override;
		void validateResult( PrimitiveEvaluator::Result *result ) const override;

		//! @name Standard Query Functions
		////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Not yet implemented.
		float surfaceArea() const override;
		/// Not yet implemented.
		float volume() const override;
		/// Not yet implemented.
		Imath::V3f centerOfGravity() const override;
		/// Operates only on the point centres without taking into account their width.
		bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const override;
		/// Not yet implemented.
		bool pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const override;
		/// Not yet implemented.
		bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			PrimitiveEvaluator::Result *result, float maxDistance = Imath::limits<float>::max() ) const override;
		/// Not yet implemented.
		int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const override;
		//@}

	protected :

		/// \todo It would be much better if PrimitiveEvaluator::Description didn't require these create()
		/// functions and instead just called the constructors that have to exist anyway.
		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );
		friend struct PrimitiveEvaluator::Description<PointsPrimitiveEvaluator>;
		static PrimitiveEvaluator::Description<PointsPrimitiveEvaluator> g_evaluatorDescription;

	private :


		friend class Result;

		PointsPrimitivePtr m_pointsPrimitive;
		PrimitiveVariable m_p;
		const std::vector<Imath::V3f> *m_pVector;

		void buildTree();
		bool m_haveTree;
		typedef tbb::mutex TreeMutex;
		TreeMutex m_treeMutex;
		IECore::V3fTree m_tree;

};

IE_CORE_DECLAREPTR( PointsPrimitiveEvaluator );

} // namespace IECoreScene

#endif // IECORESCENE_POINTSPRIMITIVEEVALUATOR_H
