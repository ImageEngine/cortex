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

#include "IECoreGL/bindings/RendererBinding.h"

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/TextureLoader.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECore/SimpleTypedData.h"

using namespace boost::python;
using namespace IECore;

namespace IECoreGL
{

static ShaderLoaderPtr shaderLoader( Renderer &r )
{
	return r.shaderLoader();
}

static TextureLoaderPtr textureLoader( Renderer &r )
{
	return r.textureLoader();
}

static void worldBegin( Renderer &r )
{
	if ( boost::static_pointer_cast<const IECore::StringData>(r.getOption( "gl:mode" ))->readable() == "deferred" )
	{
		// The deferred render uses multiple threads when rendering procedurals. So we enable threads for python here.
		// \todo Consider moving this to IECore::Renderer::worldBegin binding (assuming all decent renderers are multithreaded).
		if ( !PyEval_ThreadsInitialized() )
		{
			PyEval_InitThreads();
		}
	}
	r.worldBegin();
}

void bindRenderer()
{
	IECorePython::RunTimeTypedClass<Renderer>()
		.def( init<>() )
		.def( "worldBegin", &worldBegin )
		.def( "scene", &Renderer::scene )
		.def( "shaderLoader", &shaderLoader )
		.def( "textureLoader", &textureLoader )
	;
}

}
