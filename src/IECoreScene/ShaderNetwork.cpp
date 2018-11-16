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

#include "IECoreScene/ShaderNetwork.h"

#include "boost/lexical_cast.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index_container.hpp"

#include <mutex>

using namespace std;
using namespace boost;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// ShaderNetwork::Implementation
//////////////////////////////////////////////////////////////////////////

class ShaderNetwork::Implementation
{

	public :

		IECore::InternedString addShader( IECore::InternedString handle, const IECoreScene::ShaderPtr &shader )
		{
			handle = uniqueHandle( handle );
			const bool inserted = m_nodes.insert( { handle, shader } ).second;
			assert( inserted ); (void)inserted;
			if( m_nodes.size() == 1 )
			{
				m_output = Parameter( handle, "" );
			}
			m_hashDirty = true;
			return handle;
		}

		void setShader( const IECore::InternedString &handle, const IECoreScene::ShaderPtr &shader )
		{
			m_nodes.insert( { handle, shader } ).first->mutableShader() = shader;
			m_hashDirty = true;
		}

		IECoreScene::Shader *getShader( const IECore::InternedString &handle )
		{
			auto it = m_nodes.find( handle );
			if( it == m_nodes.end() )
			{
				return nullptr;
			}
			m_hashDirty = true; // Assume shader will be modified
			return it->shader.get();
		}

		const IECoreScene::Shader *getShader( const IECore::InternedString &handle ) const
		{
			auto it = m_nodes.find( handle );
			if( it == m_nodes.end() )
			{
				return nullptr;
			}
			return it->shader.get();
		}

		void removeShader( const IECore::InternedString &handle )
		{
			auto it = m_nodes.find( handle );
			if( it == m_nodes.end() )
			{
				throw IECore::Exception( "Shader not in network" );
			}
			removeShader( it );
		}

		ShaderIterator removeShader( const ShaderIterator &iterator )
		{
			auto internalIt = m_nodes.iterator_to( *static_cast<const Node *>( iterator.m_node ) );
			internalIt = removeShader( internalIt );
			return ShaderIterator( &m_nodes, internalIt != m_nodes.end() ? &*internalIt : nullptr );
		}

		size_t size() const
		{
			return m_nodes.size();
		}

		ShaderNetwork::ShaderRange shaders()
		{
			m_hashDirty = true; // Assume shaders will be modified
			return ShaderRange(
				ShaderIterator( &m_nodes, m_nodes.size() ? &*m_nodes.begin() : nullptr ),
				ShaderIterator( &m_nodes, nullptr )
			);
		}

		ShaderNetwork::ConstShaderRange shaders() const
		{
			return ConstShaderRange(
				ConstShaderIterator( &m_nodes, m_nodes.size() ? &*m_nodes.begin() : nullptr ),
				ConstShaderIterator( &m_nodes, nullptr )
			);
		}

		void addConnection( const Connection &connection )
		{
			auto sourceIt = m_nodes.find( connection.source.shader );
			if( sourceIt == m_nodes.end() )
			{
				throw IECore::Exception( "Source shader not in network" );
			}

			auto destinationIt = m_nodes.find( connection.destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( "Destination shader not in network" );
			}

			Connection c = connection;
			bool inserted = destinationIt->mutableInputConnections().insert( c ).second;
			if( !inserted )
			{
				throw IECore::Exception( "Destination parameter already has an input" );
			}
			inserted = sourceIt->mutableOutputConnections().insert( c ).second;
			assert( inserted );

			m_hashDirty = true;
		}

		void removeConnection( const Connection &connection )
		{
			auto destinationIt = m_nodes.find( connection.destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( "Destination shader not in network" );
			}

			auto connectionIt = destinationIt->inputConnections.find( connection.destination );
			if( connectionIt == destinationIt->inputConnections.end() || *connectionIt != connection )
			{
				throw IECore::Exception( "Connection not in network" );
			}

			destinationIt->mutableInputConnections().erase( connectionIt );

			auto sourceIt = m_nodes.find( connection.source.shader );
			assert( sourceIt != m_nodes.end() );
			sourceIt->mutableOutputConnections().erase( connection.destination );
		}

