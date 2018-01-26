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

#include "IECoreGL/bindings/ShaderBinding.h"

#include "IECoreGL/Shader.h"
#include "IECoreGL/Texture.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "boost/python/suite/indexing/container_utils.hpp"

using namespace boost::python;
using namespace std;

namespace IECoreGL
{

static boost::python::list uniformParameterNames( const Shader &s )
{
	vector<string> n;
	s.uniformParameterNames( n );
	boost::python::list result;
	for( unsigned int i=0; i<n.size(); i++ )
	{
		result.append( n[i] );
	}
	return result;
}

static object parameterToObject( const Shader::Parameter *p )
{
	if( !p )
	{
		return object();
	}
	else
	{
		return object( Shader::Parameter( *p ) );
	}
}

static object uniformParameter( const Shader &s, const std::string &n )
{
	return parameterToObject( s.uniformParameter( n ) );
}

static boost::python::list vertexAttributeNames( const Shader &s )
{
	vector<string> n;
	s.vertexAttributeNames( n );
	boost::python::list result;
	for( unsigned int i=0; i<n.size(); i++ )
	{
		result.append( n[i] );
	}
	return result;
}

static object vertexAttribute( const Shader &s, const std::string &n )
{
	return parameterToObject( s.vertexAttribute( n ) );
}

static object csParameter( const Shader &s )
{
	return parameterToObject( s.csParameter() );
}

static ShaderPtr shader( Shader::Setup &s )
{
	return const_cast<Shader *>( s.shader() );
}

void bindShader()
{
	scope s = IECorePython::RunTimeTypedClass<Shader>()
		.def( init<const std::string &, const std::string &>() )
		.def( init<const std::string &, const std::string &, const std::string &>() )
		.def( "program", &Shader::program )
		.def( "vertexSource", &Shader::vertexSource, return_value_policy<copy_const_reference>() )
		.def( "geometrySource", &Shader::geometrySource, return_value_policy<copy_const_reference>() )
		.def( "fragmentSource", &Shader::fragmentSource, return_value_policy<copy_const_reference>() )
		.def( "uniformParameterNames", &uniformParameterNames )
		.def( "uniformParameter", &uniformParameter )
		.def( "vertexAttributeNames", &vertexAttributeNames )
		.def( "vertexAttribute", &vertexAttribute )
		.def( "csParameter", &csParameter )
		.def( "defaultVertexSource", &Shader::defaultVertexSource, return_value_policy<copy_const_reference>() ).staticmethod( "defaultVertexSource" )
		.def( "defaultFragmentSource", &Shader::defaultFragmentSource, return_value_policy<copy_const_reference>() ).staticmethod( "defaultFragmentSource" )
		.def( "constant", &Shader::constant, return_value_policy<IECorePython::CastToIntrusivePtr>() ).staticmethod( "constant" )
		.def( "facingRatio", &Shader::facingRatio, return_value_policy<IECorePython::CastToIntrusivePtr>() ).staticmethod( "facingRatio" )
	;

	IECorePython::RefCountedClass<Shader::Setup, IECore::RefCounted>( "Setup" )
		.def( init<ConstShaderPtr>() )
		.def( "shader", &shader )
		.def( "addUniformParameter", (void (Shader::Setup::*)( const std::string &, ConstTexturePtr ))&Shader::Setup::addUniformParameter )
		.def( "addUniformParameter", (void (Shader::Setup::*)( const std::string &, IECore::ConstDataPtr ))&Shader::Setup::addUniformParameter )
		.def( "addVertexAttribute", &Shader::Setup::addVertexAttribute )
	;

	class_<Shader::Parameter>( "Parameter", no_init )
		.def_readonly( "type", &Shader::Parameter::type )
		.def_readonly( "size", &Shader::Parameter::size )
		.def_readonly( "location", &Shader::Parameter::location )
		.def_readonly( "textureUnit", &Shader::Parameter::textureUnit )
		.def( self == self )
	;
}

} // namespace IECoreGL
