//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#ifndef IECORESCENE_SHADERNETWORKALGO_H
#define IECORESCENE_SHADERNETWORKALGO_H

#include "IECore/ObjectVector.h"

#include "IECoreScene/ShaderNetwork.h"

namespace IECoreScene
{

namespace ShaderNetworkAlgo
{

/// Adds all shaders from `sourceNetwork` into `network`, renaming their handles as necessary to
/// preserve uniqueness. If the `connections` parameter is `true`, then the corresponding connections
/// are also copied over. Returns the parameter within `destinationNetwork` that corresponds to
/// `sourceNetwork->getOutput()`.
IECORESCENE_API ShaderNetwork::Parameter addShaders( ShaderNetwork *network, const ShaderNetwork *sourceNetwork, bool connections = true );

/// Removes any shaders which are not eventually connected to `network->getOutput()`.
IECORESCENE_API void removeUnusedShaders( ShaderNetwork *network );

/// Performs a depth-first traversal of the upstream network by following input connections from `shader`.
/// The `visitor` functor is called exactly once for each shader encountered and should have signature
/// `visitor( const ShaderNetwork *network, const IECore::InternedString &shaderHandle )`. If the `shader`
/// parameter is not specified, it defaults to `network->getOutput().shader`.
template<typename Visitor>
void depthFirstTraverse( const ShaderNetwork *network, Visitor &&visitor, IECore::InternedString shader = "" );

/// Finds connections involving the individual components of point/color parameters, and converts them
/// for use with OSL. The `oslVersion` argument is used to determine how conversion is performed,
/// and should be passed a value of `OSL_VERSION`. For OSL prior to 1.10, intermediate shaders are
/// inserted to emulate connections between components. For later versions, no new shaders are inserted, but
/// components are renamed from our `.x, .y, .z` suffixes to OSL's `[0], [1], [2]` suffixes.
/// \todo Remove the version without the `oslVersion` argument.
IECORESCENE_API void convertOSLComponentConnections( ShaderNetwork *network );
IECORESCENE_API void convertOSLComponentConnections( ShaderNetwork *network, int oslVersion );

/// Converts from the legacy ObjectVector format previously used to represent shader networks.
IECORESCENE_API ShaderNetworkPtr convertObjectVector( const IECore::ObjectVector *network );

} // namespace ShaderNetworkAlgo

} // namespace IECoreScene

#include "IECoreScene/ShaderNetworkAlgo.inl"

#endif // IECORESCENE_SHADERNETWORKALGO_H
