//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreImage/DisplayDriverServer.h"
#include "IECoreImageBindings/DisplayDriverServerBinding.h"

using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreImage;

namespace
{

void setPortRange( boost::python::tuple range )
{
	DisplayDriverServer::setPortRange( { extract<DisplayDriverServer::Port>( range[0] ), extract<DisplayDriverServer::Port>( range[1] ) } );
}

boost::python::tuple getPortRange()
{
	auto range = DisplayDriverServer::getPortRange();
	return boost::python::make_tuple( range.first, range.second );
}

void registerPortRange( const std::string &name, boost::python::tuple range )
{
	DisplayDriverServer::registerPortRange( name, { extract<DisplayDriverServer::Port>( range[0] ), extract<DisplayDriverServer::Port>( range[1] ) } );
}


boost::python::tuple registeredPortRange( const std::string &name )
{
	auto range = DisplayDriverServer::registeredPortRange( name );
	return boost::python::make_tuple( range.first, range.second );
}

} // namespace

namespace IECoreImageBindings
{

void bindDisplayDriverServer()
{
	using boost::python::arg;

	RunTimeTypedClass<DisplayDriverServer>()
		.def( init<DisplayDriverServer::Port>( ( arg( "portNumber" ) = 0 ) ) )
		.def( "portNumber", &DisplayDriverServer::portNumber )
		.def( "setPortRange", &::setPortRange ).staticmethod( "setPortRange" )
		.def( "getPortRange", &::getPortRange ).staticmethod( "getPortRange" )
		.def( "registerPortRange", &::registerPortRange ).staticmethod( "registerPortRange" )
		.def( "deregisterPortRange", &DisplayDriverServer::deregisterPortRange ).staticmethod( "deregisterPortRange" )
		.def( "registeredPortRange", &registeredPortRange ).staticmethod( "registeredPortRange" )
	;

}

} // namespace IECoreImageBindings
