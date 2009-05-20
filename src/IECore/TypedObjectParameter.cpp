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

#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"

#include "IECore/Renderable.h"
#include "IECore/StateRenderable.h"
#include "IECore/AttributeState.h"
#include "IECore/Shader.h"
#include "IECore/Transform.h"
#include "IECore/MatrixMotionTransform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/Group.h"
#include "IECore/ObjectVector.h"

namespace IECore
{
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Renderable, RenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( StateRenderable, StateRenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( AttributeState, AttributeStateParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Shader, ShaderParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Transform, TransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MatrixMotionTransform, MatrixMotionTransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MatrixTransform, MatrixTransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( VisibleRenderable, VisibleRenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Group, GroupParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( CompoundObject, CompoundObjectParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( ObjectVector, ObjectVectorParameter );
}
