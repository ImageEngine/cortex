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

#ifndef IE_CORE_SIMPLETYPEDPARAMETER_H
#define IE_CORE_SIMPLETYPEDPARAMETER_H

#include <vector>

#include "IECore/SimpleTypedData.h"
#include "IECore/TypedParameter.h"

namespace IECore
{

typedef TypedParameter<bool> BoolParameter;
typedef TypedParameter<std::string> StringParameter;
typedef TypedParameter<Imath::V2i> V2iParameter;
typedef TypedParameter<Imath::V3i> V3iParameter;
typedef TypedParameter<Imath::V2f> V2fParameter;
typedef TypedParameter<Imath::V3f> V3fParameter;
typedef TypedParameter<Imath::V2d> V2dParameter;
typedef TypedParameter<Imath::V3d> V3dParameter;
typedef TypedParameter<Imath::Color3f> Color3fParameter;
typedef TypedParameter<Imath::Color4f> Color4fParameter;
typedef TypedParameter<Imath::Box2i> Box2iParameter;
typedef TypedParameter<Imath::Box3i> Box3iParameter;
typedef TypedParameter<Imath::Box2f> Box2fParameter;
typedef TypedParameter<Imath::Box3f> Box3fParameter;
typedef TypedParameter<Imath::Box2d> Box2dParameter;
typedef TypedParameter<Imath::Box3d> Box3dParameter;
typedef TypedParameter<Imath::M44f> M44fParameter;
typedef TypedParameter<Imath::M44d> M44dParameter;

IE_CORE_DECLAREPTR( BoolParameter );
IE_CORE_DECLAREPTR( StringParameter );
IE_CORE_DECLAREPTR( V2iParameter );
IE_CORE_DECLAREPTR( V3iParameter );
IE_CORE_DECLAREPTR( V2fParameter );
IE_CORE_DECLAREPTR( V3fParameter );
IE_CORE_DECLAREPTR( V2dParameter );
IE_CORE_DECLAREPTR( V3dParameter );
IE_CORE_DECLAREPTR( Color3fParameter );
IE_CORE_DECLAREPTR( Color4fParameter );
IE_CORE_DECLAREPTR( Box2iParameter );
IE_CORE_DECLAREPTR( Box3iParameter );
IE_CORE_DECLAREPTR( Box2fParameter );
IE_CORE_DECLAREPTR( Box3fParameter );
IE_CORE_DECLAREPTR( Box2dParameter );
IE_CORE_DECLAREPTR( Box3dParameter );
IE_CORE_DECLAREPTR( M44fParameter );
IE_CORE_DECLAREPTR( M44dParameter );

}

#endif // IE_CORE_SIMPLETYPEDPARAMETER_H
