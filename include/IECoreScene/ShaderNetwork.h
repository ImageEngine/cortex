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

#ifndef IECORESCENE_SHADERNETWORK_H
#define IECORESCENE_SHADERNETWORK_H

#include "IECoreScene/Shader.h"
#include "IECoreScene/TypeIds.h"
#include "IECore/CompoundObject.h"

#include "boost/range/iterator_range_core.hpp"

namespace IECoreScene
{

/// Contains a collection of `Shader` objects and maintains connections between them.
class IECORESCENE_API ShaderNetwork : public IECore::BlindDataHolder
{

	public :

		ShaderNetwork();
		~ShaderNetwork();

		IE_CORE_DECLAREEXTENSIONOBJECT( ShaderNetwork, ShaderNetworkTypeId, IECore::BlindDataHolder );

		/// Shader accessors
		/// ----------------
		///
		/// Each shader in the network is identified by a unique string handle.
		/// The ShaderNetwork is the sole owner of the contained shaders, and
		/// provides only const access to them. This allows various internal
		/// optimisations to be made. Shader parameters are specified by filling
		/// `Shader::parameters()` before calling `addShader()` or `setShader()`.
		/// Subsequent modifications must be made by calling `getShader()` and
		/// reinserting a modified copy via `setShader()`.

		class IECORESCENE_API ShaderIterator;

		/// Adds a shader, uniquefying the handle if necessary to avoid clashes with
		/// existing shaders in the network. Returns the handle used. A copy of the
		/// shader is taken so that subsequent modifications to it will not affect
		/// the network.
		IECore::InternedString addShader( const IECore::InternedString &handle, const Shader *shader );
		/// As above, but without the overhead of copying `shader`. The caller must be the sole
		/// owner of `shader`, and may not modify it following the call.
		///
		/// ```
		/// ShaderPtr shader = new Shader();
		/// network->addShader( "handle", std::move( shader ) );
		/// assert( shader == nullptr );
		/// ```
		IECore::InternedString addShader( const IECore::InternedString &handle, ShaderPtr &&shader );
		/// Sets the shader with the named handle. Replaces any existing shader with the same
		/// handle. A copy of the shader is taken, so subsequent modifications to it will
		/// not affect the network.
		void setShader( const IECore::InternedString &handle, const Shader *shader );
		/// As above, but without the overhead of copying `shader`. The caller must be the sole
		/// owner of `shader`, and may not modify it following the call.
		void setShader( const IECore::InternedString &handle, ShaderPtr &&shader );
		const Shader *getShader( const IECore::InternedString &handle ) const;
		void removeShader( const IECore::InternedString &handle );
		ShaderIterator removeShader( const ShaderIterator &iterator );

		/// Returns the number of shaders.
		size_t size() const;

		/// Shader iteration
		/// ----------------
		///
		/// Access to all shaders is provided by the iterator range returned
		/// by the `shaders()` method. Iterators remain valid following calls
		/// to `addShader()/removeShader()` (except for the iterator corresponding
		/// to the removed shader).
		///
		/// ```
		/// for( const auto &s : network->shaders() )
		/// {
		/// 	InternedString handle = s.first;
		/// 	const Shader *shader = s.second.get();
		/// }
		/// ```

		using ShaderRange = boost::iterator_range<ShaderIterator>;

		ShaderRange shaders() const;

		/// Connections
		/// -----------
		///
		/// Shaders within the network are joined by specifying connections
		/// between their parameters. These connections are managed via the
		/// methods below. When shaders are removed from the network, their
		/// connections are automatically removed too.

		/// Represents the endpoint for a connection.
		struct IECORESCENE_API Parameter
		{
			Parameter() = default;
			Parameter( const IECore::InternedString &shader, const IECore::InternedString &name = "" );

			IECore::InternedString shader;
			IECore::InternedString name;

			/// Returns false if shader and name are empty.
			operator bool () const;

		};

		/// Represents a connection between shader parameters
		struct IECORESCENE_API Connection
		{
			Connection() = default;
			Connection( const Parameter &source, const Parameter &destination );

			Parameter source;
			Parameter destination;
		};

		void addConnection( const Connection &connection );
		void removeConnection( const Connection &connection );

		/// Returns an empty Parameter if no input exists.
		Parameter input( const Parameter &destination ) const;

		/// Connection iteration
		/// --------------------
		///
		/// All input and output connections for a given shader may
		/// be accessed via the iterator ranges returned by `inputConnections()`
		/// and `outputConnections()`. Iterators are not invalidated by calls
		/// to `addConnection()/removeConnection()` (except the iterator to the
		/// removed connection).
		///
		/// ```
		/// for( const ShaderNetwork::Connection &c : network->inputConnections( shader ) )
		/// {
		/// 	...
		/// }
		/// ```

		class IECORESCENE_API ConnectionIterator;
		using ConnectionRange = boost::iterator_range<ConnectionIterator>;

		ConnectionRange inputConnections( const IECore::InternedString &handle ) const;
		ConnectionRange outputConnections( const IECore::InternedString &handle ) const;

		/// Output
		/// ------
		///
		/// The output shader is the final shader in the network, the one
		/// that should be assigned to objects. The output is defined as
		/// a Parameter so that texture networks can be defined with a
		/// particular output from a multi-output shader.

		void setOutput( const Parameter &output );
		const Parameter &getOutput() const;

		/// Convenience returning `getShader( getOutput().shader )`
		const Shader *outputShader() const;

		/// String Substitutions
		/// --------------------
		///
		/// We support special syntax that allows you to substitute string attributes
		/// into the values of string parameters on shaders.
		/// 
		/// If a string parameter, or string vector parameter, contains the token
		/// <attr:PARAMETER_NAME>, then it will be subsituted with the value of a
		/// string attribute named PARAMETER_NAME.  If there is no attribute named
		/// PARAMETER_NAME, the token will be replaced with an empty string.
		///
		/// If you wish to output a literal string containing "<attr:PARAMETER_NAME>",
		/// ( for example because you want to use Arnold's render time substitution ),
		/// you can escape the angle brackets with backslashes, like
		/// "\<attr:PARAMETER_NAME\>"

		/// Appends all attributes used by `applySubstitutions()` into the hash.
		void hashSubstitutions( const IECore::CompoundObject *attributes, IECore::MurmurHash &h ) const;

		/// Apply substitutions to all string and string vector parameters in the network,
		/// based on the provided attributes.
		void applySubstitutions( const IECore::CompoundObject *attributes );

	private :

		class Implementation;
		std::unique_ptr<Implementation> m_implementation;

		// Accessors for implementation. Direct access through
		// `m_implementation` is wrong because it does not propagate
		// const-ness. Someday we may be able to use `std::propagate_const`
		// instead.
		Implementation *implementation();
		const Implementation *implementation() const;

};

IE_CORE_DECLAREPTR( ShaderNetwork )

} // namespace IECoreScene

#include "IECoreScene/ShaderNetwork.inl"

#endif // IECORESCENE_SHADERNETWORK_H
