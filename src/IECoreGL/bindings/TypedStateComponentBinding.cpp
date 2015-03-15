//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECore/MessageHandler.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreGL/StateComponent.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/bindings/TypedStateComponentBinding.h"
#include "IECoreGL/bindings/TypedStateComponentBinding.inl"

using namespace boost::python;

namespace IECoreGL
{

void bindTypedStateComponents()
{
	bindTypedStateComponent< Color >( "Color" );
	bindTypedStateComponent< BlendColorStateComponent >( "BlendColorStateComponent" );
	bindTypedStateComponent< BlendEquationStateComponent >( "BlendEquationStateComponent" );
	bindTypedStateComponent< TransparentShadingStateComponent >( "TransparentShadingStateComponent" );
	bindTypedStateComponent< BoundColorStateComponent >( "BoundColorStateComponent" );
	bindTypedStateComponent< WireframeColorStateComponent >( "WireframeColorStateComponent" );
	bindTypedStateComponent< OutlineColorStateComponent >( "OutlineColorStateComponent" );
	bindTypedStateComponent< PointColorStateComponent >( "PointColorStateComponent" );
	bindTypedStateComponent< BlendFuncStateComponent >( "BlendFuncStateComponent" );
	bindTypedStateComponent< DoubleSidedStateComponent >( "DoubleSidedStateComponent" );
	bindTypedStateComponent< RightHandedOrientationStateComponent >( "RightHandedOrientationStateComponent" );
	bindTypedStateComponent< CullingSpaceStateComponent >( "CullingSpaceStateComponent" );
	bindTypedStateComponent< CullingBoxStateComponent >( "CullingBoxStateComponent" );
	bindTypedStateComponent< ProceduralThreadingStateComponent >( "ProceduralThreadingStateComponent" );
	bindTypedStateComponent< CameraVisibilityStateComponent >( "CameraVisibilityStateComponent" );
	bindTypedStateComponent< AutomaticInstancingStateComponent >( "AutomaticInstancingStateComponent" );
	bindTypedStateComponent< PointSmoothingStateComponent >( "PointSmoothingStateComponent" );
	bindTypedStateComponent< LineSmoothingStateComponent >( "LineSmoothingStateComponent" );
	bindTypedStateComponent< PolygonSmoothingStateComponent >( "PolygonSmoothingStateComponent" );

	enum_<GLPointsUsage>( "GLPointsUsage" )
		.value( "ForPointsOnly", ForPointsOnly )
		.value( "ForPointsAndDisks", ForPointsAndDisks )
		.value( "ForAll", ForAll )
	;

	enum_<RendererSpace>( "RendererSpace" )
		.value( "ObjectSpace", ObjectSpace )
		.value( "WorldSpace", WorldSpace )
	;

}

} // namespace IECoreGL
