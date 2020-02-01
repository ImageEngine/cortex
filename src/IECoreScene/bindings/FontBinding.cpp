//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "FontBinding.h"

#include "IECoreScene/Font.h"
#include "IECoreScene/Group.h"
#include "IECoreScene/MeshPrimitive.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

using namespace boost::python;
using namespace boost;
using namespace std;
using namespace IECorePython;
using namespace IECoreScene;

namespace
{

MeshPrimitivePtr mesh1( Font &f, char c )
{
	IECorePython::ScopedGILRelease gilRelease;
	ConstMeshPrimitivePtr m = f.mesh( c );
	if( m )
	{
		return m->copy();
	}
	return nullptr;
}

MeshPrimitivePtr mesh2( Font &f, const std::string &s )
{
	IECorePython::ScopedGILRelease gilRelease;
	return f.mesh( s );
}

} // namespace

namespace IECoreSceneModule
{

void bindFont()
{
	RunTimeTypedClass<Font>()
		.def(  init<const std::string &>() )
		.def( "fileName", &Font::fileName, return_value_policy<copy_const_reference>() )
		.def( "setCurveTolerance", &Font::setCurveTolerance )
		.def( "getCurveTolerance", &Font::getCurveTolerance )
		.def( "setKerning", &Font::setKerning )
		.def( "getKerning", &Font::getKerning )
		.def( "setLineSpacing", &Font::setLineSpacing )
		.def( "getLineSpacing", &Font::getLineSpacing )
		.def( "mesh", &mesh1 )
		.def( "mesh", &mesh2 )
		.def( "meshGroup", &Font::meshGroup )
		.def( "advance", &Font::advance )
		.def( "bound", (Imath::Box2f (Font::*)( )const)&Font::bound )
		.def( "bound", (Imath::Box2f (Font::*)( char )const)&Font::bound )
		.def( "bound", (Imath::Box2f (Font::*)( const std::string &)const)&Font::bound )
	;

}

} // namespace IECoreSceneModule
