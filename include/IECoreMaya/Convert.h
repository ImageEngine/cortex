//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_CONVERT_H
#define IE_COREMAYA_CONVERT_H

#include "IECoreMaya/Export.h"

#include "IECore/Convert.h"
#include "IECore/Data.h"
#include "IECore/TransformationMatrix.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathEuler.h"

#include "maya/MString.h"
#include "maya/MBoundingBox.h"
#include "maya/MPoint.h"
#include "maya/MVector.h"
#include "maya/MMatrix.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatVector.h"
#include "maya/MColor.h"
#include "maya/MQuaternion.h"
#include "maya/MTransformationMatrix.h"
#include "maya/MCommandResult.h"
#include "maya/MEulerRotation.h"
#include "maya/MDistance.h"
#include "maya/MAngle.h"
#include "maya/MTime.h"

#include <string>

/// Specialising in the IECore namespace. This is OK because the Maya types
/// will never be referenced in IECore
namespace IECore
{

template<>
IECOREMAYA_API std::string convert( const MString &from );

template<>
IECOREMAYA_API MString convert( const std::string &from );

template<>
IECOREMAYA_API Imath::V3f convert( const MVector &from );

template<>
IECOREMAYA_API Imath::V3f convert( const MFloatVector &from );

template<>
IECOREMAYA_API Imath::V3d convert( const MVector &from );

template<>
IECOREMAYA_API Imath::V3d convert( const MFloatVector &from );

template<>
IECOREMAYA_API Imath::V3f convert( const MPoint &from );

template<>
IECOREMAYA_API Imath::V3f convert( const MFloatPoint &from );

template<>
IECOREMAYA_API Imath::V3d convert( const MPoint &from );

template<>
IECOREMAYA_API Imath::V3d convert( const MFloatPoint &from );

template<>
IECOREMAYA_API MVector convert( const Imath::V3f &from );

template<>
IECOREMAYA_API MVector convert( const Imath::V3d &from );

template<>
IECOREMAYA_API MVector convert( const Imath::Color3f &from );

template<>
IECOREMAYA_API MFloatVector convert( const Imath::V3f &from );

template<>
IECOREMAYA_API MFloatVector convert( const Imath::V3d &from );

template<>
IECOREMAYA_API MFloatVector convert( const Imath::Color3f &from );

template<>
IECOREMAYA_API MPoint convert( const Imath::V3f &from );

template<>
IECOREMAYA_API MPoint convert( const Imath::V3d &from );

template<>
IECOREMAYA_API MFloatPoint convert( const Imath::V3f &from );

template<>
IECOREMAYA_API MFloatPoint convert( const Imath::V3d &from );

template<>
IECOREMAYA_API Imath::Color3f convert( const MVector &from );

template<>
IECOREMAYA_API Imath::Color3f convert( const MColor &from );

template<>
IECOREMAYA_API Imath::Color4f convert( const MColor &from );

template<>
IECOREMAYA_API MColor convert( const Imath::Color3f &from );

template<>
IECOREMAYA_API MColor convert( const Imath::Color4f &from );

template<>
IECOREMAYA_API MBoundingBox convert( const Imath::Box3f &from );

template<>
IECOREMAYA_API Imath::Box3f convert( const MBoundingBox &from );

template<>
IECOREMAYA_API MBoundingBox convert( const Imath::Box3d &from );

template<>
IECOREMAYA_API Imath::Box3d convert( const MBoundingBox &from );

template<>
IECOREMAYA_API Imath::Quatf convert( const MQuaternion &from );

template<>
IECOREMAYA_API MQuaternion convert( const Imath::Quatf &from );

template<>
IECOREMAYA_API Imath::Quatd convert( const MQuaternion &from );

template<>
IECOREMAYA_API MQuaternion convert( const Imath::Quatd &from );

template<>
IECOREMAYA_API Imath::M44f convert( const MMatrix &from );

template<>
IECOREMAYA_API Imath::M44d convert( const MMatrix &from );

template<>
IECOREMAYA_API MMatrix convert( const Imath::M44f &from );

template<>
IECOREMAYA_API MMatrix convert( const Imath::M44d &from );

template<>
IECOREMAYA_API Imath::Eulerf convert( const MEulerRotation &from );

template<>
IECOREMAYA_API MEulerRotation convert( const Imath::Eulerf &from );

template<>
IECOREMAYA_API Imath::Eulerd convert( const MEulerRotation &from );

template<>
IECOREMAYA_API MEulerRotation convert( const Imath::Eulerd &from );

template<>
IECOREMAYA_API IECore::TransformationMatrixf convert( const MTransformationMatrix &from );

template<>
IECOREMAYA_API MTransformationMatrix convert( const IECore::TransformationMatrixf &from );

template<>
IECOREMAYA_API IECore::TransformationMatrixd convert( const MTransformationMatrix &from );

template<>
IECOREMAYA_API MTransformationMatrix convert( const IECore::TransformationMatrixd &from );

template<>
IECOREMAYA_API IECore::DataPtr convert( const MCommandResult &from );

template<>
IECOREMAYA_API MDistance convert( const double &from );

template<>
IECOREMAYA_API double convert( const MDistance &from );

template<>
IECOREMAYA_API MAngle convert( const double &from );

template<>
IECOREMAYA_API double convert( const MAngle &from );

template<>
IECOREMAYA_API MTime convert( const double &from );

template<>
IECOREMAYA_API double convert( const MTime &from );

template<>
IECOREMAYA_API MDistance convert( const float &from );

template<>
IECOREMAYA_API float convert( const MDistance &from );

template<>
IECOREMAYA_API MAngle convert( const float &from );

template<>
IECOREMAYA_API float convert( const MAngle &from );

template<>
IECOREMAYA_API MTime convert( const float &from );

template<>
IECOREMAYA_API float convert( const MTime &from );

} // namespace IECore

#endif // IE_COREMAYA_CONVERT_H
