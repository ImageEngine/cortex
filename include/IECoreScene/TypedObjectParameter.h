//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_TYPEDOBJECTPARAMETER_H
#define IECORESCENE_TYPEDOBJECTPARAMETER_H

#include "IECore/TypedObjectParameter.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( Renderable )
IE_CORE_FORWARDDECLARE( StateRenderable )
IE_CORE_FORWARDDECLARE( AttributeState )
IE_CORE_FORWARDDECLARE( Shader )
IE_CORE_FORWARDDECLARE( Transform )
IE_CORE_FORWARDDECLARE( MatrixMotionTransform )
IE_CORE_FORWARDDECLARE( MatrixTransform )
IE_CORE_FORWARDDECLARE( VisibleRenderable )
IE_CORE_FORWARDDECLARE( Group )
IE_CORE_FORWARDDECLARE( SmoothSkinningData )

typedef IECore::TypedObjectParameter<Renderable> RenderableParameter;
typedef IECore::TypedObjectParameter<StateRenderable> StateRenderableParameter;
typedef IECore::TypedObjectParameter<AttributeState> AttributeStateParameter;
typedef IECore::TypedObjectParameter<Shader> ShaderParameter;
typedef IECore::TypedObjectParameter<Transform> TransformParameter;
typedef IECore::TypedObjectParameter<MatrixMotionTransform> MatrixMotionTransformParameter;
typedef IECore::TypedObjectParameter<MatrixTransform> MatrixTransformParameter;
typedef IECore::TypedObjectParameter<VisibleRenderable> VisibleRenderableParameter;
typedef IECore::TypedObjectParameter<Group> GroupParameter;
typedef IECore::TypedObjectParameter<SmoothSkinningData> SmoothSkinningDataParameter;

IE_CORE_DECLAREPTR( RenderableParameter );
IE_CORE_DECLAREPTR( StateRenderableParameter );
IE_CORE_DECLAREPTR( AttributeStateParameter );
IE_CORE_DECLAREPTR( ShaderParameter );
IE_CORE_DECLAREPTR( TransformParameter );
IE_CORE_DECLAREPTR( MatrixMotionTransformParameter );
IE_CORE_DECLAREPTR( MatrixTransformParameter );
IE_CORE_DECLAREPTR( VisibleRenderableParameter );
IE_CORE_DECLAREPTR( GroupParameter );
IE_CORE_DECLAREPTR( SmoothSkinningDataParameter );

} // namespace IECoreScene

#endif // IECORESCENE_TYPEDOBJECTPARAMETER_H
