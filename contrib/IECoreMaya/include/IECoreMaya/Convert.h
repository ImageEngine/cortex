//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathMatrix.h"
#include "IECore/TransformationMatrix.h"

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

#include <string>

namespace IECoreMaya
{

/// The convert function is used to provide easy conversion
/// of simple types (like vectors and strings) between maya
/// and basic IECore types - the Converter classes exist for conversions
/// of more complex objects. This function must be specialised for
/// any types it will be used with.
/// \todo Should the convert function go in IECore?
/// \todo Add more conversions.
template<typename T, typename F>
T convert( const F &from )
{
	return static_cast<T>( from );
}

template<>
std::string convert( const MString &from );

template<>
MString convert( const std::string &from );

template<>
Imath::V3f convert( const MVector &from );

template<>
Imath::V3f convert( const MFloatVector &from );

template<>
Imath::V3d convert( const MVector &from );

template<>
Imath::V3d convert( const MFloatVector &from );

template<>
Imath::V3f convert( const MPoint &from );

template<>
Imath::V3f convert( const MFloatPoint &from );

template<>
Imath::V3d convert( const MPoint &from );

template<>
Imath::V3d convert( const MFloatPoint &from );

template<>
MVector convert( const Imath::V3f &from );

template<>
MVector convert( const Imath::V3d &from );

template<>
MFloatVector convert( const Imath::V3f &from );

template<>
MFloatVector convert( const Imath::V3d &from );

template<>
MPoint convert( const Imath::V3f &from );

template<>
MPoint convert( const Imath::V3d &from );

template<>
MFloatPoint convert( const Imath::V3f &from );

template<>
MFloatPoint convert( const Imath::V3d &from );

template<>
Imath::Color3f convert( const MVector &from );

template<>
Imath::Color3f convert( const MColor &from );

template<>
Imath::Color4f convert( const MColor &from );

template<>
MBoundingBox convert( const Imath::Box3f &from );

template<>
Imath::Box3f convert( const MBoundingBox &from );

template<>
Imath::Quatf convert( const MQuaternion &from );

template<>
MQuaternion convert( const Imath::Quatf &from );

template<>
Imath::Quatd convert( const MQuaternion &from );

template<>
MQuaternion convert( const Imath::Quatd &from );

template<>
Imath::M44f convert( const MMatrix &from );

template<>
MMatrix convert( const Imath::M44f &from );

template<>
IECore::TransformationMatrixf convert( const MTransformationMatrix &from );

template<>
MTransformationMatrix convert( const IECore::TransformationMatrixf &from );

template<>
IECore::TransformationMatrixd convert( const MTransformationMatrix &from );

template<>
MTransformationMatrix convert( const IECore::TransformationMatrixd &from );

} // namespace IECoreMaya

#endif // IE_COREMAYA_CONVERT_H
