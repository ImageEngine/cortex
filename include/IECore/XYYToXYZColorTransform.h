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

#ifndef IE_CORE_XYYTOXYZCOLORTRANSFORM_H
#define IE_CORE_XYYTOXYZCOLORTRANSFORM_H

#include "IECore/ColorTransform.h"

namespace IECore
{

/// Forward declaration
template< typename, typename > class XYZToXYYColorTransform;

/// A templated ColorTransform class to perform xyY->XYZ color transformations
template<typename F, typename T>
class XYYToXYZColorTransform : public ColorTransform< F, T >
{	
	public:
		typedef XYZToXYYColorTransform< T, F > InverseType;
	
		/// Creates a default transform using the following whitepoint xy chromacity
		/// w: 0.312713, 0.329016	
		/// This white point is only used if you need the inverse transform.
		XYYToXYZColorTransform();
	
		/// Creates a transform using the specified xy whitepoint chromacity (which is only ever used
		/// if you need the inverse transform, too). Class "C" should be a 2d vector type compatible with IECore::VectorTraits.
		template< typename C >
		XYYToXYZColorTransform(		
			const C &referenceWhite						
		);

		/// Perform the conversion
		virtual T transform( const F &f );
		
		/// Returns an instance of a class able to perform the inverse conversion
		InverseType inverse() const;
	
	private:
	
		Imath::V2f m_referenceWhite;
};

} // namespace IECore

#include "IECore/XYYToXYZColorTransform.inl"

#endif // IE_CORE_XYYTOXYZCOLORTRANSFORM_H
