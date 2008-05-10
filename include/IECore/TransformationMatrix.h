//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TRANSFORMATIONMATRIX_H
#define IE_CORE_TRANSFORMATIONMATRIX_H

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathEuler.h"

namespace IECore
{

/// Defines a transformation matrix that supports scale and rotation pivots in the same way Maya defines MTransformationMatrix.
// Provides direct access to the transformation components and a utility function that builds the final matrix. The matrix is built
// with the following operations:
// translate( -scalePivot ) * scale( scale ) * shear( shear ) * translate( scalePivot ) * translate( scalePivotTranslation ) * translate( -rotatePivot ) * rotate( rotationOrientation ) * rotate( rotate ) * translate( rotatePivot ) * translate( rotatePivotTranslation) * translate( translate )
// \todo add more utility methods.

template< class T >
class TransformationMatrix
{
	public:

		Imath::Vec3< T > scalePivot;
		Imath::Vec3< T > scale;
		Imath::Vec3< T > shear;
		Imath::Vec3< T > scalePivotTranslation;
		Imath::Vec3< T > rotatePivot;
		Imath::Quat< T > rotationOrientation;
		Imath::Euler< T > rotate;
		Imath::Vec3< T > rotatePivotTranslation;
		Imath::Vec3< T > translate;

		/// Default constructor sets to identity transformation.
		TransformationMatrix();

		/// Basic constructor for setting common parameters: scale, rotate and translate.
		TransformationMatrix( const Imath::Vec3< T > s, const Imath::Euler< T > r, const Imath::Vec3< T > t );

		/// Copy constructor
		TransformationMatrix( const TransformationMatrix &cp );

		/// Returns the transform this object represents.
		Imath::Matrix44<T> transform( ) const;

		bool operator == (const TransformationMatrix &t) const;
		
};

typedef TransformationMatrix<double> TransformationMatrixd;
typedef TransformationMatrix<float> TransformationMatrixf;

#include "IECore/TransformationMatrix.inl"

}

#endif // IE_CORE_TRANSFORMATIONMATRIX_H
