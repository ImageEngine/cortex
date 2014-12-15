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

#ifndef IE_CORE_PRIMITIVEEVALUATOR_H
#define IE_CORE_PRIMITIVEEVALUATOR_H

#include <string>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

#include "IECore/Export.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/Primitive.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( PrimitiveEvaluator );

/// Primitive evaluators permit spatial queries on primitives, such as determing the closest point, or retrieving the position
/// from a given UV coordinate, etc. Individual primitive types derive their own evaluators from this interface, and register
/// them by creating static instances of PrimitiveEvaluator::Description. The evaluator takes an internal copy of the primitive,
/// so subsequent changes to it will not be reflected in the evaluator's results.
/// \ingroup geometryProcessingGroup
class IECORE_API PrimitiveEvaluator : public RunTimeTyped
{
	public:

		typedef PrimitiveEvaluatorPtr ( *CreatorFn )( ConstPrimitivePtr );

		IE_CORE_DECLARERUNTIMETYPED( PrimitiveEvaluator, RunTimeTyped );

		/// An interface defining the possible results returned from a query. Attempting to read back the results of a failed
		/// query will yield undefined behaviour.
		/// \threading Implementations should ensure that it's safe to call multiple Result methods concurrently.
		class Result : public RefCounted
		{
			public:

				IE_CORE_DECLAREMEMBERPTR( Result );

				virtual ~Result();

				/// Returns the point computed by the query.
				virtual Imath::V3f point() const =0;

				/// Returns the geometric normal. Shading normals should be evaluated via an appropriate
				/// primitive variable.
				virtual Imath::V3f normal() const =0;

				/// Returns the UV from the result point.
				virtual Imath::V2f uv() const =0;

				/// Return the surface tangent along U
				virtual Imath::V3f uTangent() const =0;

				/// Return the surface tangent along U
				virtual Imath::V3f vTangent() const =0;

				//! @name Primitive Variable Functions
				/// These functions evaluate the given primitive variable using the appropriate interpolation type. Passing
				/// an invalid primvar leads to undefined behaviour, but will most likely crash the application.
				//@{
				virtual Imath::V3f          vectorPrimVar( const PrimitiveVariable &pv ) const =0;
				virtual float               floatPrimVar ( const PrimitiveVariable &pv ) const =0;
				virtual int                 intPrimVar   ( const PrimitiveVariable &pv ) const =0;
				virtual const std::string  &stringPrimVar( const PrimitiveVariable &pv ) const =0;
				virtual Imath::Color3f      colorPrimVar ( const PrimitiveVariable &pv ) const =0;
				virtual half                halfPrimVar  ( const PrimitiveVariable &pv ) const =0;
				//@}
		};
		IE_CORE_DECLAREPTR( Result );

		/// Returns a primitive evaluator which is compatible with the given primitive, from those
		/// evaluator types which have been registered.
		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );

		virtual ~PrimitiveEvaluator();

		/// Create a result instance which is suitable for passing to one of the query methods
		virtual ResultPtr createResult() const = 0;

		/// Returns the primitive which we're currently evaluating
		virtual ConstPrimitivePtr primitive() const = 0;

		//! @name Query Functions
		/// The Result passed to each of the methods below must previously have been created
		/// by a call to the createResult() method on the same evaluator instance as is being called -
		/// passing any other result will cause undefined behaviour.
		/// \threading Query implementations should ensure that they may be called from multiple
		/// concurrent threads provided that a unique Result instance is used per thread. This
		/// implies that all query data must be stored in the Result and not in the
		/// PrimitiveEvaluator itself.
		////////////////////////////////////////////////////////////////////////////////////////
		//@{

		/// Computes the signed distance between the given point and the primitive. By default this is
		/// just the signed distance between the point, and the plane specified by the closest point and normal,
		/// but derived class are free to override it as they see fit. Returns true on success.
		/// \todo: pass a result to this method in major version 6
		virtual bool signedDistance( const Imath::V3f &p, float &distance ) const;

		/// Computes the surface area of the primitive
		virtual float surfaceArea() const=0;

		/// Computes the volume of the primitive. The result is undefined if the primitive is not closed, or self intersects.
		virtual float volume() const=0;

		/// Computes the primitive's center of gravity. The result is undefined if the primitive is not closed, or self intersects.
		virtual Imath::V3f centerOfGravity() const=0;

		/// Find the closest point on the primitive to the given query point. Returns true on success.
		/// \todo Extend this to pass a maximum distance past which results are no longer interesting.
		virtual bool closestPoint( const Imath::V3f &p, Result *result ) const =0;

		/// Find the point on the primitive at the given query UV. Returns true on success
		virtual bool pointAtUV( const Imath::V2f &uv, Result *result ) const =0;

		/// Finds the closest intersection point for the given ray. Optionally specify a maximum distance of interest.
		/// Returns true if an intersection was found.
		virtual bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
			Result *result, float maxDistance = Imath::limits<float>::max() ) const =0;

		/// Finds all intersection points for the given ray. Optionally specify a maximum distance of interest.
		/// Returns the number of interections found.
		virtual int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
			std::vector<ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const =0;

		//@}

		/// Throws an exception if the passed result type is not compatible with the current evaluator
		virtual void validateResult( Result *result ) const =0;

		/// A class to allow registration of primitive evaluators with the system. Simply declare an instance
		/// of Description< YourEvaluatorType, YourPrimitiveType >
		template< typename E, typename P = typename E::PrimitiveType >
		struct Description
		{
			Description()
			{
				registerCreator( P::staticTypeId(), & E::create );
			}
		};

	private:

		static void registerCreator( TypeId id, CreatorFn f );

		typedef std::map< TypeId, CreatorFn > CreatorMap;

		static CreatorMap &getCreateFns();

};

IE_CORE_DECLAREPTR( PrimitiveEvaluator );

} // namespace IECore

#endif // IE_CORE_PRIMITIVEEVALUATOR_H
