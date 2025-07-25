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

#include "IECoreScene/TypedObjectParameter.h"

#include "IECoreScene/Renderable.h"
#include "IECoreScene/Shader.h"
#include "IECoreScene/VisibleRenderable.h"

#include "IECore/TypedObjectParameter.inl"

namespace IECore
{

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( IECoreScene::RenderableParameter, IECoreScene::RenderableParameterTypeId );
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( IECoreScene::ShaderParameter, IECoreScene::ShaderParameterTypeId );
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( IECoreScene::VisibleRenderableParameter, IECoreScene::VisibleRenderableParameterTypeId );

template class TypedObjectParameter<IECoreScene::Renderable>;
template class TypedObjectParameter<IECoreScene::Shader>;
template class TypedObjectParameter<IECoreScene::VisibleRenderable>;

}
