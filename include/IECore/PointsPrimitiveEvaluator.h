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

#ifndef IECORE_POINTSPRIMITIVEEVALUATOR_H
#define IECORE_POINTSPRIMITIVEEVALUATOR_H

#include "tbb/mutex.h"

#include "IECore/PrimitiveEvaluator.h"
#include "IECore/KDTree.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( PointsPrimitive )

/// The PointsPrimitiveEvaluator implements the PrimitiveEvaluator interface for
/// PointsPrimitives.
/// \ingroup geometryProcessingGroup
class PointsPrimitiveEvaluator : public PrimitiveEvaluator
{

	public :

		typedef PointsPrimitive PrimitiveType;

		IE_CORE_DECLARERUNTIMETYPED( PointsPrimitiveEvaluator, PrimitiveEvaluator );

		class Result : public PrimitiveEvaluator::Result
		{
			public :

				IE_CORE_DECLAREMEMBERPTR( Result );

				virtual Imath::V3f point() const;
				/// Not yet implemented.
				virtual Imath::V3f normal() const;
				/// Not yet implemented.
				virtual Imath::V2f uv() const;
				/// Not yet implemented.
				virtual Imath::V3f uTangent() const;
				/// Not yet implemented.
				virtual Imath::V3f vTangent() const;
				size_t pointIndex() const;

				virtual Imath::V3f vectorPrimVar( const PrimitiveVariable &pv ) const;
				virtual float floatPrimVar( const PrimitiveVariable &pv ) const;
				virtual int intPrimVar( const PrimitiveVariable &pv ) const;
				virtual const std::string &stringPrimVar( const PrimitiveVariable &pv ) const;
				virtual Imath::Color3f colorPrimVar( const PrimitiveVariable &pv ) const;
				virtual half halfPrimVar( const PrimitiveVariable &pv ) const;
				
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
		virtual ~PointsPrimitiveEvaluator();

		virtual ConstPrimitivePtr primitive() const;
		
		virtual PrimitiveEvaluator::ResultPtr createResult() const;
		virtual void validateResult( PrimitiveEvaluator::Result *result ) const;

		//! @name Standard Query Functions
		////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Not yet implemented.
		virtual float surfaceArea() const;
		/// Not yet implemented.
		virtual float volume() const;
		/// Not yet implemented.
		virtual Imath::V3f centerOfGravity() const;
		/// Operates only on the point centres without taking into account their width.
		virtual bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const;
		/// Not yet implemented.
		virtual bool pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const;
		/// Not yet implemented.	
		virtual bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			PrimitiveEvaluator::Result *result, float maxDistance = Imath::limits<float>::max() ) const;
		/// Not yet implemented.	
		virtual int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const;
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
		V3fTree m_tree;		
		
};

IE_CORE_DECLAREPTR( PointsPrimitiveEvaluator );

} // namespace IECore

#endif // IECORE_POINTSPRIMITIVEEVALUATOR_H
