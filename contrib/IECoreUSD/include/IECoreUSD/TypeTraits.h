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

#ifndef IECOREUSD_TYPETRAITS_H
#define IECOREUSD_TYPETRAITS_H

#include "IECore/GeometricTypedData.h"
#include "IECore/InternedString.h"
#include "IECore/TypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/vt/array.h"
#include "pxr/base/vt/value.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "OpenEXR/half.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"

namespace IECoreUSD
{

template<typename T>
struct CortexTypeTraits
{
	using USDType = void;
	static const bool BitwiseEquivalent = false;
};

template<typename T>
struct USDTypeTraits
{
	using CortexType = void;
	using CortexDataType = void;
	using CortexVectorDataType = void;
	static const bool BitwiseEquivalent = false;
};

#define IECOREUSD_CORTEXTYPETRAITS_SPECIALISATION( CORTEXTYPE, USDTYPE, BITWISE_EQUIVALENT ) \
	template<> \
	struct CortexTypeTraits<CORTEXTYPE> \
	{ \
		using USDType = USDTYPE; \
		static const bool BitwiseEquivalent = BITWISE_EQUIVALENT; \
	}; \

#define IECOREUSD_USDTYPETRAITS_SPECIALISATION( CORTEXTYPE, USDTYPE, BITWISE_EQUIVALENT, CORTEXDATATYPE ) \
	template<> \
	struct USDTypeTraits<USDTYPE> \
	{ \
		using CortexType = CORTEXTYPE; \
		using CortexDataType = CORTEXDATATYPE<CortexType>; \
		using CortexVectorDataType = CORTEXDATATYPE<std::vector<CortexType>>; \
		static const bool BitwiseEquivalent = BITWISE_EQUIVALENT; \
	};

#define IECOREUSD_TYPETRAITS_SPECIALISATION( CORTEXTYPE, USDTYPE, BITWISE_EQUIVALENT, CORTEXDATATYPE ) \
	IECOREUSD_CORTEXTYPETRAITS_SPECIALISATION( CORTEXTYPE, USDTYPE, BITWISE_EQUIVALENT ) \
	IECOREUSD_USDTYPETRAITS_SPECIALISATION( CORTEXTYPE, USDTYPE, BITWISE_EQUIVALENT, CORTEXDATATYPE )

IECOREUSD_TYPETRAITS_SPECIALISATION( bool, bool, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( unsigned char, unsigned char, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( half, pxr::GfHalf, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( float, float, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( double, double, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( int, int, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( unsigned int, unsigned int, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( int64_t, int64_t, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( uint64_t, uint64_t, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V2i, pxr::GfVec2i, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V3i, pxr::GfVec3i, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V2f, pxr::GfVec2f, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V3f, pxr::GfVec3f, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V2d, pxr::GfVec2d, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::V3d, pxr::GfVec3d, true, IECore::GeometricTypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::Color4f, pxr::GfVec4f, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::M33f, pxr::GfMatrix3f, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::M33d, pxr::GfMatrix3d, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::M44f, pxr::GfMatrix4f, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::M44d, pxr::GfMatrix4d, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::Quatf, pxr::GfQuatf, false, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( Imath::Quatd, pxr::GfQuatd, false, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( std::string, std::string, true, IECore::TypedData )
IECOREUSD_TYPETRAITS_SPECIALISATION( IECore::InternedString, pxr::TfToken, false, IECore::TypedData )
// Only specialising CortexTypeTraits, because we can't map `USDTypeTraits<GfVec3f>`
// to both `Imath::Vec3f` and `Imath::Color3f`.
IECOREUSD_CORTEXTYPETRAITS_SPECIALISATION( Imath::Color3f, pxr::GfVec3f, true )
// Only specialising USDTypeTraits, because we can't map `Quatf` to both
// `GfQuath` and `GfQuatf`.
/// \todo Should we convert to `Imath::Quat<half>` in Cortex instead?
IECOREUSD_USDTYPETRAITS_SPECIALISATION( Imath::Quatf, pxr::GfQuath, false, IECore::TypedData )

} // namespace IECoreUSD

#endif // IECOREUSD_TYPETRAITS_H
