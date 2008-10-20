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

#ifndef IE_CORE_RGBTOXYZCOLORTRANSFORM_INL
#define IE_CORE_RGBTOXYZCOLORTRANSFORM_INL

#include <cassert>

#include "OpenEXR/ImathVec.h"

#include "IECore/Convert.h"
#include "IECore/VectorTraits.h"

namespace IECore
{

template<typename F, typename T>
RGBToXYZColorTransform<F, T>::RGBToXYZColorTransform() 
{	
	setMatrix( 
		Imath::V2f( 0.64, 0.33 ),
		Imath::V2f( 0.3, 0.6 ),
		Imath::V2f( 0.15, 0.06 ),
		Imath::V2f( 0.312713, 0.329016 )

	);
}

template<typename F, typename T>
template<typename M>
RGBToXYZColorTransform<F, T>::RGBToXYZColorTransform( const M &matrix ) 
{
	m_matrix = convert<Imath::M33f>( matrix );
}

template<typename F, typename T>
template<typename C>
RGBToXYZColorTransform<F, T>::RGBToXYZColorTransform(		
	const C &rChromacity,
	const C &gChromacity,
	const C &bChromacity,

	const C &referenceWhite						
)
{
	setMatrix( rChromacity, gChromacity, bChromacity, referenceWhite );
}

template<typename F, typename T>
template<typename C>
void RGBToXYZColorTransform<F, T>::setMatrix(		
	const C &rChromacity,
	const C &gChromacity,
	const C &bChromacity,

	const C &referenceWhite						
)
{
	BOOST_STATIC_ASSERT( ( TypeTraits::IsVec2<C>::value ) );

	typedef VectorTraits<C> VecTraits;
	
	/// \todo Implement this in terms of an xyY->XYZ converter
	float xr = VecTraits::get( rChromacity, 0 );
	float yr = VecTraits::get( rChromacity, 1 );
	
	float xg = VecTraits::get( gChromacity, 0 );
	float yg = VecTraits::get( gChromacity, 1 );
	
	float xb = VecTraits::get( bChromacity, 0 );
	float yb = VecTraits::get( bChromacity, 1 );
	
	float xw = VecTraits::get( referenceWhite, 0 );
	float yw = VecTraits::get( referenceWhite, 1 );
				
	float Xr = xr / yr;
	float Yr = 1.0;
	float Zr = ( 1.0 - xr - yr ) / yr;
	
	float Xg = xg / yg;
	float Yg = 1.0;
	float Zg = ( 1.0 - xg - yg ) / yg;
	
	float Xb = xb / yb;
	float Yb = 1.0;
	float Zb = ( 1.0 - xb - yb ) / yb;
	
	float Xw = xw / yw;
	float Yw = 1.0;
	float Zw = ( 1.0 - xw - yw ) / yw;	
	
	Imath::M33f m = Imath::M33f
		(
			Xr, Yr, Zr,
			Xg, Yg, Zg,
			Xb, Yb, Zb
		).inverse();
		
	Imath::V3f rw( Xw, Yw, Zw );
	
	Imath::V3f s = rw * m;
	
	float Sr = s.x;	
	float Sg = s.y;	
	float Sb = s.z;			
	
	m_matrix = Imath::M33f
	(
		Sr * Xr, Sr * Yr, Sr * Zr,
		Sg * Xg, Sg * Yg, Sg * Zg,
		Sb * Xb, Sb * Yb, Sb * Zb
	);

}		

template<typename F, typename T>
T RGBToXYZColorTransform<F, T>::operator()( F f )
{
	Imath::V3f from = IECore::convert< Imath::V3f >( f );
	
	return IECore::convert<T>( from * m_matrix );		
}

template<typename F, typename T>
typename RGBToXYZColorTransform<F, T>::InverseType RGBToXYZColorTransform<F, T>::inverse() const
{
	return InverseType( m_matrix.inverse() );
}

template<typename F, typename T>
const Imath::M33f &RGBToXYZColorTransform<F, T>::matrix() const
{
	return m_matrix;
}

} // namespace IECore

#endif // IE_CORE_RGBTOXYZCOLORTRANSFORM_INL