		Parameter input( const Parameter &destination ) const
		{
			auto destinationIt = m_nodes.find( destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( "Destination shader not in network" );
			}
			auto connectionIt = destinationIt->inputConnections.find( destination );
			if( connectionIt == destinationIt->inputConnections.end() )
			{
				return Parameter();
			}
			return connectionIt->source;
		}

		ShaderNetwork::ConnectionRange inputConnections( const IECore::InternedString &handle ) const
		{
			auto it = m_nodes.find( handle );
			if( it == m_nodes.end() )
			{
				throw IECore::Exception( "Destination shader not in network" );
			}

			return ConnectionRange(
				ConnectionIterator( &it->inputConnections, it->inputConnections.size() ? &*it->inputConnections.begin() : nullptr ),
				ConnectionIterator( &it->inputConnections, nullptr )
			);
		}

		ShaderNetwork::ConnectionRange outputConnections( const IECore::InternedString &handle ) const
		{
			auto it = m_nodes.find( handle );
			if( it == m_nodes.end() )
			{
				throw IECore::Exception( "Source shader not in network" );
			}

			return ConnectionRange(
				ConnectionIterator( &it->outputConnections, it->outputConnections.size() ? &*it->outputConnections.begin() : nullptr ),
				ConnectionIterator( &it->outputConnections, nullptr )
			);
		}

		void setOutput( const Parameter &output )
		{
			if( output == m_output )
			{
				return;
			}

			if( m_nodes.find( output.shader ) == m_nodes.end() )
			{
				throw IECore::Exception( "Output shader not in network" );
			}

			m_output = output;
			m_hashDirty = true;
		}

		const Parameter &getOutput() const
		{
			return m_output;
		}

		IECoreScene::Shader *outputShader()
		{
			return getShader( m_output.shader );
		}

		const IECoreScene::Shader *outputShader() const
		{
			return getShader( m_output.shader );
		}

		bool isEqualTo( const Implementation *other ) const
		{
			// Check shaders and connections

			if( other->m_nodes.size() != m_nodes.size() )
			{
				return false;
			}

			for( const auto &node : m_nodes )
			{
				auto it = other->m_nodes.find( node.handle );
				if( it == other->m_nodes.end() )
				{
					return false;
				}

				const Node &otherNode = *it;
				if( !node.shader->isEqualTo( otherNode.shader.get() ) )
				{
					return false;
				}

				for( const auto &connection : node.inputConnections )
				{
					auto otherConnectionIt = otherNode.inputConnections.find(
						connection.destination
					);
					if( otherConnectionIt == otherNode.inputConnections.end() )
					{
						return false;
					}
					if( *otherConnectionIt != connection )
					{
						return false;
					}
				}
			}

			// Check output

			if( m_output != other->m_output )
			{
				return false;
			}

			return true;
		}

		void hash( IECore::MurmurHash &h ) const
		{
			std::unique_lock<std::mutex> lock( m_hashMutex );
			if( m_hashDirty )
			{
				m_hash = MurmurHash();
				for( const auto &node : m_nodes )
				{
					m_hash.append( node.handle );
					node.shader->hash( m_hash );

					for( const auto &connection : node.inputConnections )
					{
						m_hash.append( connection.source.shader );
						m_hash.append( connection.source.name );
						m_hash.append( connection.destination.name );
					}
				}

				m_hash.append( m_output.shader );
				m_hash.append( m_output.name );

				m_hashDirty = false;
			}
			lock.unlock();

			h.append( m_hash );
		}

		void copyFrom( const Implementation *other, IECore::Object::CopyContext *context )
		{
			m_nodes.clear();

			for( const auto &node : other->m_nodes )
			{
				setShader( node.handle, context->copy( node.shader.get() ) );
			}

			for( const auto &node : other->m_nodes )
			{
				for( const auto &connection : node.inputConnections )
				{
					addConnection( connection );
				}
			}

			m_output = other->m_output;

			m_hash = other->m_hash;
			m_hashDirty = other->m_hashDirty;
		}

