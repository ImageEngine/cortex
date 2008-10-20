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

#ifndef IE_CORE_RGBTOXYZCOLORTRANSFORM_H
#define IE_CORE_RGBTOXYZCOLORTRANSFORM_H

#include "IECore/ColorTransform.h"

namespace IECore
{

/// Forward declaration
template< typename, typename > class XYZToRGBColorTransform;

/// A templated ColorTransform class to perform RGB->XYZ color transformations
template<typename F, typename T>
class RGBToXYZColorTransform : public ColorTransform< F, T >
{	
	public:
	
		typedef XYZToRGBColorTransform< T, F > InverseType;
		
		/// Creates a default transform using the following xy chromacities:
		///      x     y
		/// r: 0.64, 0.33
		/// g: 0.3, 0.6 
		/// b: 0.15, 0.06 
		/// w: 0.312713, 0.329016
		RGBToXYZColorTransform();
	
		/// Creates a transform using the specified 3x3 matrix
		template< typename M >
		RGBToXYZColorTransform( const M &matrix );
		
		/// Creates a transform using the specified xy chromacities. Class "C" should
		/// be a 2d vector type compatible with IECore::VectorTraits.
		template< typename C >
		RGBToXYZColorTransform(		
			const C &rChromacity,
			const C &gChromacity,
			const C &bChromacity,
			
			const C &referenceWhite						
		);

		/// Perform the conversion
		T operator()( F f );
		
		/// Returns an instance of a class able to perform the inverse conversion
		InverseType inverse() const;
		
		/// Retrieves the matrix used to perform the transformation
		const Imath::M33f &matrix() const;
		
	protected :
	
		template< typename C >
		void setMatrix(	
			const C &rChromacity,
			const C &gChromacity,
			const C &bChromacity,
			
			const C &referenceWhite						
		);
	
	private:
	
		Imath::M33f m_matrix;
};

} // namespace IECore

#include "IECore/RGBToXYZColorTransform.inl"

#endif // IE_CORE_RGBTOXYZCOLORTRANSFORM_H
