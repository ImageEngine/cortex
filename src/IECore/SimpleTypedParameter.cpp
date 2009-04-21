//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include <string>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/SimpleTypedParameter.h"

#include "IECore/TypedParameter.inl"

using std::string;
using namespace Imath;

namespace IECore
{

IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( bool, BoolParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( string, StringParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2i, V2iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3i, V3iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2f, V2fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3f, V3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2d, V2dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3d, V3dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Color3f, Color3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Color4f, Color4fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2i, Box2iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3i, Box3iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2f, Box2fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3f, Box3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2d, Box2dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3d, Box3dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( M44f, M44fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( M44d, M44dParameter )

}
