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

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "boost/lexical_cast.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/regex.hpp"
#include "boost/algorithm/string/replace.hpp"

#include <mutex>
#include <unordered_set>

using namespace std;
using namespace boost;
using namespace IECore;
using namespace IECoreScene;

namespace {

struct ReplaceFunctor
{
	ReplaceFunctor( const IECore::CompoundObject *attributes ) : m_attributes( attributes )
	{
	}

	std::string operator()( const boost::smatch & match )
	{
		// Search for attribute matching token
		const StringData *sourceAttribute = m_attributes->member<StringData>( match[1].str() );
		if( sourceAttribute )
		{
			return sourceAttribute->readable();
		}
		else
		{
			// Otherwise, return empty string
			return "";
		}
	}

	const IECore::CompoundObject *m_attributes;
};

boost::regex attributeRegex()
{
	// Extract ATTR_NAME from the pattern <attr:ATTR_NAME>
	// Only match if the angle brackets haven't been escaped with a backslash
	static boost::regex r( "(?<!\\\\)<attr:([^>]*[^\\\\>])>" );
	return r;	
}

bool stringFindSubstitutions( const std::string &target, std::unordered_set< InternedString > &requestedAttributes )
{
	bool found = false;
	boost::sregex_iterator matches( target.begin(), target.end(), attributeRegex(), boost::match_default );
	for( boost::sregex_iterator i = matches; i != boost::sregex_iterator(); i++ )
	{
		found = true;

		// The one group in the expression gives us the token we're looking for
		requestedAttributes.insert( InternedString( (*i)[1] ) );
	}

	// Strings with escaped substitutions don't require attributes, but we do need call
	// stringApplySubstitutions on them so that the escape symbols get removed
	if( target.find( "\\<" ) != string::npos )
	{
		found = true;
	}
	if( target.find( "\\>" ) != string::npos )
	{
		found = true;
	}
	return found;
}

std::string stringApplySubstitutions( const std::string &target, const IECore::CompoundObject *attributes )
{
	ReplaceFunctor rf( attributes );
	std::string result = boost::regex_replace(
		target, attributeRegex(), rf, boost::match_default | boost::format_all
	);
	boost::replace_all( result, "\\<", "<" );
	boost::replace_all( result, "\\>", ">" );
	return result;
}


} // namespace

//////////////////////////////////////////////////////////////////////////
// ShaderNetwork::Implementation
//////////////////////////////////////////////////////////////////////////

class ShaderNetwork::Implementation
{

	public :

		IECore::InternedString addShader( IECore::InternedString handle, const IECoreScene::Shader *shader )
		{
			return addShader( handle, shader->copy() );
		}

		IECore::InternedString addShader( IECore::InternedString handle, ShaderPtr &&shader )
		{
			handle = uniqueHandle( handle );
			setShader( handle, std::move( shader ) );
			return handle;
		}

		void setShader( const IECore::InternedString &handle, const IECoreScene::Shader *shader )
		{
			setShader( handle, shader->copy() );
		}

