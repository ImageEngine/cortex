//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SPHEREPRIMITIVEEVALUATOR_H
#define IE_CORE_SPHEREPRIMITIVEEVALUATOR_H

#include <vector>

#include "IECore/PrimitiveEvaluator.h"
#include "IECore/SpherePrimitive.h"

namespace IECore
{

/// An implementation of PrimitiveEvaluator to allow spatial queries to be performed on spheres. 
/// \todo Currently ignores zMin, zMax, thetaMax parameters, instead assuming that there sphere is whole and facing outwards.
class SpherePrimitiveEvaluator : public PrimitiveEvaluator
{
	public:
	
		typedef SpherePrimitive PrimitiveType;
	
		IE_CORE_DECLARERUNTIMETYPED( SpherePrimitiveEvaluator, PrimitiveEvaluator );
					
		class Result : public PrimitiveEvaluator::Result
		{
			friend class SpherePrimitiveEvaluator;
			
			public:
			
				Result();
	
				Imath::V3f point() const;								
				Imath::V3f normal() const;
				Imath::V2f uv() const;
				Imath::V3f uTangent() const;
				Imath::V3f vTangent() const;		
				
				Imath::V3f          vectorPrimVar( const PrimitiveVariable &pv ) const;
				float               floatPrimVar ( const PrimitiveVariable &pv ) const;
				int                 intPrimVar   ( const PrimitiveVariable &pv ) const;
				const std::string  &stringPrimVar( const PrimitiveVariable &pv ) const;
				Imath::Color3f      colorPrimVar ( const PrimitiveVariable &pv ) const;
				half                halfPrimVar  ( const PrimitiveVariable &pv ) const;
				
			protected:
			
				Imath::V3f m_p;
							
				template<typename T>
				T getPrimVar( const PrimitiveVariable &pv ) const;
			
			
		};
		IE_CORE_DECLAREPTR( Result );		
		
		SpherePrimitiveEvaluator( ConstSpherePrimitivePtr sphere );
		
		virtual ~SpherePrimitiveEvaluator();
		
		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );
		
		virtual PrimitiveEvaluator::ResultPtr createResult() const;
		
		virtual bool closestPoint( const Imath::V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const;
		
		virtual bool pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const;
		
		virtual bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
			const PrimitiveEvaluator::ResultPtr &result, float maxDistance = Imath::limits<float>::max() ) const;
			
		virtual int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction, 
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const;
		
	protected:
	
		ConstSpherePrimitivePtr m_sphere;
};

IE_CORE_DECLAREPTR( SpherePrimitiveEvaluator );

} // namespace IECore

#endif // IE_CORE_SPHEREPRIMITIVEEVALUATOR_H
