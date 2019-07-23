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
#include "boost/python/stl_iterator.hpp"

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
		const Shader &shader = extract<const Shader &>( items[i][1] )();
		result->addShader( handle, &shader );
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

const char *addShader( ShaderNetwork &network, const IECore::InternedString &handle, const Shader &shader )
{
	return network.addShader( handle, &shader ).c_str();
}

void setShader( ShaderNetwork &network, const IECore::InternedString &handle, const Shader &shader )
{
	network.setShader( handle, &shader );
}

ShaderPtr getShader( const ShaderNetwork &network, const IECore::InternedString &handle )
{
	ConstShaderPtr s = network.getShader( handle );
	return s ? s->copy() : nullptr;
}

boost::python::dict shaders( ShaderNetwork &network )
{
	boost::python::dict result;
	for( const auto &s : network.shaders() )
	{
		result[s.first.c_str()] = s.second->copy();
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

ShaderPtr outputShader( const ShaderNetwork &network )
{
	ConstShaderPtr s = network.outputShader();
	return s ? s->copy() : nullptr;
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

void testShaderNetworkMove()
{
	ShaderNetworkPtr shaderNetwork = new ShaderNetwork;

	// Test move-based `addShader()`

	ShaderPtr shader = new Shader;
	const Shader *rawShader = shader.get(); // For test purposes only
	shaderNetwork->addShader( "s1", std::move( shader ) );
	if( shaderNetwork->getShader( "s1" ) != rawShader )
	{
		throw Exception( "ShaderNetwork::addShader() : Shader not referenced directly" );
	}
	if( shader.get() )
	{
		throw Exception( "ShaderNetwork::addShader() : ShaderPtr not reset" );
	}
	if( rawShader->refCount() != 1 )
	{
		throw Exception( "ShaderNetwork::addShader() : Shader ownership is not unique" );
	}

	// Test move-based `setShader()`

	shader = new Shader;
	rawShader = shader.get(); // For test purposes only
	shaderNetwork->setShader( "s2", std::move( shader ) );
	if( shaderNetwork->getShader( "s2" ) != rawShader )
	{
		throw Exception( "ShaderNetwork::setShader() : Shader not referenced directly" );
	}
	if( shader.get() )
	{
		throw Exception( "ShaderNetwork::setShader() : ShaderPtr not reset" );
	}
	if( rawShader->refCount() != 1 )
	{
		throw Exception( "ShaderNetwork::setShader() : Shader ownership is not unique" );
	}

	// Check that it is an error to use the move-based methods
	// if you don't have sole ownership.

	shader = new Shader;
	ShaderPtr shaderSharer = shader;
	bool errored = false;
	try
	{
		shaderNetwork->addShader( "s3", std::move( shader ) );
	}
	catch( ... )
	{
		errored = true;
	}

	if( !errored )
	{
		throw Exception( "ShaderNetwork : Sole ownership was not enforced" );
	}

}

} // namespace

void IECoreSceneModule::bindShaderNetwork()
{

	def( "testShaderNetworkMove", &testShaderNetworkMove );

	scope shaderNetworkScope = RunTimeTypedClass<ShaderNetwork>()
		.def( init<>() )
		.def( "__init__", make_constructor( constructor, default_call_policies(), ( arg( "shaders" ) = dict(), arg( "connections" ) = list(), arg( "output" ) = object() ) ) )
		.def( "addShader", &addShader, ( arg( "handle" ), arg( "shader" ) ) )
		.def( "setShader", &setShader, ( arg( "handle" ), arg( "shader" ) ) )
		.def( "getShader", &getShader )
		.def( "removeShader", (void (ShaderNetwork::*)( const InternedString & ))&ShaderNetwork::removeShader )
		.def( "shaders", &shaders )
		.def( "getOutput", &ShaderNetwork::getOutput, return_value_policy<copy_const_reference>() )
		.def( "setOutput", &ShaderNetwork::setOutput )
		.def( "outputShader", &outputShader )
		.def( "size", &ShaderNetwork::size )
		.def( "__len__", &ShaderNetwork::size )
		.def( "addConnection", &ShaderNetwork::addConnection )
		.def( "removeConnection", &ShaderNetwork::removeConnection )
		.def( "input", &ShaderNetwork::input )
		.def( "inputConnections", &inputConnections )
		.def( "outputConnections", &outputConnections )
		.def( "hashSubstitutions", &ShaderNetwork::hashSubstitutions )
		.def( "applySubstitutions", &ShaderNetwork::applySubstitutions )
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
