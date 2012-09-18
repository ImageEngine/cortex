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

#ifndef IE_COREMAYA_TYPETRAITS_H
#define IE_COREMAYA_TYPETRAITS_H

#include "IECore/TypeTraits.h"

namespace IECoreMaya
{

namespace TypeTraits
{

/// IsUnit
template<typename T> struct IsUnit : public boost::false_type {};
template<> struct IsUnit< MAngle > : public boost::true_type {};
template<> struct IsUnit< MTime > : public boost::true_type {};
template<> struct IsUnit< MDistance > : public boost::true_type {};

BOOST_STATIC_ASSERT( (IsUnit<MTime>::value) );
BOOST_STATIC_ASSERT( (IsUnit<MAngle>::value) );
BOOST_STATIC_ASSERT( (IsUnit<MDistance>::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsUnit< float > >::value) );

} // namespace TypeTraits

} // namespace IECoreMaya

#endif // IE_COREMAYA_TYPETRAITS_H
