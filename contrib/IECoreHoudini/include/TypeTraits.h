//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TYPETRAITS_H
#define IECOREHOUDINI_TYPETRAITS_H

#include "IECore/TypeTraits.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

namespace IECoreHoudini
{

namespace TypeTraits
{

/// IsDetailGbAttribFloatTypedData
template<typename T> struct IsDetailGbAttribFloatTypedData : public boost::false_type {};
template<> struct IsDetailGbAttribFloatTypedData< IECore::FloatData > : public boost::true_type {};
template<> struct IsDetailGbAttribFloatTypedData< IECore::V2fData > : public boost::true_type {};
template<> struct IsDetailGbAttribFloatTypedData< IECore::V3fData > : public boost::true_type {};
template<> struct IsDetailGbAttribFloatTypedData< IECore::Color3fData > : public boost::true_type {};

/// IsDetailGbAttribIntTypedData
template<typename T> struct IsDetailGbAttribIntTypedData : public boost::false_type {};
template<> struct IsDetailGbAttribIntTypedData< IECore::IntData > : public boost::true_type {};
template<> struct IsDetailGbAttribIntTypedData< IECore::V2iData > : public boost::true_type {};
template<> struct IsDetailGbAttribIntTypedData< IECore::V3iData > : public boost::true_type {};

/// IsVectorGbAttribFloatTypedData
template<typename T> struct IsVectorGbAttribFloatTypedData : public boost::false_type {};
template<> struct IsVectorGbAttribFloatTypedData< IECore::FloatVectorData > : public boost::true_type {};
template<> struct IsVectorGbAttribFloatTypedData< IECore::V2fVectorData > : public boost::true_type {};
template<> struct IsVectorGbAttribFloatTypedData< IECore::V3fVectorData > : public boost::true_type {};
template<> struct IsVectorGbAttribFloatTypedData< IECore::Color3fVectorData > : public boost::true_type {};

/// IsVectorGbAttribIntTypedData
template<typename T> struct IsVectorGbAttribIntTypedData : public boost::false_type {};
template<> struct IsVectorGbAttribIntTypedData< IECore::IntVectorData > : public boost::true_type {};
template<> struct IsVectorGbAttribIntTypedData< IECore::V2iVectorData > : public boost::true_type {};
template<> struct IsVectorGbAttribIntTypedData< IECore::V3iVectorData > : public boost::true_type {};

/// IsDetailGbAttribTypedData
template<typename T> struct IsDetailGbAttribTypedData : boost::mpl::or_< IsDetailGbAttribFloatTypedData<T>, IsDetailGbAttribIntTypedData<T> > {};

/// IsVectorGbAttribTypedData
template<typename T> struct IsVectorGbAttribTypedData : boost::mpl::or_< IsVectorGbAttribFloatTypedData<T>, IsVectorGbAttribIntTypedData<T> > {};

} // namespace TypeTraits

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TYPETRAITS_H
