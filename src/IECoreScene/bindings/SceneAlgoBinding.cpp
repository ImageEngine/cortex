//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#include "SceneAlgoBinding.h"

#include "IECoreScene/SceneAlgo.h"

#include "IECorePython/ScopedGILRelease.h"


using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;

namespace
{

dict parallelReadAll( const SceneInterface *src, int startFrame, int endFrame, float frameRate, unsigned int flags )
{
	SceneAlgo::SceneStats stats;
	{
		IECorePython::ScopedGILRelease scopedGILRelease;
		stats = SceneAlgo::parallelReadAll( src, startFrame, endFrame, frameRate, flags );
	}

	dict result;
	for (const auto &stat : stats )
	{
		result[stat.first] = stat.second;
	}

	return result;
}

} // namespace

namespace IECoreSceneModule
{

void bindSceneAlgo()
{
	object module( borrowed( PyImport_AddModule( "IECoreScene.SceneAlgo" ) ) );
	scope().attr( "SceneAlgo" ) = module;

	scope _scope( module );

	enum_< SceneAlgo::ProcessFlags > ("ProcessFlags")
		.value("None", SceneAlgo::None )
		// `None` is a keyword in Python 3, so we
		// have to make do with `None_`.
		.value("None_", SceneAlgo::None )
		.value("Bounds", SceneAlgo::Bounds )
		.value("Transforms", SceneAlgo::Transforms )
		.value("Attributes", SceneAlgo::Attributes )
		.value("Tags", SceneAlgo::Tags )
		.value("Sets", SceneAlgo::Sets )
		.value("Objects", SceneAlgo::Objects)
		.value("All", SceneAlgo::All)
		.export_values()
		;

	def( "copy", &SceneAlgo::copy );

	def( "parallelReadAll", &::parallelReadAll);
}

} // namespace IECoreSceneModule
