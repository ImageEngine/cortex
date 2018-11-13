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

#include "ShaderNetworkBinding.h"

#include "IECoreScene/ShaderNetwork.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace
{

ShaderNetworkPtr constructor( dict shaders, list connections, object output )
{
	ShaderNetworkPtr result = new ShaderNetwork;

	list items = shaders.items();
	for( size_t i = 0, e = len( items ); i < e; ++i )
	{
		InternedString handle = extract<const char *>( items[i][0] )();
		ShaderPtr shader = extract<ShaderPtr>( items[i][1] )();
		result->addShader( handle, shader );
	}

	for( stl_input_iterator<object> it( connections ), eIt; it != eIt; ++it )
	{
		ShaderNetwork::Connection c = extract<ShaderNetwork::Connection>( *it );
		result->addConnection( c );
	}

	if( output != object() )
	{
		const ShaderNetwork::Parameter o = extract<ShaderNetwork::Parameter>( output );
		result->setOutput( o );
	}

	return result;
}

boost::python::dict shaders( ShaderNetwork &network )
{
	boost::python::dict result;
	for( const auto &s : network.shaders() )
	{
		result[s.first.c_str()] = s.second;
	}
	return result;
}

boost::python::list inputConnections( const ShaderNetwork &network, const InternedString &handle )
{
	boost::python::list result;
	for( const auto &c : network.inputConnections( handle ) )
	{
		result.append( c );
	}
	return result;
}

boost::python::list outputConnections( const ShaderNetwork &network, const InternedString &handle )
{
	boost::python::list result;
	for( const auto &c : network.outputConnections( handle ) )
	{
		result.append( c );
	}
	return result;
}

const char *parameterShaderGet( const ShaderNetwork::Parameter &p )
{
	return p.shader.c_str();
}

void parameterShaderSet( ShaderNetwork::Parameter &p, const InternedString &s )
{
	p.shader = s;
}

const char *parameterNameGet( const ShaderNetwork::Parameter &p )
{
	return p.name.c_str();
}

void parameterNameSet( ShaderNetwork::Parameter &p, const InternedString &n )
{
	p.name = n;
}

struct ParameterFromString
{

	static void registerConverter()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<ShaderNetwork::Parameter>()
		);
	}

	private :

		static void *convertible( PyObject *obj )
		{
			return PyString_Check( obj ) ? obj : nullptr;
		}

		static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
		{
			InternedString shader = extract<InternedString>( obj );
			void *storage = ( (converter::rvalue_from_python_storage<ShaderNetwork::Parameter>*) data )->storage.bytes;
			new( storage ) ShaderNetwork::Parameter( shader );
			data->convertible = storage;
		}

};

struct ParameterFromTuple
{

	static void registerConverter()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<ShaderNetwork::Parameter>()
		);
	}

	private :

		static void *convertible( PyObject *obj )
		{
			if( !PyTuple_Check( obj ) )
			{
				return nullptr;
			}
			if( PyTuple_Size( obj ) != 2 )
			{
				return nullptr;
			}
			if( !PyString_Check( PyTuple_GetItem( obj, 0 ) ) )
			{
				return nullptr;
			}
			if( !PyString_Check( PyTuple_GetItem( obj, 1 ) ) )
			{
				return nullptr;
			}
			return obj;
		}

		static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
		{
			InternedString shader = extract<InternedString>( PyTuple_GetItem( obj, 0 ) );
			InternedString name = extract<InternedString>( PyTuple_GetItem( obj, 1 ) );

			void *storage = ( (converter::rvalue_from_python_storage<ShaderNetwork::Parameter>*) data )->storage.bytes;
			new( storage ) ShaderNetwork::Parameter( shader, name );
			data->convertible = storage;
		}

};

struct ConnectionFromTuple
{

	static void registerConverter()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<ShaderNetwork::Connection>()
		);
	}

	private :

		static void *convertible( PyObject *obj )
		{
			if( !PyTuple_Check( obj ) )
			{
				return nullptr;
			}
			if( PyTuple_Size( obj ) != 2 )
			{
				return nullptr;
			}
			if( !extract<ShaderNetwork::Parameter>( ( PyTuple_GetItem( obj, 0 ) ) ).check() )
			{
				return nullptr;
			}
			if( !extract<ShaderNetwork::Parameter>( ( PyTuple_GetItem( obj, 1 ) ) ).check() )
			{
				return nullptr;
			}
			return obj;
		}

		static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
		{
			ShaderNetwork::Parameter source = extract<ShaderNetwork::Parameter>( PyTuple_GetItem( obj, 0 ) );
			ShaderNetwork::Parameter destination = extract<ShaderNetwork::Parameter>( PyTuple_GetItem( obj, 1 ) );

			void *storage = ( (converter::rvalue_from_python_storage<ShaderNetwork::Connection>*) data )->storage.bytes;
			new( storage ) ShaderNetwork::Connection( source, destination );
			data->convertible = storage;
		}

};

} // namespace

void IECoreSceneModule::bindShaderNetwork()
{

	scope shaderNetworkScope = RunTimeTypedClass<ShaderNetwork>()
		.def( init<>() )
		.def( "__init__", make_constructor( constructor, default_call_policies(), ( arg( "shaders" ) = dict(), arg( "connections" ) = list(), arg( "output" ) = object() ) ) )
		.def( "addShader", &ShaderNetwork::addShader, ( arg( "handle" ), arg( "shader" ) ) )
		.def( "setShader", &ShaderNetwork::setShader, ( arg( "handle" ), arg( "shader" ) ) )
		.def( "getShader", (Shader *(ShaderNetwork::*)( const InternedString & ))&ShaderNetwork::getShader, return_value_policy<IECorePython::CastToIntrusivePtr>() )
		.def( "removeShader", (void (ShaderNetwork::*)( const InternedString & ))&ShaderNetwork::removeShader )
		.def( "shaders", &shaders )
		.def( "getOutput", &ShaderNetwork::getOutput, return_value_policy<copy_const_reference>() )
		.def( "setOutput", &ShaderNetwork::setOutput )
		.def( "outputShader", (Shader *(ShaderNetwork::*)())&ShaderNetwork::outputShader, return_value_policy<IECorePython::CastToIntrusivePtr>() )
		.def( "size", &ShaderNetwork::size )
		.def( "__len__", &ShaderNetwork::size )
		.def( "addConnection", &ShaderNetwork::addConnection )
		.def( "removeConnection", &ShaderNetwork::removeConnection )
		.def( "input", &ShaderNetwork::input )
		.def( "inputConnections", &inputConnections )
		.def( "outputConnections", &outputConnections )
	;

	class_<ShaderNetwork::Parameter>( "Parameter" )
		.def( init<const InternedString &, const InternedString &>( ( arg( "shader" ), arg( "name" ) = "" ) ) )
		.add_property( "shader", &parameterShaderGet, &parameterShaderSet )
		.add_property( "name", &parameterNameGet, &parameterNameSet )
		.def( self == self )
		.def( self != self )
		.def( repr( self ) )
		.def( !self )
	;

	class_<ShaderNetwork::Connection>( "Connection" )
		.def( init<const ShaderNetwork::Parameter &, const ShaderNetwork::Parameter &>() )
		.def_readwrite( "source", &ShaderNetwork::Connection::source )
		.def_readwrite( "destination", &ShaderNetwork::Connection::destination )
		.def( self == self )
		.def( self != self )
		.def( repr( self ) )
	;

	ParameterFromString::registerConverter();
	ParameterFromTuple::registerConverter();
	ConnectionFromTuple::registerConverter();

}
