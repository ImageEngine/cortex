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
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECoreGL/Shader.h"
#include "IECoreGL/bindings/ShaderBinding.h"

#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace std;

namespace IECoreGL
{

boost::python::list parameterNames( const Shader &s )
{
	vector<string> n;
	s.parameterNames( n );
	boost::python::list result;
	for( unsigned int i=0; i<n.size(); i++ )
	{
		result.append( n[i] );
	}
	return result;
}

void bindShader()
{
	IECore::RunTimeTypedClass<Shader>()
		.def( init<const std::string &, const std::string &>() )
		.def( "parameterNames", &parameterNames )
		.def( "parameterIndex", &Shader::parameterIndex )
		.def( "hasParameter", &Shader::hasParameter )
		.def( "parameterType", (IECore::TypeId (Shader::*)( GLint ) const )&Shader::parameterType )
		.def( "parameterType", (IECore::TypeId (Shader::*)( const std::string & ) const )&Shader::parameterType )
		.def( "getParameter", (IECore::DataPtr (Shader::*)( GLint ) const )&Shader::getParameter )
		.def( "getParameter", (IECore::DataPtr (Shader::*)( const std::string & ) const )&Shader::getParameter )
		.def( "valueValid", (bool (Shader::*)( GLint, IECore::ConstDataPtr ) const )&Shader::valueValid )
		.def( "valueValid", (bool (Shader::*)( const std::string &, IECore::ConstDataPtr ) const )&Shader::valueValid )
		.def( "setParameter", (void (Shader::*)( GLint, IECore::ConstDataPtr ))&Shader::setParameter )
		.def( "setParameter", (void (Shader::*)( const std::string &, IECore::ConstDataPtr ))&Shader::setParameter )
		.def( "setParameter", (void (Shader::*)( GLint, unsigned int ))&Shader::setParameter )
		.def( "setParameter", (void (Shader::*)( const std::string &, unsigned int ))&Shader::setParameter )
		.def( "setParameter", (void (Shader::*)( const std::string &, int ))&Shader::setParameter )
		.def( "setParameter", (void (Shader::*)( GLint, int ))&Shader::setParameter )
		.def( "constant", &Shader::constant ).staticmethod( "constant" )
		.def( "facingRatio", &Shader::facingRatio ).staticmethod( "facingRatio" )
		.def( self==self )
	;
}

}
