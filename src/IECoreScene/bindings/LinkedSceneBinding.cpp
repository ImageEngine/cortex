//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECoreScene/LinkedScene.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "LinkedSceneBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

static LinkedScenePtr constructor( const std::string &fileName, IndexedIO::OpenMode mode )
{
	return new LinkedScene( fileName, mode );
}

static LinkedScenePtr constructor2( SceneInterfacePtr scn )
{
	return new LinkedScene( scn );
}

void bindLinkedScene()
{
	IECore::CompoundDataPtr (*linkAttributeData)( const SceneInterface *scene) = &LinkedScene::linkAttributeData;
	IECore::CompoundDataPtr (*retimedLinkAttributeData)( const SceneInterface *scene, double time) = &LinkedScene::linkAttributeData;

	RunTimeTypedClass<LinkedScene>()
		.def( "__init__", make_constructor( &constructor ), "Opens a linked scene file for read or write." )
		.def( "__init__", make_constructor( &constructor2 ), "Creates a linked scene to expand links in the given scene file." )
		.def( "writeLink", &LinkedScene::writeLink )
		.def( "linkAttributeData", linkAttributeData )
		.def( "linkAttributeData", retimedLinkAttributeData ).staticmethod( "linkAttributeData" )
		.def_readonly("linkAttribute", &LinkedScene::linkAttribute )
		.def_readonly("fileNameLinkAttribute", &LinkedScene::fileNameLinkAttribute )
		.def_readonly("rootLinkAttribute", &LinkedScene::rootLinkAttribute )
		.def_readonly("timeLinkAttribute", &LinkedScene::timeLinkAttribute )
	;
}

} // namespace IECoreSceneModule
