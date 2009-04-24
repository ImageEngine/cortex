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

#include <boost/python.hpp>

#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/bindings/SceneBinding.h"

#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;

namespace IECoreGL
{

static list select( Scene &s, const Imath::Box2f &b )
{
	std::list<HitRecord> hits;
	s.select( b, hits );
	list result;
	for( std::list<HitRecord>::const_iterator it=hits.begin(); it!=hits.end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

void bindScene()
{
	IECore::RunTimeTypedClass<Scene>()
		.def( init<>() )
		.def( "root", (GroupPtr (Scene::*)() )&Scene::root )
		.def( "render", (void (Scene::*)() const )&Scene::render )
		.def( "render", (void (Scene::*)( ConstStatePtr ) const )&Scene::render )
		.def( "select", &select )
		.def( "setCamera", &Scene::setCamera )
		.def( "getCamera", (CameraPtr (Scene::*)())&Scene::getCamera )
	;
}

}
