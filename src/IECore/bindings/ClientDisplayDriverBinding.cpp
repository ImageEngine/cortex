//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ClientDisplayDriver.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

template< typename T >
std::vector< T > listToVector( const boost::python::list &names )
{
	std::vector< T > n;
	boost::python::container_utils::extend_container( n, names );
	return n;
}

static ClientDisplayDriverPtr clientDisplayDriverConstructor( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const list &channelNames, CompoundDataPtr parameters )
{
	return new ClientDisplayDriver( displayWindow, dataWindow, listToVector<std::string>( channelNames ), parameters );
}

void bindClientDisplayDriver()
{
	RunTimeTypedClass<ClientDisplayDriver>()
		.def( "__init__", make_constructor( &clientDisplayDriverConstructor, default_call_policies(), ( boost::python::arg_( "displayWindow" ), boost::python::arg_( "dataWindow" ), boost::python::arg_( "channelNames" ), boost::python::arg_( "parameters" ) ) ) )
		.def( "host", &ClientDisplayDriver::host )
		.def( "port", &ClientDisplayDriver::port )
	;
}

} // namespace IECore
