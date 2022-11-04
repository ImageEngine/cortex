//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_SCENESHAPEPROXY_H
#define IE_COREMAYA_SCENESHAPEPROXY_H

#include "IECoreMaya/SceneShape.h"

namespace IECoreMaya
{

/// A proxy derived from the SceneShape which exposes the same functionality as the base clase
/// with the exception, that we never register it as a maya SubSceneOverride. The reasoning
/// behind this is that the SubSceneOverride does not take into account the visibility state of the shape.
/// During an update loop of the SubSceneOverride, all SceneShapes will be queried for their update state,
/// regardless their visibility in the scene. This query is slow and we get a huge drop in performance
/// when having a huge amount of SceneShapes in the scene.
/// This is considered to be a bug in the ViewPort 2 API. Our attempts to rewrite the code to use
/// "MPxGeometryOverride" or "MPxDrawOverride" prove themselves as unstable or not suitable for our
/// use case, why we decided to use this "hackery" and not register a proxy of the SceneShape for
/// drawing at all
class IECOREMAYA_API SceneShapeProxy : public SceneShape
{
	public :

		SceneShapeProxy();
		virtual ~SceneShapeProxy();

		static void *creator();
		static MStatus initialize();
		static MTypeId id;
};

}

#endif // IE_COREMAYA_SCENESHAPEPROXY_H
