//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef SPHERICALHARMONICSROTATIONMATRIX_H
#define SPHERICALHARMONICSROTATIONMATRIX_H

#include "OpenEXR/ImathEuler.h"
#include "IECore/SphericalHarmonics.h"

namespace IECore
{

/*
* Rotation class for Spherical Harmonics.
* Implements complex spherical harmonics rotation as described in 
* Choi, Cheol Ho et al, "Rapid and stable determination of rotation
* matrices between spherical harmonics by direct recursion", J. Chem.
* Phys. Vol 111, No. 19, 1999, pp 8825-8831.
*
* In order to actually work, I've changed the following equations: 
* Equation 8.7: I used transposed W instead of the hermitian conjugate of W. That's the only way that 
*     the solution for R at equation 8.15 can be computed from F and G at 5.4-5.5 and 8.4-8.7. 
* Equation 8.10: looks wrong because it always compute the same value for m!=0. 
*     Figuring out the correct values for alpha and beta that fit the equations 8.11-8.14 is hard.
*     Instead, I've reapplied equation 8.7 and found the following relationships that substitutes 8.11-8.14:
*     i=0,j=0: R(i,j) = F(0,0) + i*G(0,0) ( G(0,0) is always zero )
*     i>0,j>0 or i<0,j<0: R(i,j) = sign(j)*((-1)^|j|)*F(-|i|,|j| + F(-|i|,-|j|)
*     i<0,j>0 or i>0,j<0: R(i,j) = ((-1)^|j|*G(-|i|,|j|) + sign(j)*G(-|i|,-|j|)
*     i=0,j>0 or i>0,j=0: R(i,j) = sqrt(2)*F(-|i|,-|j|)
*     i=0,j<0 or i<0,j=0: R(i,j) = sign(j+.5)*sqrt(2)*G(-|i|,-|j|)
* Lucio Moser March 2009
*
* \todo Current implementation does not take advantage of the matrix sparsity.
*/
template < typename V >
class SphericalHarmonicsRotationMatrix
{
	public:

		typedef V BaseType;

		SphericalHarmonicsRotationMatrix();

		// Constructs a SH rotation matrix for the given X,Y,Z rotation (in radians).
		template <class S>
		SphericalHarmonicsRotationMatrix( const Imath::Vec3<S>& r );

	    // Set matrix to rotation by XYZ euler angles (in radians)
		template <class S>
		void setEulerAngles( const Imath::Vec3<S>& r );

		// Set matrix to rotation around given axis by given angle
		template <class S>
		void setAxisAngle ( const Imath::Vec3<S>& ax, S ang);

		// Set matrix to rotation by a given quaternion.
		template <class S>
		void setQuaternion( const Imath::Quat<S>& q );

		// Set matrix to rotation by a given rotation matrix
		template <class S>
		void setRotation( const Imath::Matrix44<S>& m );

		// Return the 3D rotation as a matrix.
		const Imath::Matrix44< V > rotation() const
		{
			return m_3dRotation;
		}

		// Applies the rotation to the given SphericalHarmonics object.
		template< typename U >
		void transform( SphericalHarmonics<U> &sh ) const;

	private:

		mutable unsigned int m_bands;
		Imath::Matrix44< V > m_3dRotation;
		mutable std::vector< V > m_squareRoots;
		mutable std::vector< std::vector<V> > m_R;
		mutable std::vector< std::vector<V> > m_F;	
		mutable std::vector< std::vector<V> > m_G;
		mutable bool m_newRotation;

		// compute coefficient a from equation 6.2
		V compute_a( unsigned int l, int m1, int m2 ) const;
		// compute coefficient b from equation 6.3
		V compute_b( unsigned int l, int m1, int m2 ) const;
		// compute coefficient c from equation 6.10
		V compute_c( unsigned int l, int m1, int m2 ) const;
		// compute coefficient d from equation 6.11
		V compute_d( unsigned int l, int m1, int m2 ) const;
		// compute coefficients H and K from equations 7.1 and 7.2
		void computeHK( unsigned int band, int m1, int m2, int i, int j, V &H, V &K ) const;
		// used for all cases |m2| <> band. Equations 7.3 and 7.4
		// updates Fm1,m2 F-m1,-m2, Gm1,m2 and G-m1,-m2
		void computeRecurrenceRelation1( unsigned int band, int m1, int m2 ) const;
		// used for case m2 = -band. Equations 7.5 and 7.6
		// updates Fm1,m2 F-m1,-m2, Gm1,m2 and G-m1,-m2
		void computeRecurrenceRelation2( unsigned int band, int m1, int m2 ) const;
		// used for case m2 = band. Equations 7.7 and 7.8
		// updates Fm1,m2 F-m1,-m2, Gm1,m2 and G-m1,-m2
		void computeRecurrenceRelation3( unsigned int band, int m1, int m2 ) const;

		// utility function to get values from the real harmonics rotation matrix.
		V F( unsigned int band, int i, int j ) const;
		// utility function to get values from the complex harmonics rotation matrix.
		V G( unsigned int band, int i, int j ) const;
		// utility function to set values on the real harmonics rotation matrix.
		void setF( unsigned int band, int i, int j, V value ) const;
		// utility function to set values on the complex harmonics rotation matrix.
		void setG( unsigned int band, int i, int j, V value ) const;
		// utility function to set the pair F and G and use the simetries to set the opposite values
		// according to equations 5.8
		void setFG( unsigned int band, int m1, int m2, V realValue, V complexValue ) const;

		void computeRealRotation( ) const;
		void computeComplexRotation( ) const;
		void computeSquareRoots( ) const;

		template< typename U >
		void applyRotation( SphericalHarmonics<U> &sh ) const;

		// returns the offset in columns or rows for the center of the submatrix for the given band.
		static int bandMatrixOffset( int band );
		// function to read submatrices cells for a given band. m1 and m2 can be any value from [-band,band]
		static V bandMatrixValue( const std::vector< std::vector<V> > &mat, int band, int m1, int m2 );
		// function to write submatrices cells for a given band. m1 and m2 can be any value from [-band,band]
		static void setBandMatrixValue( std::vector< std::vector<V> > &mat, int band, int m1, int m2, V value );

};

// Define multiplication of a SphericalHarmonics by the rotation matrix.
template <class S, class T>
const SphericalHarmonics<S> & operator *= (SphericalHarmonics<S> &sh, const SphericalHarmonicsRotationMatrix<T> &m)
{
	m.transform( sh );
	return sh;
}

typedef SphericalHarmonicsRotationMatrix<float> SHRotationf;
typedef SphericalHarmonicsRotationMatrix<double> SHRotationd;

} // namespace IECore

#include "IECore/SphericalHarmonicsRotationMatrix.inl"

#endif // SPHERICALHARMONICSROTATIONMATRIX_H
