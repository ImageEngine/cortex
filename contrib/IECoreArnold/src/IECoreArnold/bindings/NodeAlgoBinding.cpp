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

#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/bindings/NodeAlgoBinding.h"

#include "IECore/Object.h"

using namespace boost::python;
using namespace IECoreArnold;
using namespace IECoreArnoldBindings;

namespace
{

object convertWrapper( const IECore::Object *object )
{
	return atNodeToPythonObject( NodeAlgo::convert( object ) );
}

object convertWrapper2( object pythonSamples, object pythonSampleTimes )
{
	std::vector<const IECore::Object *> samples;
	std::vector<float> sampleTimes;
	container_utils::extend_container( samples, pythonSamples );
	container_utils::extend_container( sampleTimes, pythonSampleTimes );

	return atNodeToPythonObject( NodeAlgo::convert( samples, sampleTimes ) );
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
