//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_CURVESPRIMITIVEEVALUATOR_H
#define IECORE_CURVESPRIMITIVEEVALUATOR_H

#include "tbb/mutex.h"

#include "IECore/Export.h"
#include "IECore/PrimitiveEvaluator.h"
#include "IECore/BoundedKDTree.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CurvesPrimitive )

/// Implements the PrimitiveEvaluator interface to allow queries of
/// CurvesPrimitives.
/// \ingroup geometryProcessingGroup
class IECORE_API CurvesPrimitiveEvaluator : public PrimitiveEvaluator
{

	public :

		typedef CurvesPrimitive PrimitiveType;

		IE_CORE_DECLARERUNTIMETYPED( CurvesPrimitiveEvaluator, PrimitiveEvaluator );

		class IECORE_API Result : public PrimitiveEvaluator::Result
		{
			public :

				IE_CORE_DECLAREMEMBERPTR( Result );

				Imath::V3f point() const override;
				/// Not yet implemented.
				Imath::V3f normal() const override;
				/// U parameter will always be 0.
				Imath::V2f uv() const override;
				/// Not yet implemented.
				Imath::V3f uTangent() const override;
				Imath::V3f vTangent() const override;
				unsigned curveIndex() const;

				Imath::V3f vectorPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::V2f vec2PrimVar( const PrimitiveVariable &pv ) const override;
				float floatPrimVar( const PrimitiveVariable &pv ) const override;
				int intPrimVar( const PrimitiveVariable &pv ) const override;
				const std::string &stringPrimVar( const PrimitiveVariable &pv ) const override;
				Imath::Color3f colorPrimVar( const PrimitiveVariable &pv ) const override;
				half halfPrimVar( const PrimitiveVariable &pv ) const override;
				
			private :
			
				friend class CurvesPrimitiveEvaluator;
			
				Result( PrimitiveVariable p, bool linear, bool periodic );
								
				typedef void (Result::*InitFunction)( unsigned curveIndex, float v, const CurvesPrimitiveEvaluator *evaluator );

				template<bool linear, bool periodic>
				void init( unsigned curveIndex, float v, const CurvesPrimitiveEvaluator *evaluator );
				
				template<typename T>
				T primVar( const PrimitiveVariable &pv, const float *coefficients ) const;
				
				unsigned m_curveIndex;
				float m_v;
				float m_segmentV;
				float m_coefficients[4];
				float m_derivativeCoefficients[4];
				unsigned m_vertexDataIndices[4];
				unsigned m_varyingDataIndices[2];
				PrimitiveVariable m_p;
				bool m_linear;
				InitFunction m_init;
				
				
		};
		IE_CORE_DECLAREPTR( Result );

		CurvesPrimitiveEvaluator( ConstCurvesPrimitivePtr curves );
		virtual ~CurvesPrimitiveEvaluator();

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
		virtual bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const;
		/// Returns pointAtV( 0, uv[1], result ).
		virtual bool pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const;
		/// Not yet implemented.	
		virtual bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			PrimitiveEvaluator::Result *result, float maxDistance = Imath::limits<float>::max() ) const;
		/// Not yet implemented.	
		virtual int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const;
		//@}

		//! @name Curve specific query functions
		////////////////////////////////////////////////////////////////////////////////////////
		//@{
		bool pointAtV( unsigned curveIndex, float v, PrimitiveEvaluator::Result *result ) const;
		/// Returns the length of the given curve from vStart to vEnd.
		/// Returns 0.0f if inappropriate parameters are given.
		float curveLength( unsigned curveIndex, float vStart=0.0f, float vEnd=1.0f ) const;
		//@}

		//! @name Topology access
		/// These functions make it easier to index curve data manually in cases where the
		/// queries above are not sufficient.
		////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Equivalent to CurvesPrimitive::verticesPerCurve() but returns a reference to
		/// the vector within the IntVectorData.
		const std::vector<int> &verticesPerCurve() const;
		/// One value per curve, storing the offset to the first vertex value for
		/// that curve.
		const std::vector<int> &vertexDataOffsets() const;
		/// As above but providing the offset for data with varying interpolation.
		const std::vector<int> &varyingDataOffsets() const;
		//@}

	protected :
		
		/// \todo It would be much better if PrimitiveEvaluator::Description didn't require these create()
		/// functions and instead just called the constructors that have to exist anyway.
		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );
		friend struct PrimitiveEvaluator::Description<CurvesPrimitiveEvaluator>;
		static PrimitiveEvaluator::Description<CurvesPrimitiveEvaluator> g_evaluatorDescription;
		
	private :

		friend class Result;

		float integrateCurve( unsigned curveIndex, float vStart, float vEnd, int samples, Result& typedResult ) const;
		
		CurvesPrimitivePtr m_curvesPrimitive;
		const std::vector<int> &m_verticesPerCurve;
		std::vector<int> m_vertexDataOffsets; // one value per curve
		std::vector<int> m_varyingDataOffsets; // one value per curve
		PrimitiveVariable m_p;
		
		void buildTree();
		bool m_haveTree;
		typedef tbb::mutex TreeMutex;
		TreeMutex m_treeMutex;
		Box3fTree m_tree;
		std::vector<Imath::Box3f> m_treeBounds;
		struct Line;
		std::vector<Line> m_treeLines;
		
		void closestPointWalk( Box3fTree::NodeIndex nodeIndex, const Imath::V3f &p, unsigned &curveIndex, float &v, float &closestDistSquared ) const;
		
};

IE_CORE_DECLAREPTR( CurvesPrimitiveEvaluator );

} // namespace IECore

#endif // IECORE_CURVESPRIMITIVEEVALUATOR_H