		void save( IECore::Object::SaveContext *context ) const
		{
			IndexedIOPtr container = context->container( ShaderNetwork::staticTypeName(), g_ioVersion );

			IndexedIOPtr shaders = container->subdirectory( "shaders", IndexedIO::CreateIfMissing );
			IndexedIOPtr connections = container->subdirectory( "connections", IndexedIO::CreateIfMissing );

			int connectionIndex = 0;
			for( const Node &node : m_nodes )
			{
				context->save( node.shader.get(), shaders.get(), node.handle );
				for( const Connection &connection : node.inputConnections )
				{
					InternedString c[4] = {
						connection.source.shader,
						connection.source.name,
						connection.destination.shader,
						connection.destination.name
					};
					connections->write(
						std::to_string( connectionIndex ),
						c, 4
					);
					connectionIndex++;
				}
			}

			InternedString o[2] = { m_output.shader, m_output.name };
			container->write( "output", o, 2 );
		}

		void load( IECore::Object::LoadContextPtr context )
		{
			unsigned int v = g_ioVersion;
			ConstIndexedIOPtr container = context->container( staticTypeName(), v );

			ConstIndexedIOPtr shaders = container->subdirectory( "shaders" );
			IndexedIO::EntryIDList handles;
			shaders->entryIds( handles, IndexedIO::Directory );
			for( const auto &handle : handles )
			{
				ShaderPtr shader = context->load<Shader>( shaders.get(), handle );
				setShader( handle, shader );
			}

			ConstIndexedIOPtr connections = container->subdirectory( "connections" );
			IndexedIO::EntryIDList connectionIndices;
			for( const auto &connectionIndex : connectionIndices )
			{
				InternedString c[4];
				InternedString *cp = c;
				connections->read( connectionIndex, cp, 4 );
				addConnection(
					Connection( Parameter( cp[0], cp[1] ), Parameter( cp[2], cp[3] ) )
				);
			}

			InternedString o[2];
			InternedString *op = o;
			container->read( "output", op, 2 );
			m_output = Parameter( op[0], op[1] );
		}

		void memoryUsage( IECore::Object::MemoryAccumulator &accumulator ) const
		{
			accumulator.accumulate( this, m_nodes.size() * sizeof( Node ) );
			for( const auto &node : m_nodes )
			{
				accumulator.accumulate( node.shader.get() );
			}
		}

	private :

		friend ShaderNetwork::ConnectionIterator;
		friend ShaderNetwork::ShaderIterator;
		friend ShaderNetwork::ConstShaderIterator;

		// Even though we only have one index, we use `multi_index_container` for its
		// `iterator_to()` method, which allows us to provide iterators in our public
		// API while still hiding the internal data structures entirely. The same applies
		// to our `NodeContainer` structure below.
		using Connections = boost::multi_index_container<
			Connection,
			boost::multi_index::indexed_by<
				// Uniquely indexed by destination parameter, since there can only be
				// one connection per destination.
				boost::multi_index::ordered_unique<
					multi_index::member<Connection, Parameter, &Connection::destination>
				>
			>
		>;

		struct Node
		{
			Node( IECore::InternedString handle, const IECoreScene::ShaderPtr &shader )
				:	handle( handle ), shader( shader )
			{
			}

			IECore::InternedString handle;
			IECoreScene::ShaderPtr shader;

			Connections inputConnections;
			Connections outputConnections;

			const std::string &handleKey() const { return handle.string(); }
			const IECoreScene::Shader *shaderKey() const { return shader.get(); }

			// `multi_index_container` only provides const access to elements
			// to avoid keys changing behind its back. But we know that
			// `shader` isn't used as a key, so this cast is kosher.
			ShaderPtr &mutableShader() const
			{
				return const_cast<ShaderPtr &>( shader );
			}
			// As above.
			Connections &mutableInputConnections() const
			{
				return const_cast<Connections &>( inputConnections );
			}
			// As above.
			Connections &mutableOutputConnections() const
			{
				return const_cast<Connections &>( outputConnections );
			}

		};

