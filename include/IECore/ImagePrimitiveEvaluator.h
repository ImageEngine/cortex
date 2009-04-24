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

#ifndef IE_CORE_IMAGEPRIMITIVEEVALUATOR_H
#define IE_CORE_IMAGEPRIMITIVEEVALUATOR_H

#include <vector>

#include "IECore/PrimitiveEvaluator.h"
#include "IECore/ImagePrimitive.h"

namespace IECore
{

/// An implementation of PrimitiveEvaluator to allow queries to be performed on ImagePrimitive instances
class ImagePrimitiveEvaluator : public PrimitiveEvaluator
{
	public:
	
		typedef ImagePrimitive PrimitiveType;
	
		IE_CORE_DECLARERUNTIMETYPED( ImagePrimitiveEvaluator, PrimitiveEvaluator );
					
		class Result : public PrimitiveEvaluator::Result
		{
			friend class ImagePrimitiveEvaluator;
			
			public:
				
				IE_CORE_DECLAREMEMBERPTR( Result );
				
				Result( const Imath::Box3f &bound, const Imath::Box2i &dataWindow );
				virtual ~Result();
	
				Imath::V3f point() const;								
				Imath::V3f normal() const;
				Imath::V2f uv() const;
				Imath::V3f uTangent() const;
				Imath::V3f vTangent() const;		
				
				Imath::V3f          vectorPrimVar( const PrimitiveVariable &pv ) const;
				float               floatPrimVar ( const PrimitiveVariable &pv ) const;
				int                 intPrimVar   ( const PrimitiveVariable &pv ) const;
				unsigned int        uintPrimVar  ( const PrimitiveVariable &pv ) const;
				short               shortPrimVar ( const PrimitiveVariable &pv ) const;
				unsigned short      ushortPrimVar( const PrimitiveVariable &pv ) const;
				char                charPrimVar  ( const PrimitiveVariable &pv ) const;
				unsigned char       ucharPrimVar ( const PrimitiveVariable &pv ) const;
				const std::string  &stringPrimVar( const PrimitiveVariable &pv ) const;
				Imath::Color3f      colorPrimVar ( const PrimitiveVariable &pv ) const;
				half                halfPrimVar  ( const PrimitiveVariable &pv ) const;
				
				Imath::V2i pixel() const;
				
			protected:
			
				Imath::Box3f m_bound;
				Imath::V3f m_p;
				Imath::Box2i m_dataWindow;
			
				template<typename T>
				T getPrimVar( const PrimitiveVariable &pv ) const;
				
				template<typename T>
				T indexData( const std::vector<T> &data, const Imath::V2i &p ) const;
			
		};
		IE_CORE_DECLAREPTR( Result );		
		
		static PrimitiveEvaluatorPtr create( ConstPrimitivePtr primitive );
		
		ImagePrimitiveEvaluator( ConstImagePrimitivePtr image );
		
		virtual ~ImagePrimitiveEvaluator();
		
		virtual ConstPrimitivePtr primitive() const;
		
		virtual PrimitiveEvaluator::ResultPtr createResult() const;
		
		virtual void validateResult( const PrimitiveEvaluator::ResultPtr &result ) const;	
						
		virtual bool closestPoint( const Imath::V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const;
		
		virtual bool pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const;
		
		/// Returns the object-space point at the center of the specified pixel
		virtual bool pointAtPixel( const Imath::V2i &pixel, const PrimitiveEvaluator::ResultPtr &result ) const;
		
		virtual bool intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
			const PrimitiveEvaluator::ResultPtr &result, float maxDistance = Imath::limits<float>::max() ) const;
			
		virtual int intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction, 
			std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance = Imath::limits<float>::max() ) const;
			
		virtual float volume() const;	

		virtual Imath::V3f centerOfGravity() const;
		
		virtual float surfaceArea() const;
		
		/// Returns the "R" (red) channel of the image, if available. Otherwise returns the variables' "end" iterator.
		PrimitiveVariableMap::const_iterator R() const;
		
		/// Returns the "G" (green) channel of the image, if available. Otherwise returns the variables' "end" iterator.		
		PrimitiveVariableMap::const_iterator G() const;
		
		/// Returns the "B" (blue) channel of the image, if available. Otherwise returns the variables' "end" iterator.		
		PrimitiveVariableMap::const_iterator B() const;
		
		/// Returns the "A" (alpha) channel of the image, if available. Otherwise returns the variables' "end" iterator.		
		PrimitiveVariableMap::const_iterator A() const;
		
		/// Returns the "Y" (luminance) channel of the image, if available. Otherwise returns the variables' "end" iterator.		
		PrimitiveVariableMap::const_iterator Y() const;
		
		
					
	protected:
	
		ConstImagePrimitivePtr m_image;
				
};

IE_CORE_DECLAREPTR( ImagePrimitiveEvaluator );

} // namespace IECore

#endif // IE_CORE_IMAGEPRIMITIVEEVALUATOR_H
