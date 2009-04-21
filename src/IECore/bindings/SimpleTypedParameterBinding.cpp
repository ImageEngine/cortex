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

#include "boost/python.hpp"

#include "IECore/bindings/TypedParameterBinding.h"
#include "IECore/SimpleTypedParameter.h"

using namespace std;
using namespace Imath;

namespace IECore
{

void bindSimpleTypedParameter()
{
	bindTypedParameter<bool>( "BoolParameter" );
	bindTypedParameter<V2i>( "V2iParameter" );
	bindTypedParameter<V3i>( "V3iParameter" );
	bindTypedParameter<V2f>( "V2fParameter" );
	bindTypedParameter<V3f>( "V3fParameter" );
	bindTypedParameter<V2d>( "V2dParameter" );
	bindTypedParameter<V3d>( "V3dParameter" );
	bindTypedParameter<Color3f>( "Color3fParameter" );
	bindTypedParameter<Color4f>( "Color4fParameter" );
	bindTypedParameter<Box2i>( "Box2iParameter" );
	bindTypedParameter<Box3i>( "Box3iParameter" );
	bindTypedParameter<Box2f>( "Box2fParameter" );
	bindTypedParameter<Box3f>( "Box3fParameter" );
	bindTypedParameter<Box2d>( "Box2dParameter" );
	bindTypedParameter<Box3d>( "Box3dParameter" );
	bindTypedParameter<M44f>( "M44fParameter" );
	bindTypedParameter<M44d>( "M44dParameter" );
	bindTypedParameter<string>( "StringParameter" );
}

} // namespace IECore
