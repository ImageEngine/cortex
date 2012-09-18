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

#ifndef IE_CORE_XYYTOXYZCOLORTRANSFORM_INL
#define IE_CORE_XYYTOXYZCOLORTRANSFORM_INL

#include <cassert>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathMath.h"

#include "IECore/Convert.h"
#include "IECore/VectorTraits.h"

#include "IECore/XYYToXYZColorTransform.h"
#include "IECore/XYZToXYYColorTransform.h"

namespace IECore
{

template<typename F, typename T>
XYYToXYZColorTransform<F, T>::XYYToXYZColorTransform()
{
	m_referenceWhite.x = 0.312713;
	m_referenceWhite.y = 0.329016;
}

template<typename F, typename T>
template<typename C>
XYYToXYZColorTransform<F, T>::XYYToXYZColorTransform(
	const C &referenceWhite
)
{
	typedef VectorTraits<C> VecTraits;
	m_referenceWhite.x = VecTraits::get( referenceWhite, 0 );
	m_referenceWhite.y = VecTraits::get( referenceWhite, 1 );
}

template<typename F, typename T>
T XYYToXYZColorTransform<F, T>::transform( const F &f )
{
	Imath::V3f xyy = IECore::convert< Imath::V3f >( f );
	if ( Imath::Math<float>::fabs( xyy.y ) <= Imath::limits<float>::epsilon() )
	{
		return IECore::convert<T>( Imath::V3f( 0, 0, 0 ) );
	}

	Imath::V3f xyz;

	xyz.x = xyy.x * xyy.z / xyy.y;
	xyz.y = xyy.z;
	xyz.z = ( 1.0 - xyy.x - xyy.y ) * xyy.z / xyy.y;

	return IECore::convert<T>( xyz );
}

template<typename F, typename T>
typename XYYToXYZColorTransform<F, T>::InverseType XYYToXYZColorTransform<F, T>::inverse() const
{
	return InverseType( m_referenceWhite );
}

} // namespace IECore

#endif // IE_CORE_XYYTOXYZCOLORTRANSFORM_INL

