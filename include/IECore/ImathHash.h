//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IMATHHASH_H
#define IE_CORE_IMATHHASH_H

#include "IECore/Half.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathBox.h"
#include "Imath/ImathColor.h"
#include "Imath/ImathMatrix.h"
#include "Imath/ImathQuat.h"
#include "Imath/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECore
{

namespace Detail
{

template <typename T>
struct PodHash
{
	size_t operator()( const T &x ) const
	{
		return std::hash<std::string_view>()( std::string_view( (char*)&x, sizeof( T ) ) );
	}
};

} // namespace Detail

} // namespace IECore

namespace std
{

template <> struct hash<half> : public IECore::Detail::PodHash<half> { };
template <> struct hash<Imath::V2f> : public IECore::Detail::PodHash<Imath::V2f> { };
template <> struct hash<Imath::V2i> : public IECore::Detail::PodHash<Imath::V2i> { };
template <> struct hash<Imath::V2d> : public IECore::Detail::PodHash<Imath::V2d> { };
template <> struct hash<Imath::V3f> : public IECore::Detail::PodHash<Imath::V3f> { };
template <> struct hash<Imath::V3i> : public IECore::Detail::PodHash<Imath::V3i> { };
template <> struct hash<Imath::V3d> : public IECore::Detail::PodHash<Imath::V3d> { };
template <> struct hash<Imath::Color3f> : public IECore::Detail::PodHash<Imath::Color3f> { };
template <> struct hash<Imath::Color4f> : public IECore::Detail::PodHash<Imath::Color4f> { };
template <> struct hash<Imath::Quatf> : public IECore::Detail::PodHash<Imath::Quatf> { };
template <> struct hash<Imath::Quatd> : public IECore::Detail::PodHash<Imath::Quatd> { };
template <> struct hash<Imath::M33f> : public IECore::Detail::PodHash<Imath::M33f> { };
template <> struct hash<Imath::M33d> : public IECore::Detail::PodHash<Imath::M33d> { };
template <> struct hash<Imath::M44f> : public IECore::Detail::PodHash<Imath::M44f> { };
template <> struct hash<Imath::M44d> : public IECore::Detail::PodHash<Imath::M44d> { };
template <> struct hash<Imath::Box2i> : public IECore::Detail::PodHash<Imath::Box2i> { };
template <> struct hash<Imath::Box3i> : public IECore::Detail::PodHash<Imath::Box3i> { };
template <> struct hash<Imath::Box2f> : public IECore::Detail::PodHash<Imath::Box2f> { };
template <> struct hash<Imath::Box3f> : public IECore::Detail::PodHash<Imath::Box3f> { };
template <> struct hash<Imath::Box2d> : public IECore::Detail::PodHash<Imath::Box2d> { };
template <> struct hash<Imath::Box3d> : public IECore::Detail::PodHash<Imath::Box3d> { };

} // namespace std

#endif // IE_CORE_IMATHHASH_H
