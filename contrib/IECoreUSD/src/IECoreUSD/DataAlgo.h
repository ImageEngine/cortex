//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#ifndef IECOREUSD_DATAALGO_H
#define IECOREUSD_DATAALGO_H

#include "IECoreUSD/TypeTraits.h"

#include "IECore/Data.h"
#include "IECore/TypedData.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/vt/value.h"
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/usd/attribute.h"

namespace IECoreUSD
{

namespace DataAlgo
{

/// From USD to Cortex
/// ==================

/// Conversion of any type for which a USDTypeTraits
/// specialisation exists, e.g. pxr::GfVec3f -> Imath::V3f.
template<typename T>
typename USDTypeTraits<T>::CortexType fromUSD( const T &value );

/// Converts USD `array` to Cortex VectorData.
template<typename T>
typename USDTypeTraits<T>::CortexVectorDataType::Ptr fromUSD( const pxr::VtArray<T> &array );

/// Converts USD `value` to Cortex Data, applying any additional
/// geometric interpretation implied by `valueTypeName`. Returns nullptr
/// if no appropriate conversion exists.
IECore::DataPtr fromUSD( const pxr::VtValue &value, const pxr::SdfValueTypeName &valueTypeName );

/// Converts the value of `attribute` at the specified time, using the attribute's
/// type name to apply geometric interpretation.
IECore::DataPtr fromUSD( const pxr::UsdAttribute &attribute, pxr::UsdTimeCode time=pxr::UsdTimeCode::Default() );

/// From Cortex to USD
/// ==================

/// Conversion of any type for which a CortexTypeTraits
/// specialisation exists, e.g. Imath::V3f -> pxr::GfVec3f.
template<typename T>
typename CortexTypeTraits<T>::USDType toUSD( const T &value, typename std::enable_if_t<!std::is_void<typename CortexTypeTraits<T>::USDType>::value> *enabler = nullptr );

/// Conversion of any supported data type to a generic VtValue.
/// Returns an empty VtValue if no conversion is available.
pxr::VtValue toUSD( const IECore::Data *data );

/// Returns the Sdf type for `data`. This augments the type of
/// the VtValue returned by `toUSD( data )`. For example, `toUSD()`
/// might return a plain `GfVec3f` while `valueTypeName()` might
/// return `Sdf_ValueTypeNamesType->Point3f`.
pxr::SdfValueTypeName valueTypeName( const IECore::Data *data );

} // namespace DataAlgo

} // namespace IECoreUSD

#include "DataAlgo.inl"

#endif // IECOREUSD_DATAALGO_H
