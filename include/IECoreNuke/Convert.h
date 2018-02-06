//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_CONVERT_H
#define IECORENUKE_CONVERT_H

#include "IECoreNuke/Export.h"

#include "IECore/Convert.h"

#include "DDImage/Box.h"
#include "DDImage/Box3.h"
#include "DDImage/Matrix4.h"
#include "DDImage/Vector2.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector4.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

/// Specialising in the IECore namespace. This is OK because the Nuke types
/// will never be referenced in IECore. And it means that all the convert<>
/// functions are in one namespace.
namespace IECore
{

template<>
IECORENUKE_API Imath::V2f convert( const DD::Image::Vector2 &from );

template<>
IECORENUKE_API Imath::V2d convert( const DD::Image::Vector2 &from );

/// Discards from.z
template<>
IECORENUKE_API Imath::V2f convert( const DD::Image::Vector3 &from );

/// Discards from.z
template<>
IECORENUKE_API Imath::V2d convert( const DD::Image::Vector3 &from );

template<>
IECORENUKE_API Imath::V3f convert( const DD::Image::Vector3 &from );

template<>
IECORENUKE_API Imath::V3d convert( const DD::Image::Vector3 &from );

template<>
IECORENUKE_API Imath::Color3f convert( const DD::Image::Vector3 &from );

template<>
IECORENUKE_API DD::Image::Vector3 convert( const Imath::V3f &from );

template<>
IECORENUKE_API DD::Image::Vector3 convert( const Imath::V3d &from );

/// Discards from.z and from.w
template<>
IECORENUKE_API Imath::V2f convert( const DD::Image::Vector4 &from );

/// Discards from.z and from.w
template<>
IECORENUKE_API Imath::V2d convert( const DD::Image::Vector4 &from );

/// Discards from.w
template<>
IECORENUKE_API Imath::V3f convert( const DD::Image::Vector4 &from );

/// Discards from.w
template<>
IECORENUKE_API Imath::Color3f convert( const DD::Image::Vector4 &from );

/// Discards from.w
template<>
IECORENUKE_API Imath::V3d convert( const DD::Image::Vector4 &from );

template<>
IECORENUKE_API Imath::M44f convert( const DD::Image::Matrix4 &from );

template<>
IECORENUKE_API Imath::M44d convert( const DD::Image::Matrix4 &from );

template<>
IECORENUKE_API Imath::Box2i convert( const DD::Image::Box &from );

template<>
IECORENUKE_API DD::Image::Box3 convert( const Imath::Box3f &from );

} // namespace IECore

#endif // IECORENUKE_CONVERT_H