		using NodeContainer = boost::multi_index_container<
			Node,
			boost::multi_index::indexed_by<
				// Index ordered by string comparison on handle. This provides lookups
				// by handle and gives us a stable order to use when hashing.
				boost::multi_index::ordered_unique<
					multi_index::const_mem_fun<Node, const string &, &Node::handleKey>
				>
			>
		>;

		NodeContainer m_nodes;
		Parameter m_output;

		// We cache the hash of the network here, so
		// repeated calls to our `Object::hash()` implementation
		// are quick. This is important in Renderer backends that
		// want to use the hash to index into a cache of converted
		// shaders.
		mutable IECore::MurmurHash m_hash;
		// Tracks whether or not the hash is up to date.
		mutable bool m_hashDirty = true;
		// Used to avoid multiple threads trying to update the
		// hash at the same time.
		mutable std::mutex m_hashMutex;

		InternedString uniqueHandle( const InternedString &handle )
		{
			if( m_nodes.find( handle ) == m_nodes.end() )
			{
				return handle;
			}

			string result = handle;
			for( int i = 1; true; ++i )
			{
				result = result + std::to_string( i );
				if( m_nodes.find( result ) == m_nodes.end() )
				{
					return result;
				}
			}
		}

		NodeContainer::iterator removeShader( NodeContainer::iterator it )
		{
			if( m_output.shader == it->handle )
			{
				m_output = Parameter();
			}

			for( const auto &c : it->inputConnections )
			{
				auto sourceIt = m_nodes.find( c.source.shader );
				assert( sourceIt != m_nodes.end() );
				sourceIt->mutableOutputConnections().erase( c.destination );
			}

			for( const auto &c : it->outputConnections )
			{
				auto destinationIt = m_nodes.find( c.destination.shader );
				assert( destinationIt != m_nodes.end() );
				destinationIt->mutableInputConnections().erase( c.destination );
			}

			m_hashDirty = true;
			return m_nodes.erase( it );
		}

		static unsigned int g_ioVersion;

};

unsigned int ShaderNetwork::Implementation::g_ioVersion = 0;

//////////////////////////////////////////////////////////////////////////
// ShaderNetwork iterators
//////////////////////////////////////////////////////////////////////////

void ShaderNetwork::ConnectionIterator::increment()
{
	auto container = static_cast<const Implementation::Connections *>( m_container );
	auto it = container->iterator_to( *m_connection );
	++it;
	m_connection = it != container->end() ? &*it : nullptr;
}

template<typename ShaderPtrType>
void ShaderNetwork::ShaderIteratorT<ShaderPtrType>::increment()
{
	auto container = static_cast<const Implementation::NodeContainer *>( m_container );
	auto it = container->iterator_to( *static_cast<const Implementation::Node *>( m_node ) );
	++it;
	m_node = it != container->end() ? &*it : nullptr;
}

template<typename ShaderPtrType>
std::pair<IECore::InternedString, ShaderPtrType> ShaderNetwork::ShaderIteratorT<ShaderPtrType>::dereference() const
{
	const Implementation::Node *node = static_cast<const Implementation::Node *>( m_node );
	return std::pair<IECore::InternedString, ShaderPtrType>( node->handle, node->shader );
}

template class ShaderNetwork::ShaderIteratorT<ShaderPtr>;
template class ShaderNetwork::ShaderIteratorT<ConstShaderPtr>;

//////////////////////////////////////////////////////////////////////////
// ShaderNetwork
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( ShaderNetwork );

ShaderNetwork::Parameter::Parameter( const IECore::InternedString &shader, const IECore::InternedString &name )
	:	shader( shader ), name( name )
{
}

ShaderNetwork::Connection::Connection( const Parameter &source, const Parameter &destination )
	:	source( source ), destination( destination )
{
}

