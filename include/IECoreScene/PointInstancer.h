//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2026, Cinesite VFX Ltd. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#pragma once

#include "IECoreScene/PointsPrimitive.h"

#include <unordered_set>

namespace IECoreScene
{

class IECORESCENE_API PointInstancer : public IECoreScene::PointsPrimitive
{

	public :

		PointInstancer( size_t numPoints = 0 );
		~PointInstancer() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( PointInstancer, IECoreScene::PointInstancerTypeId, IECoreScene::PointsPrimitive );

		/// PrimitiveVariable accessors
		/// ===========================
		///
		/// Accessor methods are provided for all PrimitiveVariables which have
		/// specific meaning to the PointInstancer.
		///
		/// In USD, these are stored as bare attributes rather than primvars,
		/// and as such they can't have indices. We prefer to use
		/// PrimitiveVariables for uniformity of access in algorithms such as
		/// `PointsAlgo::deletePoints()` and for nodes in Gaffer, meaning that
		/// in Cortex they _can_ have indices. The accessors are designed to
		/// discourage the use of indices, but remain robust should they be
		/// encountered :
		///
		/// - `set()` functions create a PrimitiveVariable with null `indices`.
		///   Pass a `nullptr` value to remove the PrimitiveVariable.
		/// - `get()` functions return an IndexedView. An invalid view is returned
		///   if the PrimitiveVariable doesn't exist or has the wrong type or
		///   interpolation.
		///
		/// For full flexibility when needed, access `Primitive::variables`
		/// directly instead.
		///
		/// \todo We currently use our own names for each of these properties, for
		/// backwards compatibility with Instancer workflows in Gaffer. But in future
		/// we should align the names to USD.

		/// Sets the prototypes to be instanced. Interpretation is left to
		/// the consuming rendering system, but in practice these are locations
		/// in a SceneInterface or Gaffer scene.
		void setPrototypes( IECore::StringVectorDataPtr &prototypes );
		/// Returns an invalid view if the variable doesn't exist, or if it
		/// exists but has the wrong type or interpolation.
		PrimitiveVariable::IndexedView<std::string> getPrototypes() const;

		void setPrototypeIndex( const IECore::IntVectorDataPtr &prototypeIndex );
		PrimitiveVariable::IndexedView<int> getPrototypeIndex() const;

		void setPosition( const IECore::V3fVectorDataPtr &position );
		PrimitiveVariable::IndexedView<Imath::V3f> getPosition() const;

		void setScale( const IECore::V3fVectorDataPtr &scale );
		PrimitiveVariable::IndexedView<Imath::V3f> getScale() const;

		void setOrientation( const IECore::QuatfVectorDataPtr &orientation );
		PrimitiveVariable::IndexedView<Imath::Quatf> getOrientation() const;

		void setID( const IECore::Int64VectorDataPtr &ids );
		PrimitiveVariable::IndexedView<int64_t> getID() const;

		/// Constant primitive variable named "invisibleIds".
		///
		/// > Note : In addition to this, USD also has "inactiveIds" whose
		/// > only distinguishing feature is not being animatable. Since we
		/// > have no concept of a non-animatable PrimitiveVariable, we have
		/// > only "invisibleIds".
		void setInvisibleIDs( const IECore::Int64VectorDataPtr &invisibleIds );
		PrimitiveVariable::IndexedView<int64_t> getInvisibleIDs() const;

		/// Queries
		/// =======
		///
		/// Higher-level functionality used to evaluate the instancing.

		struct IECORESCENE_API VisibilityQuery
		{

			VisibilityQuery( const PointInstancer &pointInstancer )
				:	m_id( pointInstancer.getID() )
			{
				if( auto invisibleIds = pointInstancer.getInvisibleIDs() )
				{
					m_invisibleIds.insert( invisibleIds.begin(), invisibleIds.end() );
				}
			}

			bool visible( size_t pointIndex )
			{
				const size_t id = m_id ? m_id[pointIndex] : pointIndex;
				return m_invisibleIds.find( id ) == m_invisibleIds.end();
			}

			private :

				PrimitiveVariable::IndexedView<int64_t> m_id;
				std::unordered_set<int64_t> m_invisibleIds;

		};

		/// Utility class to query properties for individual instances.
		struct IECORESCENE_API TransformQuery
		{

			TransformQuery( const PointInstancer &pointInstancer );

			Imath::M44f transform( size_t pointIndex ) const;

			private :

				PrimitiveVariable::IndexedView<Imath::V3f> m_position;
				PrimitiveVariable::IndexedView<Imath::Quatf> m_orientation;
				PrimitiveVariable::IndexedView<Imath::V3f> m_scale;

		};

};

IE_CORE_DECLAREPTR( PointInstancer )

} // namespace IECoreScene
