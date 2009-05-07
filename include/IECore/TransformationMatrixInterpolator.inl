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


// Partially specialize for TransformationMatrix
// Assumes the two rotate members have the same rotation order and that they are sufficiently close 
// to each other so that the euler interpolation will look good.
template<typename T>
struct LinearInterpolator< TransformationMatrix<T> >
{
	void operator()(const TransformationMatrix<T> &y0, 
			const TransformationMatrix<T> &y1,
			double x, 
			TransformationMatrix<T> &result) const
	{	
		LinearInterpolator< Imath::Vec3< T > >()( y0.scalePivot, y1.scalePivot, x, result.scalePivot );
		LinearInterpolator< Imath::Vec3< T > >()( y0.scale, y1.scale, x, result.scale );
		LinearInterpolator< Imath::Vec3< T > >()( y0.shear, y1.shear, x, result.shear );
		LinearInterpolator< Imath::Vec3< T > >()( y0.scalePivotTranslation, y1.scalePivotTranslation, x, result.scalePivotTranslation );
		LinearInterpolator< Imath::Vec3< T > >()( y0.rotatePivot, y1.rotatePivot, x, result.rotatePivot );
		LinearInterpolator< Imath::Quat< T > >()( y0.rotationOrientation, y1.rotationOrientation, x, result.rotationOrientation );
			
		/// \todo We're doing the interpolation using quaternions so we get the shortest rotation, but this is
		/// throwing away the continuity that the eulers provide. we need to give some more thought to how we do this.
		/// this code is currently primarily used as part of the following process :
		///
		/// 1) We take a maya dag node and extract the transform using an IECoreMaya::FromMayaTransformConverter with
		/// space set to world and with the euler filter on.
		/// 2 ) We put the transform in an AttributeCache.
		/// 3 ) We read the cache using an InterpolatedCache, to get accurate positions at subframe locations for
		/// specifying motion blocks to 3delight. Those positions are given to the renderer as simple matrices.
		/// The InterpolatedCache uses this function to perform the interpolation.
		///
		/// As part of this process the interpolation here makes sense. It gives us nicely interpolated values and
		/// we don't care about the loss of continuity as that's thrown away by going to matrix form anyway. However
		/// in other cases we might prefer to keep the continuity provided by just naively interpolating euler values - for
		/// instance if we don't intend to go to a matrix form at the end.
		///
		/// We should review the following :
		/// 
		///		* What members the TransformationMatrix has and why, and how this relates to
		/// 	  the maya MTransformationMatrix class. Do we really need all those pivots and offsets?
		///		* What we cache and how for transformation caches. Eulers? Quaternions? World space? Local space with hierarchies?
		///		* How those caches are used in renderman, nuke and elsewhere.
		///		* What should be in cortex and what shouldn't.
		
		Imath::Quat<T> q0 = y0.rotate.toQuat();
		Imath::Quat<T> q1 = y1.rotate.toQuat();
		Imath::Quat<T> q; LinearInterpolator<Imath::Quat<T> >()( q0, q1, x, q );
		result.rotate.extract( q );		
		
		LinearInterpolator< Imath::Vec3< T > >()( y0.rotatePivotTranslation, y1.rotatePivotTranslation, x, result.rotatePivotTranslation );
		LinearInterpolator< Imath::Vec3< T > >()( y0.translate, y1.translate, x, result.translate );
	}
};

// Partially specialize for TransformationMatrix
// Assumes the two rotate members have the same rotation order and that they are sufficiently close 
// to each other so that the euler interpolation will look good.
template<typename T>
struct CubicInterpolator< TransformationMatrix<T> >
{
	void operator()(const TransformationMatrix<T> &y0, 
			const TransformationMatrix<T> &y1,
			const TransformationMatrix<T> &y2,
			const TransformationMatrix<T> &y3,
			double x, 
			TransformationMatrix<T> &result) const
	{
		CubicInterpolator< Imath::Vec3< T > >()( y0.scalePivot, y1.scalePivot, y2.scalePivot, y3.scalePivot, x, result.scalePivot );
		CubicInterpolator< Imath::Vec3< T > >()( y0.scale, y1.scale, y2.scale, y3.scale, x, result.scale );
		CubicInterpolator< Imath::Vec3< T > >()( y0.shear, y1.shear, y2.shear, y3.shear, x, result.shear );
		CubicInterpolator< Imath::Vec3< T > >()( y0.scalePivotTranslation, y1.scalePivotTranslation, y2.scalePivotTranslation, y3.scalePivotTranslation, x, result.scalePivotTranslation );
		CubicInterpolator< Imath::Vec3< T > >()( y0.rotatePivot, y1.rotatePivot, y2.rotatePivot, y3.rotatePivot, x, result.rotatePivot );
		CubicInterpolator< Imath::Quat< T > >()( y0.rotationOrientation, y1.rotationOrientation, y2.rotationOrientation, y3.rotationOrientation, x, result.rotationOrientation );
		CubicInterpolator< Imath::Vec3< T > >()( y0.rotate, y1.rotate, y2.rotate, y3.rotate, x, result.rotate );
		CubicInterpolator< Imath::Vec3< T > >()( y0.rotatePivotTranslation, y1.rotatePivotTranslation, y2.rotatePivotTranslation, y3.rotatePivotTranslation, x, result.rotatePivotTranslation );
		CubicInterpolator< Imath::Vec3< T > >()( y0.translate, y1.translate, y2.translate, y3.translate, x, result.translate );
	}
};
