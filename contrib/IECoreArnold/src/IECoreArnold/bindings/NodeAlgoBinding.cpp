//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "IECoreArnold/bindings/NodeAlgoBinding.h"

#include "IECoreArnold/NodeAlgo.h"

#include "IECore/Object.h"

#include "boost/format.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

using namespace boost::python;
using namespace IECoreArnold;
using namespace IECoreArnoldBindings;

namespace
{

AtUniverse *pythonObjectToAtUniverse( const boost::python::object &universe )
{
	if( universe.is_none() )
	{
		return nullptr;
	}

	const std::string className = extract<std::string>( universe.attr( "__class__" ).attr( "__name__" ) );
	if( className != "LP_AtUniverse" )
	{
		throw IECore::Exception( boost::str( boost::format( "%1% is not an AtUniverse" ) % className ) );
	}

	object ctypes = import( "ctypes" );
	object address = ctypes.attr( "addressof" )( object( universe.attr( "contents" ) ) );

	return reinterpret_cast<AtUniverse *>( extract<size_t>( address )() );
}

object convertWrapper( const IECore::Object *object, boost::python::object universe, const std::string &nodeName )
{
	return atNodeToPythonObject( NodeAlgo::convert( object, pythonObjectToAtUniverse( universe ), nodeName, nullptr ) );
}

object convertWrapper2( object pythonSamples, float motionStart, float motionEnd, boost::python::object universe, const std::string &nodeName )
{
	std::vector<const IECore::Object *> samples;
	container_utils::extend_container( samples, pythonSamples );

	return atNodeToPythonObject( NodeAlgo::convert( samples, motionStart, motionEnd, pythonObjectToAtUniverse( universe ), nodeName, nullptr ) );
}

} // namespace

namespace IECoreArnoldBindings
{

boost::python::object atNodeToPythonObject( AtNode *node )
{
	if( !node )
	{
		return object();
	}

	object ctypes = import( "ctypes" );
	object arnold = import( "arnold" );

	object atNodeType = arnold.attr( "AtNode" );
	object pointerType = ctypes.attr( "POINTER" )( atNodeType );
	object converted = ctypes.attr( "cast" )( (size_t)node, pointerType );
	return converted;
}

void bindNodeAlgo()
{

	object nodeAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreArnold.NodeAlgo" ) ) ) );
	scope().attr( "NodeAlgo" ) = nodeAlgoModule;
	scope nodeAlgoModuleScope( nodeAlgoModule );

	def( "convert", &convertWrapper );
	def( "convert", &convertWrapper2 );

}

} // namespace IECoreArnoldBindings
