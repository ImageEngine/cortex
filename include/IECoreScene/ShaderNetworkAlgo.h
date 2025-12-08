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

/// Replaces connections between sub components of colors or vectors with
/// connections to whole parameters on adapter shaders. If `targetPrefix` is
/// given, only translates connections to shaders with a type starting with this
/// string.
IECORESCENE_API void addComponentConnectionAdapters( ShaderNetwork *network, std::string targetPrefix = "" );

/// Finds adapters that were created by addComponentConnectionAdapters, and
/// removes them, replacing them with the original component connections.
IECORESCENE_API void removeComponentConnectionAdapters( ShaderNetwork *network );

/// Registers an adapter to split a component from a color or vector output, ready for connection into
/// a scalar input. Used by `addComponentConnectionAdapters()`.
///
/// - `destinationShaderType` : The type prefix for the shader receiving the connection - e.g. "ai", "osl".
/// - `component` : "r", "g", "b", "a", "x", "y", or "z".
/// - `adapter` : The shader to be used as the adapter.
/// - `inParameter` : The parameter that receives the color or vector input.
/// - `outParameter` : The parameter that outputs the component.
IECORESCENE_API void registerSplitAdapter( const std::string &destinationShaderType, IECore::InternedString component, const IECoreScene::Shader *adapter, IECore::InternedString inParameter, IECore::InternedString outParameter );
/// Removes an adapter registration.
IECORESCENE_API void deregisterSplitAdapter( const std::string &destinationShaderType, IECore::InternedString component );

/// Registers an adapter to join multiple scalar components into a color or vector output. Used by `addComponentConnectionAdapters()`.
///
/// - `destinationShaderType` : The type prefix for the shader receiving the connection - e.g. "ai", "osl".
/// - `destinationParameterType` : `(V2i|V3i|V2f|V3f|Color3f|Color4f)DataTypeId`.
/// - `inParameters` : The parameters that receives the individual components of the vector or color.
/// - `outParameter` : The parameter that outputs the vector or color.
IECORESCENE_API void registerJoinAdapter( const std::string &destinationShaderType, IECore::TypeId destinationParameterType, const IECoreScene::Shader *adapter, const std::array<IECore::InternedString, 4> &inParameters, IECore::InternedString outParameter );
/// Removes an adapter registration.
IECORESCENE_API void deregisterJoinAdapter( const std::string &destinationShaderType, IECore::TypeId destinationParameterType );

/// Converts various aspects of how shaders are stored to be ready to pass directly to OSL.
/// The `oslVersion` argument is used to determine how conversion is performed, and should be passed a
/// value of `OSL_VERSION`. Conversions include:
///
/// - Connections involving the individual components of point/color parameters.
///     For OSL prior to 1.10, intermediate shaders are inserted to emulate connections between components.
///     For later versions, no new shaders are inserted, but components are renamed from our `.x, .y, .z`
///     suffixes to OSL's `[0], [1], [2]` suffixes.
/// - Ramps
///     We support RampData as a parameter type. For OSL, these must be converted to 3 parameters named
///     `<rampName>Positions`, `<rampName>Values` and `<rampName>Basis`. We also support input
///     connections to ramp Y values, specified as `<rampName>[N].y`, which currently must be implemented
///     using an adapter shader.
IECORESCENE_API void convertToOSLConventions( ShaderNetwork *network, int oslVersion );

/// Converts from the legacy ObjectVector format previously used to represent shader networks.
IECORESCENE_API ShaderNetworkPtr convertObjectVector( const IECore::ObjectVector *network );

/// We use a convention where ramps are represented by a single RampData in Cortex, but must be expanded
/// out into basic types when being passed to a renderer. We need two functions to convert back and forth.

/// Look throughout the network for parameters matching our ramp convention, for any possible <prefix> :
/// <prefix>Positions, a float vector parameter
/// <prefix>Values, a vector of a value type, such as float or color
/// <prefix>Basis, a string parameter
/// For each set of parameters found matching this convention, the 3 parameters will be replaced with one
/// ramp parameter named <prefix>. If input connections are represented using an adapter shader, they
/// will be converted to direct connections to the ramp using our support for ramp element
/// connections.
/// If `targetPrefix` is given, only translates connections to shaders with a type starting with this string
IECORESCENE_API void collapseRamps( ShaderNetwork *network, std::string targetPrefix = "" );

/// Look throughout the network for ramp parameters. If any are found, they will be expanded out into
/// 3 parameters named <name>Positions, <name>Values and <name>Basis.
/// We also support input connections to ramp Y values, specified as `<rampName>[N].y`, which currently
/// must be implemented by inserting an adapter shader.
/// If `targetPrefix` is given, only translates connections to shaders with a type starting with this string
IECORESCENE_API void expandRamps( ShaderNetwork *network, std::string targetPrefix = "" );

/// Used when dealing with SCC files written before Cortex 10.7, which used Spline*Data instead of Ramp*Data
IECORESCENE_API void convertDeprecatedSplines( ShaderNetwork *network );

} // namespace ShaderNetworkAlgo

} // namespace IECoreScene

#include "IECoreScene/ShaderNetworkAlgo.inl"
#endif // IECORESCENE_SHADERNETWORKALGO_H
