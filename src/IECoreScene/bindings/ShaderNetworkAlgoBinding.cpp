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
#include "boost/pointer_cast.hpp"

#include "ShaderNetworkAlgoBinding.h"

#include "IECoreScene/ShaderNetworkAlgo.h"

using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;

namespace
{

void convertOSLComponentConnectionsWrapper( ShaderNetwork *network, int oslVersion )
{
	ShaderNetworkAlgo::convertOSLComponentConnections( network, oslVersion );
}

CompoundDataPtr collapseSplineParametersWrapper( CompoundDataPtr parameters )
{
	return boost::const_pointer_cast< CompoundData >( ShaderNetworkAlgo::collapseSplineParameters( parameters ) );
}

CompoundDataPtr expandSplineParametersWrapper( CompoundDataPtr parameters )
{
	return boost::const_pointer_cast< CompoundData >( ShaderNetworkAlgo::expandSplineParameters( parameters ) );
}

std::string componentConnectionAdapterLabelWrapper()
{
	return ShaderNetworkAlgo::componentConnectionAdapterLabel().string();
}

} // namespace

void IECoreSceneModule::bindShaderNetworkAlgo()
{
	object module( borrowed( PyImport_AddModule( "IECoreScene.ShaderNetworkAlgo" ) ) );
	scope().attr( "ShaderNetworkAlgo" ) = module;
	scope moduleScope( module );

	def( "addShaders", &ShaderNetworkAlgo::addShaders, ( arg( "network" ), arg( "sourceNetwork" ), arg( "connections" ) = true ) );
	def( "removeUnusedShaders", &ShaderNetworkAlgo::removeUnusedShaders );
	def( "addComponentConnectionAdapters", &ShaderNetworkAlgo::addComponentConnectionAdapters, ( arg( "network" ), arg( "targetPrefix" ) = "" ) );
	def( "removeComponentConnectionAdapters", &ShaderNetworkAlgo::removeComponentConnectionAdapters, ( arg( "network" ) ) );
	def( "componentConnectionAdapterLabel", &componentConnectionAdapterLabelWrapper );
	def( "convertToOSLConventions", &ShaderNetworkAlgo::convertToOSLConventions );
	def( "convertOSLComponentConnections", &convertOSLComponentConnectionsWrapper, ( arg( "network" ), arg( "oslVersion" ) = 10900 ) );
	def( "convertObjectVector", &ShaderNetworkAlgo::convertObjectVector );
	def( "collapseSplines", &ShaderNetworkAlgo::collapseSplines, ( arg( "network" ), arg( "targetPrefix" ) = "" ) );
	def( "expandSplines", &ShaderNetworkAlgo::expandSplines, ( arg( "network" ), arg( "targetPrefix" ) = "" ) );
	def( "collapseSplineParameters", &collapseSplineParametersWrapper );
	def( "expandSplineParameters", &expandSplineParametersWrapper );

}