		void setShader( const IECore::InternedString &handle, ShaderPtr &&shader )
		{
			if( shader->refCount() != 1 )
			{
				throw Exception( "Shader reference count must be 1" );
			}
			m_nodes.insert( handle ).first->mutableShader() = ShaderPtr( std::move( shader ) );
			m_dirty = true;
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
				throw IECore::Exception( boost::str(
					boost::format(
						"Shader \"%1%\" not in network"
					) % handle.c_str()
				) );
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

		ShaderNetwork::ShaderRange shaders() const
		{
			return ShaderRange(
				ShaderIterator( &m_nodes, m_nodes.size() ? &*m_nodes.begin() : nullptr ),
				ShaderIterator( &m_nodes, nullptr )
			);
		}

		void addConnection( const Connection &connection )
		{
			auto sourceIt = m_nodes.find( connection.source.shader );
			if( sourceIt == m_nodes.end() )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Source shader \"%1%\" not in network"
					) % connection.source.shader.c_str()
				) );
			}

			auto destinationIt = m_nodes.find( connection.destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Destination shader \"%1%\" not in network"
					) % connection.destination.shader.c_str()
				) );
			}

			Connection c = connection;
			bool inserted = destinationIt->mutableInputConnections().insert( c ).second;
			if( !inserted )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Destination parameter \"%1%.%2%\" already has a connection"
					) % connection.destination.shader.c_str() % connection.destination.name.c_str()
				) );
			}
			inserted = sourceIt->mutableOutputConnections().insert( c ).second;
			assert( inserted );

			m_dirty = true;
		}

		void removeConnection( const Connection &connection )
		{
			auto destinationIt = m_nodes.find( connection.destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Destination shader \"%1%\" not in network"
					) % connection.destination.shader.c_str()
				) );
			}

			auto connectionIt = destinationIt->inputConnections.find( connection.destination );
			if( connectionIt == destinationIt->inputConnections.end() || *connectionIt != connection )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Connection \"%1%.%2% -> %3%.%4%\" not in network"
					) % connection.source.shader.c_str() % connection.source.name.c_str() % connection.destination.shader.c_str() % connection.destination.name.c_str()
				) );
			}

			destinationIt->mutableInputConnections().erase( connectionIt );

			auto sourceIt = m_nodes.find( connection.source.shader );
			assert( sourceIt != m_nodes.end() );
			sourceIt->mutableOutputConnections().erase( connection.destination );

			m_dirty = true;
		}

		Parameter input( const Parameter &destination ) const
		{
			auto destinationIt = m_nodes.find( destination.shader );
			if( destinationIt == m_nodes.end() )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"Destination shader \"%1%\" not in network"
					) % destination.shader.c_str()
				) );
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
				throw IECore::Exception( boost::str(
					boost::format(
						"Destination shader \"%1%\" not in network"
					) % handle.c_str()
				) );
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
				throw IECore::Exception( boost::str(
					boost::format(
						"Source shader \"%1%\" not in network"
					) % handle.c_str()
				) );
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
				throw IECore::Exception( boost::str(
					boost::format(
						"Output shader \"%1%\" not in network"
					) % output.shader.c_str()
				) );
			}

			m_output = output;
			m_dirty = true;
		}

		const Parameter &getOutput() const
		{
			return m_output;
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

				if( node.inputConnections.size() != otherNode.inputConnections.size() )
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
			update();
			h.append( m_hash );
		}

		void hashSubstitutions( const CompoundObject *attributes, MurmurHash &h ) const
		{
			update();
			for( const auto &a : m_neededSubstitutions )
			{
				const StringData *sourceAttribute = attributes->member<StringData>( a );
				if( sourceAttribute )
				{
					h.append( sourceAttribute->readable() );
				}
				else
				{
					// Need to append placeholder since which attributes are found matters
					h.append( 0 );
				}
			}
		}

		void applySubstitutions( const CompoundObject *attributes )
		{
			update();

			for( const auto &nodeAndParms : m_parmsNeedingSubstitution )
			{
				auto it = m_nodes.find( nodeAndParms.first );
				ShaderPtr s ( it->shader->copy() );
				for( const auto &parm : nodeAndParms.second )
				{
					StringData *targetParm = runTimeCast< StringData >( s->parameters()[parm].get() );
					if( targetParm )
					{
						targetParm->writable() = stringApplySubstitutions( targetParm->readable(), attributes );
						continue;
					}

					StringVectorData *targetParmVector = runTimeCast< StringVectorData >( s->parameters()[parm].get() );
					if( targetParmVector )
					{
						std::vector<std::string> &stringVector = targetParmVector->writable();
						for( unsigned int i = 0; i < stringVector.size(); i++ )
						{
							stringVector[i] = stringApplySubstitutions( stringVector[i], attributes );
						}
						continue;
					}

					throw Exception( "ShaderNetwork::applySubstitutions : parameter need substitution couldn't be found - was the network somehow modified without being dirtied?" );
				}
				it->mutableShader() = s;
			}
			m_dirty = true;
		}

		void copyFrom( const Implementation *other, IECore::Object::CopyContext *context )
		{
			// Shallow copy is sufficient because ShaderNetworks take complete
			// ownership of their shaders, and the shaders remain immutable at
			// all times.
			m_nodes = other->m_nodes;
			m_output = other->m_output;
			m_hash = other->m_hash;
			m_dirty = other->m_dirty;
			m_parmsNeedingSubstitution = other->m_parmsNeedingSubstitution;
			m_neededSubstitutions = other->m_neededSubstitutions;
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
				setShader( handle, shader.get() );
			}

			ConstIndexedIOPtr connections = container->subdirectory( "connections" );
			IndexedIO::EntryIDList connectionIndices;
			connections->entryIds( connectionIndices );
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
			Node( IECore::InternedString handle )
				:	handle( handle )
			{
			}

			IECore::InternedString handle;
			IECoreScene::ConstShaderPtr shader;

			Connections inputConnections;
			Connections outputConnections;

			const std::string &handleKey() const { return handle.string(); }
			const IECoreScene::Shader *shaderKey() const { return shader.get(); }

			// `multi_index_container` only provides const access to elements
			// to avoid keys changing behind its back. But we know that
			// `shader` isn't used as a key, so this cast is kosher. Note that
			// we're only providing mutable access to the pointer, not to the
			// shader. The shader remains immutable at all times.
			ConstShaderPtr &mutableShader() const
			{
				return const_cast<ConstShaderPtr &>( shader );
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

		mutable std::map< InternedString, std::vector< InternedString > > m_parmsNeedingSubstitution;
		mutable std::unordered_set< InternedString > m_neededSubstitutions;

		// Tracks whether or not the hash and substitutions are up to date.
		mutable bool m_dirty = true;

		// Used to avoid multiple threads trying to update the
		// hash and substitutions at the same time.
		mutable std::mutex m_updateMutex;

		InternedString uniqueHandle( const InternedString &handle )
		{
			if( m_nodes.find( handle ) == m_nodes.end() )
			{
				return handle;
			}

			string result;
			for( int i = 1; true; ++i )
			{
				result = handle.string() + std::to_string( i );
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

			m_dirty = true;
			return m_nodes.erase( it );
		}

		void update() const
		{
			std::unique_lock<std::mutex> lock( m_updateMutex );
			if( m_dirty )
			{
				m_hash = MurmurHash();
				m_neededSubstitutions.clear();
				m_parmsNeedingSubstitution.clear();
				for( const auto &node : m_nodes )
				{
					m_hash.append( node.handle );
					node.shader->hash( m_hash );
					std::vector< InternedString > parmsNeedingSub;

					for( const auto &connection : node.inputConnections )
					{
						m_hash.append( connection.source.shader );
						m_hash.append( connection.source.name );
						m_hash.append( connection.destination.name );
					}

					for( const auto &parm : node.shader->parameters() )
					{
						bool needed = false;
						StringData *stringParm = IECore::runTimeCast< StringData >( parm.second.get() );
						StringVectorData *stringVectorParm = IECore::runTimeCast< StringVectorData >( parm.second.get() );
						if( stringParm )
						{
							needed |= stringFindSubstitutions( stringParm->readable(), m_neededSubstitutions );
						}
						if( stringVectorParm )
						{
							for( const std::string &i : stringVectorParm->readable() )
							{
								needed |= stringFindSubstitutions( i, m_neededSubstitutions );
							}
						}
						if( needed )
						{
							parmsNeedingSub.push_back( parm.first );
						}
					}

					m_parmsNeedingSubstitution[ node.handle ] = parmsNeedingSub;	
				}

				m_hash.append( m_output.shader );
				m_hash.append( m_output.name );

				m_dirty = false;
			}
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

void ShaderNetwork::ShaderIterator::increment()
{
	auto container = static_cast<const Implementation::NodeContainer *>( m_container );
	auto it = container->iterator_to( *static_cast<const Implementation::Node *>( m_node ) );
	++it;
	m_node = it != container->end() ? &*it : nullptr;
}

std::pair<IECore::InternedString, ConstShaderPtr> ShaderNetwork::ShaderIterator::dereference() const
{
	const Implementation::Node *node = static_cast<const Implementation::Node *>( m_node );
	return std::pair<IECore::InternedString, ConstShaderPtr>( node->handle, node->shader );
}

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

IECore::InternedString ShaderNetwork::addShader( const IECore::InternedString &handle, const IECoreScene::Shader *shader )
{
	return implementation()->addShader( handle, shader );
}

IECore::InternedString ShaderNetwork::addShader( const IECore::InternedString &handle, ShaderPtr &&shader )
{
	return implementation()->addShader( handle, std::move( shader ) );
}

void ShaderNetwork::setShader( const IECore::InternedString &handle, const IECoreScene::Shader *shader )
{
	implementation()->setShader( handle, shader );
}

void ShaderNetwork::setShader( const IECore::InternedString &handle, ShaderPtr &&shader )
{
	implementation()->setShader( handle, std::move( shader ) );
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

ShaderNetwork::ShaderRange ShaderNetwork::shaders() const
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

const IECoreScene::Shader *ShaderNetwork::outputShader() const
{
	return implementation()->outputShader();
}

void ShaderNetwork::hashSubstitutions( const CompoundObject *attributes, MurmurHash &h ) const
{
	implementation()->hashSubstitutions( attributes, h );
}

void ShaderNetwork::applySubstitutions( const CompoundObject *attributes )
{
	implementation()->applySubstitutions( attributes );
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
