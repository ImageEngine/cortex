//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREHOUDINI_CONVERT_H
#define IE_COREHOUDINI_CONVERT_H

#include "IECore/Convert.h"
#include "IECore/Data.h"
#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathEuler.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "UT/UT_BoundingBox.h"
#include "UT/UT_Color.h"
#include "UT/UT_Matrix4.h"
#include "UT/UT_Vector3.h"

#include <string>

/// Specialising in the IECore namespace. This is OK because the Houdini types
/// will never be referenced in IECore
namespace IECore
{

template<>
UT_Vector3 convert( const Imath::V3f &from );

template<>
Imath::V3f convert( const UT_Vector3 &from );

template<>
UT_Vector3 convert( const Imath::V3d &from );

template<>
Imath::V3d convert( const UT_Vector3 &from );

template<>
UT_Vector4 convert( const Imath::V3f &from );

template<>
Imath::V3f convert( const UT_Vector4 &from );

template<>
UT_Vector4 convert( const Imath::V3d &from );

template<>
Imath::V3d convert( const UT_Vector4 &from );

template<>
Imath::Color3f convert( const UT_Color &from );

template<>
UT_Color convert( const Imath::Color3f &from );

template<>
Imath::Color4f convert( const UT_Color &from );

template<>
UT_Color convert( const Imath::Color4f &from );

template<>
UT_BoundingBox convert( const Imath::Box3f &from );

template<>
Imath::Box3f convert( const UT_BoundingBox &from );

template<>
UT_BoundingBox convert( const Imath::Box3d &from );

template<>
Imath::Box3d convert( const UT_BoundingBox &from );

template<>
UT_Matrix4T<double> convert( const Imath::M44d &from );

template<>
Imath::M44d convert( const UT_Matrix4T<double> &from );

template<>
Imath::M44f convert( const UT_Matrix4T<double> &from );

template<>
UT_Matrix4T<float> convert( const Imath::M44f &from );

template<>
Imath::M44f convert( const UT_Matrix4T<float> &from );

template<>
Imath::M44d convert( const UT_Matrix4T<float> &from );

} // namespace IECore

#endif // IE_COREHOUDINI_CONVERT_H
