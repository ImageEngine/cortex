//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECoreGL/Shader.h"
#include "IECoreGL/bindings/ShaderBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace std;

namespace IECoreGL
{

boost::python::list uniformParameterNames( const Shader &s )
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

boost::python::list vertexParameterNames( const Shader &s )
{
	vector<string> n;
	s.vertexParameterNames( n );
	boost::python::list result;
	for( unsigned int i=0; i<n.size(); i++ )
	{
		result.append( n[i] );
	}
	return result;
}

void bindShader()
{
	IECorePython::RunTimeTypedClass<Shader>()
		.def( init<const std::string &, const std::string &>() )
		.def( "uniformParameterNames", &uniformParameterNames )
		.def( "uniformParameterIndex", &Shader::uniformParameterIndex )
		.def( "hasUniformParameter", &Shader::hasUniformParameter )
		.def( "uniformParameterType", (IECore::TypeId (Shader::*)( GLint ) const )&Shader::uniformParameterType )
		.def( "uniformParameterType", (IECore::TypeId (Shader::*)( const std::string & ) const )&Shader::uniformParameterType )
		.def( "getUniformParameter", (IECore::DataPtr (Shader::*)( GLint ) const )&Shader::getUniformParameter )
		.def( "getUniformParameter", (IECore::DataPtr (Shader::*)( const std::string & ) const )&Shader::getUniformParameter )
		.def( "uniformValueValid", (bool (Shader::*)( GLint, const IECore::Data* ) const )&Shader::uniformValueValid )
		.def( "uniformValueValid", (bool (Shader::*)( const std::string &, const IECore::Data* ) const )&Shader::uniformValueValid )
		.def( "setUniformParameter", (void (Shader::*)( GLint, const IECore::Data* ))&Shader::setUniformParameter )
		.def( "setUniformParameter", (void (Shader::*)( const std::string &, const IECore::Data* ))&Shader::setUniformParameter )
		.def( "setUniformParameter", (void (Shader::*)( GLint, unsigned int ))&Shader::setUniformParameter )
		.def( "setUniformParameter", (void (Shader::*)( const std::string &, unsigned int ))&Shader::setUniformParameter )
		.def( "setUniformParameter", (void (Shader::*)( const std::string &, int ))&Shader::setUniformParameter )
		.def( "setUniformParameter", (void (Shader::*)( GLint, int ))&Shader::setUniformParameter )
		.def( "uniformVectorValueValid", (bool (Shader::*)( GLint, const IECore::Data*) const)&Shader::uniformVectorValueValid )
		.def( "uniformVectorValueValid", (bool (Shader::*)( const std::string &, const IECore::Data*) const)&Shader::uniformVectorValueValid )
		.def( "setUniformParameterFromVector", (void (Shader::*)( GLint, const IECore::Data*, unsigned int ))&Shader::setUniformParameterFromVector )
		.def( "setUniformParameterFromVector", (void (Shader::*)( const std::string &, const IECore::Data*, unsigned int ))&Shader::setUniformParameterFromVector )
		.def( "vertexParameterNames", &vertexParameterNames )
		.def( "vertexParameterIndex", &Shader::vertexParameterIndex )
		.def( "hasVertexParameter", &Shader::hasVertexParameter )
		.def( "vertexValueValid", (bool (Shader::*)( GLint, const IECore::Data* ) const)&Shader::vertexValueValid )
		.def( "vertexValueValid", (bool (Shader::*)( const std::string &, const IECore::Data*) const)&Shader::vertexValueValid )
		.def( "setVertexParameter", (void (Shader::*)( GLint, const IECore::Data*, bool) )&Shader::setVertexParameter, (arg_("parameterIndex"),arg_("value"),arg_("normalize")=false) )
		.def( "setVertexParameter", (void (Shader::*)( const std::string &, const IECore::Data*, bool) )&Shader::setVertexParameter, (arg_("parameterName"),arg_("value"),arg_("normalize")=false) )
		.def( "unsetVertexParameters", &Shader::unsetVertexParameters )
		.def( "constant", &Shader::constant ).staticmethod( "constant" )
		.def( "facingRatio", &Shader::facingRatio ).staticmethod( "facingRatio" )
		.def( self==self )
	;
}

}
