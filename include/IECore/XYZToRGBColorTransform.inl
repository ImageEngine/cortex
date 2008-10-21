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

#ifndef IE_CORE_XYZTORGBCOLORTRANSFORM_INL
#define IE_CORE_XYZTORGBCOLORTRANSFORM_INL

#include <cassert>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/Convert.h"
#include "IECore/VectorTraits.h"
#include "IECore/RGBToXYZColorTransform.h"

namespace IECore
{

template<typename F, typename T>
XYZToRGBColorTransform<F, T>::XYZToRGBColorTransform() 
{	
	InverseType t;
	
	m_matrix = t.matrix().inverse();

}

template<typename F, typename T>
template<typename M>
XYZToRGBColorTransform<F, T>::XYZToRGBColorTransform( const M &matrix ) 
{
	m_matrix = IECore::convert<Imath::M33f>( matrix );
}

template<typename F, typename T>
template<typename C>
XYZToRGBColorTransform<F, T>::XYZToRGBColorTransform(		
	const C &rChromacity,
	const C &gChromacity,
	const C &bChromacity,

	const C &referenceWhite						
)
{
	InverseType t( rChromacity, gChromacity, bChromacity, referenceWhite );
	
	m_matrix = t.matrix().inverse();
}

template<typename F, typename T>
T XYZToRGBColorTransform<F, T>::transform( const F &f )
{
	Imath::V3f from = IECore::convert< Imath::V3f >( f );
	assert( from.x >= -Imath::limits<float>::epsilon() );
	assert( from.y >= -Imath::limits<float>::epsilon() );
	assert( from.z >= -Imath::limits<float>::epsilon() );		
	
	assert( from.x <= 1.0f + Imath::limits<float>::epsilon() );
	assert( from.y <= 1.0f + Imath::limits<float>::epsilon() );
	assert( from.z <= 1.0f + Imath::limits<float>::epsilon() );		
	
	return IECore::convert<T>( from * m_matrix );		
}

template<typename F, typename T>
typename XYZToRGBColorTransform<F, T>::InverseType XYZToRGBColorTransform<F, T>::inverse() const
{
	return InverseType( m_matrix.inverse() );
}

template<typename F, typename T>
const Imath::M33f &XYZToRGBColorTransform<F, T>::matrix() const
{
	return m_matrix;
}

} // namespace IECore

#endif // IE_CORE_XYZTORGBCOLORTRANSFORM_INL

