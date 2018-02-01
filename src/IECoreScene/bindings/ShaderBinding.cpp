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

#include "ShaderBinding.h"

#include "IECoreScene/Shader.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

	static ShaderPtr construct( const std::string &name="defaultsurface", const std::string &type="surface", CompoundDataPtr parameters = nullptr )
	{
		return new Shader( name, type, parameters ? parameters->readable() : CompoundDataMap() );
	}

	static CompoundDataPtr parametersData( Shader &s )
	{
		return s.parametersData();
	}

	void bindShader()
	{
		RunTimeTypedClass<Shader>()
			.def( init<>() )
			.def( init<optional<const std::string &, const std::string &, const CompoundDataMap &> >() )
			.def( "__init__", make_constructor( &construct, default_call_policies(), ( boost::python::arg_( "name" )="defaultsurface", boost::python::arg_( "type" )="surface", boost::python::arg_( "parameters" )=0 ) ) )
			.add_property( "name", make_function( &Shader::getName, return_value_policy<copy_const_reference>() ), &Shader::setName )
			.add_property( "type", make_function( &Shader::getType, return_value_policy<copy_const_reference>() ), &Shader::setType )
			.add_property( "parameters", &parametersData )
		;
	}

}
