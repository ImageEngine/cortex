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

#ifndef IECORESCENE_MESHPRIMITIVEEVALUATOR_H
#define IECORESCENE_MESHPRIMITIVEEVALUATOR_H

#include <vector>

#include "tbb/mutex.h"

#include "IECore/BoundedKDTree.h"

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveEvaluator.h"
#include "IECoreScene/MeshPrimitive.h"

namespace IECoreScene
{

/// An implementation of PrimitiveEvaluator to allow spatial queries to be performed on MeshPrimitive instances
/// \ingroup geometryProcessingGroup
class IECORESCENE_API MeshPrimitiveEvaluator : public PrimitiveEvaluator
{
	public:

		typedef MeshPrimitive PrimitiveType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MeshPrimitiveEvaluator, MeshPrimitiveEvaluatorTypeId, PrimitiveEvaluator );

		class IECORESCENE_API Result : public PrimitiveEvaluator::Result
		{
			friend class MeshPrimitiveEvaluator;

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

				unsigned int triangleIndex() const;
				const Imath::V3f &barycentricCoordinates() const;
				const Imath::V3i &vertexIds() const;

			protected:

				Imath::V3i m_vertexIds;
				Imath::V3f m_bary;
				/// \todo I don't see why we have to compute these for every query when we don't even
				/// know if they'll be requested.
				Imath::V3f m_p;
				Imath::V3f m_n;
				Imath::V2f m_uv;
				unsigned int m_triangleIdx;

				template<typename T>
				T getPrimVar( const PrimitiveVariable &pv ) const;


		};
		IE_CORE_DECLAREPTR( Result );

		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );

		MeshPrimitiveEvaluator( ConstMeshPrimitivePtr mesh );

		~MeshPrimitiveEvaluator() override;

		ConstPrimitivePtr primitive() const override;
		MeshPrimitive::ConstPtr mesh() const;

		PrimitiveEvaluator::ResultPtr createResult() const override;

		void validateResult( PrimitiveEvaluator::Result *result ) const override;

		bool closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const override;

		bool pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const override;

		bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			PrimitiveEvaluator::Result *result, float maxDistance = Imath::limits<float>::max() ) const override;

		int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const override;

		/// A query specific to the MeshPrimitiveEvaluator, this just chooses a barycentric position on a specific triangle.
		bool barycentricPosition( unsigned int triangleIndex, const Imath::V3f &barycentricCoordinates, PrimitiveEvaluator::Result *result ) const;

		bool signedDistance( const Imath::V3f &p, float &distance, PrimitiveEvaluator::Result *result ) const override;

		float volume() const override;

		Imath::V3f centerOfGravity() const override;

		float surfaceArea() const override;


		/// Returns a bounding box covering all the uv coordinates of the mesh.
		const Imath::Box2f uvBound() const;

		//! @name Internal KDTrees.
		/// The MeshPrimitiveEvaluator uses internal KDTrees to perform many of
		/// its queries. Const access is provided to these so that clients can use them
		/// in implementing their own algorithms.
		//////////////////////////////////////////////////////////////////////////
		//@{
		/// A type for storing the bounding box for a triangle.
		typedef Imath::Box3f TriangleBound;
		/// A type for storing an array of bounding boxes, one per triangle.
		typedef std::vector<TriangleBound> TriangleBoundVector;
		/// A BoundedKDTree providing accelerated lookups of triangles using their bounding boxes.
		typedef IECore::BoundedKDTree<TriangleBoundVector::iterator> TriangleBoundTree;
		/// Returns a pointer to the bounding boxes for each triangle.
		const TriangleBoundVector *triangleBounds() const;
		/// Returns a pointer to a tree that can be used for performing fast spacial queries.
		///  The iterators in this tree point to elements in the vector returned by triangleBounds().
		const TriangleBoundTree *triangleBoundTree() const;

		/// A type for storing the uv bounding box for a triangle.
		typedef Imath::Box2f UVBound;
		/// A type for storing an array of uv bounds, one per triangle.
		typedef std::vector<UVBound> UVBoundVector;
		/// A BoundedKDTree providing accelerated lookups of triangles using their uv bounds.
		typedef IECore::BoundedKDTree<UVBoundVector::iterator> UVBoundTree;
		/// Returns a pointer to the uv bounding boxes for each triangle. Note that this function may
		/// return 0 in the case of the mesh not having suitable uvs.
		const UVBoundVector *uvBounds() const;
		/// Returns a pointer to a tree than can be used for performing fast uv queries. The iterators
		/// in this tree point to the elements in the vector returned by uvBounds(). Note that
		/// this function may return 0 in the case of the mesh not having suitable uvs.
		const UVBoundTree *uvBoundTree() const;
		//@}

	protected:

		ConstMeshPrimitivePtr m_mesh;
		IECore::ConstV3fVectorDataPtr m_verts;
		const std::vector<int> *m_meshVertexIds;

		TriangleBoundVector m_triangles;
		TriangleBoundTree *m_tree;

		UVBoundVector m_uvTriangles;
		UVBoundTree *m_uvTree;

		bool pointAtUVWalk( UVBoundTree::NodeIndex nodeIndex, const Imath::V2f &targetUV, Result *result ) const;
		void closestPointWalk( TriangleBoundTree::NodeIndex nodeIndex, const Imath::V3f &p, float &closestDistanceSqrd, Result *result ) const;
		bool intersectionPointWalk( TriangleBoundTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float &maxDistSqrd, Result *result, bool &hit ) const;
		void intersectionPointsWalk( TriangleBoundTree::NodeIndex nodeIndex, const Imath::Line3f &ray, float maxDistSqrd, std::vector<PrimitiveEvaluator::ResultPtr> &results ) const;

		void calculateMassProperties() const;
		void calculateAverageNormals() const;

		void triangleUVs( size_t triangleIndex, const Imath::V3i &vertexIds, Imath::V2f uv[3] ) const;
		PrimitiveVariable m_uv;

		mutable bool m_haveMassProperties;
		mutable float m_volume;
		mutable Imath::V3f m_centerOfGravity;
		mutable Imath::M33f m_inertia;

		mutable bool m_haveSurfaceArea;
		mutable float m_surfaceArea;

		typedef tbb::mutex NormalsMutex;
		mutable NormalsMutex m_normalsMutex;
		mutable bool m_haveAverageNormals;
		typedef int VertexIndex;
		typedef int TriangleIndex;
		typedef std::pair<VertexIndex, VertexIndex> Edge;

		typedef std::map< Edge, Imath::V3f > EdgeAverageNormals;
		mutable EdgeAverageNormals m_edgeAverageNormals;

		mutable IECore::V3fVectorDataPtr m_vertexAngleWeightedNormals;

};

IE_CORE_DECLAREPTR( MeshPrimitiveEvaluator );

} // namespace IECoreScene

#endif // IECORESCENE_MESHPRIMITIVEEVALUATOR_H
