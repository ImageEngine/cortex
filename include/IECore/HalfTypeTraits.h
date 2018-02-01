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

#ifndef IE_CORE_HALFTYPETRAITS_H
#define IE_CORE_HALFTYPETRAITS_H

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/half.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/type_traits/is_arithmetic.hpp"
#include "boost/type_traits/is_floating_point.hpp"
#include "boost/type_traits/is_signed.hpp"
#include "boost/type_traits/is_unsigned.hpp"

namespace boost
{

template<>
struct is_arithmetic<half> : public true_type{};

template<>
struct is_arithmetic<const half> : public true_type{};

template<>
struct is_arithmetic<volatile half> : public true_type{};

template<>
struct is_arithmetic<const volatile half> : public true_type{};

template<>
struct is_floating_point<half> : public true_type{};

template<>
struct is_floating_point<const half> : public true_type{};

template<>
struct is_floating_point<volatile half> : public true_type{};

template<>
struct is_floating_point<const volatile half> : public true_type{};

template<>
struct is_signed<half> : public true_type{};

template<>
struct is_signed<const half> : public true_type{};

template<>
struct is_signed<volatile half> : public true_type{};

template<>
struct is_signed<const volatile half> : public true_type{};

template<>
struct is_unsigned<half> : public false_type{};

template<>
struct is_unsigned<const half> : public false_type{};

template<>
struct is_unsigned<volatile half> : public false_type{};

template<>
struct is_unsigned<const volatile half> : public false_type{};

}

#endif // IE_CORE_HALFTYPETRAITS_H
