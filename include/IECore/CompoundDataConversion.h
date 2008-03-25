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

#ifndef IE_CORE_COMPOUNDDATACONVERSION_H
#define IE_CORE_COMPOUNDDATACONVERSION_H

#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"

#include "IECore/DataConversion.h"

namespace IECore
{

/// Performs the conversion "to = C2(C1(from))". Note that the functions are applied in the same order as
/// specified in the template argument list.
template<typename C1, typename C2>
class CompoundDataConversion : public DataConversion< typename C1::FromType, typename C2::ToType >
{
	public:
		/// These two types must be the same, so that the function composition works
		BOOST_STATIC_ASSERT( (boost::is_same< typename C1::ToType, typename C2::FromType >::value) );
		
		/// Inverse defined by the equality: (f o g)'(x) = ( g' o f' )(x)		
		typedef CompoundDataConversion< typename C2::InverseType, typename C1::InverseType > InverseType;
	
		/// Instantiate a conversion using the default constructors for C1 and C2
		CompoundDataConversion();
		
		/// Instantiate a conversion using given instances of C1 and C2
		CompoundDataConversion( const C1 &c1, const C2 &c2 );

		/// Perform the conversion
		typename CompoundDataConversion<C1, C2>::ToType operator()( typename CompoundDataConversion<C1, C2>::FromType f );
	
	protected:
	
		C1 m_c1;
		C2 m_c2;
};

} // namespace IECore

#include "CompoundDataConversion.inl"

#endif // IE_CORE_COMPOUNDDATACONVERSION_H
