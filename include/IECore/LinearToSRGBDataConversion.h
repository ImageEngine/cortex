//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_LINEARTOSRGBDATACONVERSION_H
#define IE_CORE_LINEARTOSRGBDATACONVERSION_H

#include "boost/static_assert.hpp"
#include "boost/type_traits/is_floating_point.hpp"

#include "IECore/HalfTypeTraits.h"
#include "IECore/DataConversion.h"

namespace IECore
{

/// Forward declaration
template<typename, typename> class SRGBToLinearDataConversion;

/// A class to perform data conversion from linear values to sRGB
template<typename F, typename T>
struct LinearToSRGBDataConversion : public DataConversion< F, T >
{
	BOOST_STATIC_ASSERT( boost::is_floating_point< F >::value );
	BOOST_STATIC_ASSERT( boost::is_floating_point< T >::value );

	typedef SRGBToLinearDataConversion<T, F> InverseType;

	/// Perform the conversion
	T operator()( F f ) const;

	/// Returns an instance of a class able to perform the inverse conversion
	InverseType inverse() const;
};

} // namespace IECore

#include "IECore/LinearToSRGBDataConversion.inl"

#endif // IE_CORE_LINEARTOSRGBDATACONVERSION_H