ShaderNetwork::ShaderNetwork()
	:	m_implementation( new Implementation )
{
}

ShaderNetwork::~ShaderNetwork()
{
}

IECore::InternedString ShaderNetwork::addShader( const IECore::InternedString &handle, const IECoreScene::ShaderPtr &shader )
{
	return implementation()->addShader( handle, shader );
}

void ShaderNetwork::setShader( const IECore::InternedString &handle, const IECoreScene::ShaderPtr &shader )
{
	implementation()->setShader( handle, shader );
}

IECoreScene::Shader *ShaderNetwork::getShader( const IECore::InternedString &handle )
{
	return implementation()->getShader( handle );
}

const IECoreScene::Shader *ShaderNetwork::getShader( const IECore::InternedString &handle ) const
{
	return implementation()->getShader( handle );
}

void ShaderNetwork::removeShader( const IECore::InternedString &handle )
{
	implementation()->removeShader( handle );
}

ShaderNetwork::ShaderIterator ShaderNetwork::removeShader( const ShaderIterator &iterator )
{
	return implementation()->removeShader( iterator );
}

size_t ShaderNetwork::size() const
{
	return implementation()->size();
}

ShaderNetwork::ShaderRange ShaderNetwork::shaders()
{
	return implementation()->shaders();
}

ShaderNetwork::ConstShaderRange ShaderNetwork::shaders() const
{
	return implementation()->shaders();
}

void ShaderNetwork::setOutput( const Parameter &output )
{
	implementation()->setOutput( output );
}

const ShaderNetwork::Parameter &ShaderNetwork::getOutput() const
{
	return implementation()->getOutput();
}

IECoreScene::Shader *ShaderNetwork::outputShader()
{
	return implementation()->outputShader();
}

const IECoreScene::Shader *ShaderNetwork::outputShader() const
{
	return implementation()->outputShader();
}

void ShaderNetwork::addConnection( const Connection &connection )
{
	implementation()->addConnection( connection );
}

void ShaderNetwork::removeConnection( const Connection &connection )
{
	implementation()->removeConnection( connection );
}

ShaderNetwork::Parameter ShaderNetwork::input( const Parameter &destination ) const
{
	return implementation()->input( destination );
}

ShaderNetwork::ConnectionRange ShaderNetwork::inputConnections( const IECore::InternedString &handle ) const
{
	return implementation()->inputConnections( handle );
}

ShaderNetwork::ConnectionRange ShaderNetwork::outputConnections( const IECore::InternedString &handle ) const
{
	return implementation()->outputConnections( handle );
}

bool ShaderNetwork::isEqualTo( const IECore::Object *other ) const
{
	if( !BlindDataHolder::isEqualTo( other ) )
	{
		return false;
	}

	return implementation()->isEqualTo( static_cast<const ShaderNetwork *>( other )->implementation() );
}

void ShaderNetwork::hash( IECore::MurmurHash &h ) const
{
	BlindDataHolder::hash( h );
	implementation()->hash( h );
}

void ShaderNetwork::copyFrom( const IECore::Object *other, IECore::Object::CopyContext *context )
{
	BlindDataHolder::copyFrom( other, context );
	implementation()->copyFrom( static_cast<const ShaderNetwork *>( other )->implementation(), context );
}

void ShaderNetwork::save( IECore::Object::SaveContext *context ) const
{
	BlindDataHolder::save( context );
	implementation()->save( context );
}

void ShaderNetwork::load( IECore::Object::LoadContextPtr context )
{
	BlindDataHolder::load( context );
	implementation()->load( context );
}

void ShaderNetwork::memoryUsage( IECore::Object::MemoryAccumulator &accumulator ) const
{
	BlindDataHolder::memoryUsage( accumulator );
	implementation()->memoryUsage( accumulator );
}

ShaderNetwork::Implementation *ShaderNetwork::implementation()
{
	return m_implementation.get();
}

const ShaderNetwork::Implementation *ShaderNetwork::implementation() const
{
	return m_implementation.get();
}