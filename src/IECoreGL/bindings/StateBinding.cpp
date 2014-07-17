//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/bindings/StateBinding.h"

using namespace boost::python;

namespace IECoreGL
{

static IECore::CompoundDataPtr userAttributes( State &s )
{
	return s.userAttributes();
}

static StatePtr defaultState()
{
	return const_cast<State *>( State::defaultState() );
}

static StateComponentPtr get( State &s, IECore::TypeId typeId )
{
	return s.get( typeId );
}

void bindState()
{
	scope s = IECorePython::RunTimeTypedClass<State>()
		.def( init<bool>() )
		.def( init<const State &>() )
		.def( "add", (void (State::*)( StatePtr ) )&State::add )
		.def(
			"add",
			(void (State::*)( StateComponentPtr, bool ) )&State::add,
			(
				boost::python::arg_( "component" ),
				boost::python::arg_( "override" ) = false
			)
		)
		.def( "get", &get )
		.def( "remove", (void (State::*)( IECore::TypeId) )&State::remove )
		.def( "isComplete", &State::isComplete )
		.def( "userAttributes", &userAttributes )
		.def( "defaultState", &defaultState ).staticmethod( "defaultState" )
		.def( "bindBaseState", &State::bindBaseState ).staticmethod( "bindBaseState" )
	;
	
	// this is turned into the actual ScopedBinding class in IECoreGL/State.py.
	class_<State::ScopedBinding, boost::noncopyable>( "_ScopedBinding", no_init )
		.def( init<const State &, State &>() )
	;
}

} // namespace IECoreGL
