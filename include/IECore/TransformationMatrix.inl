//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#ifndef IE_CORE_TRANSFORMATIONMATRIX_INL
#define IE_CORE_TRANSFORMATIONMATRIX_INL

template <class T>
TransformationMatrix<T>::TransformationMatrix() : scalePivot( 0, 0, 0), scale( 1, 1, 1 ), shear( 0, 0, 0 ), scalePivotTranslation( 0, 0, 0 ), 
						rotatePivot( 0, 0, 0 ), rotationOrientation(), rotate(), rotatePivotTranslation( 0, 0, 0 ), translate( 0, 0, 0 )
{
}

template <class T>
TransformationMatrix<T>::TransformationMatrix( const Imath::Vec3< T > s, const Imath::Euler< T > r, const Imath::Vec3< T > t ) :
		scalePivot( 0, 0, 0), scale( s ), shear( 0, 0, 0 ), scalePivotTranslation( 0, 0, 0 ), 
						rotatePivot( 0, 0, 0 ), rotationOrientation(), rotate( r ), rotatePivotTranslation( 0, 0, 0 ), translate( t )
{
}

template <class T>
TransformationMatrix<T>::TransformationMatrix( const TransformationMatrix &cp ) :
		scalePivot( cp.scalePivot ), scale( cp.scale ), shear( cp.shear ), scalePivotTranslation( cp.scalePivotTranslation ), 
						rotatePivot( cp.rotatePivot ), rotationOrientation( cp.rotationOrientation ), rotate( cp.rotate ),
						rotatePivotTranslation( cp.rotatePivotTranslation ), translate( cp.translate )
{
}

template <class T>
Imath::Matrix44<T> TransformationMatrix<T>::transform( ) const
{
	return Imath::Matrix44<T>().setTranslation( -scalePivot ) * Imath::Matrix44<T>().setScale( scale ) * Imath::Matrix44<T>().setShear( shear ) * Imath::Matrix44<T>().setTranslation( scalePivot + scalePivotTranslation - rotatePivot ) * rotationOrientation.normalized().toMatrix44() * rotate.toMatrix44() * Imath::Matrix44<T>().setTranslation( rotatePivot + rotatePivotTranslation + translate );
}

template <class T>
bool TransformationMatrix<T>::operator == (const TransformationMatrix &t) const
{
	return ( scalePivot == t.scalePivot && scale == t.scale && shear == t.shear && scalePivotTranslation == t.scalePivotTranslation &&
			rotatePivot == t.rotatePivot && rotationOrientation.normalized() == t.rotationOrientation.normalized() && 
			rotate == t.rotate && rotate.order() == t.rotate.order() &&	rotatePivotTranslation == t.rotatePivotTranslation && translate == t.translate );
}

template<class T>
std::ostream &operator << ( std::ostream &os, const TransformationMatrix<T> &x )
{
	os << "sp : " << x.scalePivot << " s : " << x.scale << " sh " << x.shear << " spt " << x.scalePivotTranslation << " rp " << x.rotatePivot << 
		" ro : " << x.rotationOrientation << " r : " << x.rotate << " rpt : " << x.rotatePivotTranslation << " t : " << x.translate;
	return os;
}

#endif // IE_CORE_TRANSFORMATIONMATRIX_INL
