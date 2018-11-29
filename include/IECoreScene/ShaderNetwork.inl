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

#ifndef IECORESCENE_SHADERNETWORK_INL
#define IECORESCENE_SHADERNETWORK_INL

#include "boost/functional/hash.hpp"
#include "boost/iterator/iterator_facade.hpp"

namespace IECoreScene
{

class ShaderNetwork::ShaderIterator : public boost::iterator_facade<ShaderIterator, std::pair<IECore::InternedString, ConstShaderPtr>, boost::forward_traversal_tag, std::pair<IECore::InternedString, ConstShaderPtr>>
{

	public :

		ShaderIterator()
			:	m_container( nullptr ), m_node( nullptr )
		{
		}

	private :

		ShaderIterator( const void *container, const void *node )
			:	m_container( container ), m_node( node )
		{
		}

		friend class boost::iterator_core_access;
		friend class ShaderNetwork;

		void increment();

		bool equal( ShaderIterator const &other ) const
		{
			return
				m_container == other.m_container &&
				m_node == other.m_node;
		}

		std::pair<IECore::InternedString, ConstShaderPtr> dereference() const;

		// Using `void *` to avoid exposing ShaderNetwork's
		// internal data structures.
		const void *m_container;
		const void *m_node;

};

class ShaderNetwork::ConnectionIterator : public boost::iterator_facade<ConnectionIterator, const Connection, boost::forward_traversal_tag>
{

	public :

		ConnectionIterator()
			:	m_container( nullptr ), m_connection( nullptr )
		{
		}

	private :

		ConnectionIterator( const void *container, const Connection *connection )
			:	m_container( container ), m_connection( connection )
		{
		}

		friend class boost::iterator_core_access;
		friend class ShaderNetwork;

		void increment();

		bool equal( ConnectionIterator const &other ) const
		{
			return
				m_container == other.m_container &&
				m_connection == other.m_connection;
		}

		const ShaderNetwork::Connection &dereference() const
		{
			return *m_connection;
		}

		// Using `void *` to avoid exposing ShaderNetwork's
		// internal data structures.
		const void *m_container;
		const Connection *m_connection;

};

inline ShaderNetwork::Parameter::operator bool () const
{
	return shader.value().size() || name.value().size();
}

inline bool operator == ( const ShaderNetwork::Parameter &lhs, const ShaderNetwork::Parameter &rhs )
{
	return lhs.shader == rhs.shader && lhs.name == rhs.name;
}

inline bool operator != ( const ShaderNetwork::Parameter &lhs, const ShaderNetwork::Parameter &rhs )
{
	return !(lhs == rhs);
}

inline bool operator < ( const ShaderNetwork::Parameter &lhs, const ShaderNetwork::Parameter &rhs )
{
	if( lhs.shader.string() < rhs.shader.string() )
	{
		return true;
	}
	else if( rhs.shader.string() < lhs.shader.string() )
	{
		return false;
	}
	else
	{
		return lhs.name.string() < rhs.name.string();
	}
}

inline std::ostream & operator << ( std::ostream &stream, const ShaderNetwork::Parameter &parameter )
{
	stream << "Parameter( \"" << parameter.shader << "\", \"" << parameter.name << "\" )";
	return stream;
}

// Make Parameter compatible with `boost::hash()`, for use in containers.
inline std::size_t hash_value( const ShaderNetwork::Parameter &parameter )
{
	std::size_t h = 0;
	boost::hash_combine( h, parameter.shader.c_str() );
	boost::hash_combine( h, parameter.name.c_str() );
	return h;
}

inline bool operator == ( const ShaderNetwork::Connection &lhs, const ShaderNetwork::Connection &rhs )
{
	return lhs.source == rhs.source && lhs.destination == rhs.destination;
}

inline bool operator != ( const ShaderNetwork::Connection &lhs, const ShaderNetwork::Connection &rhs )
{
	return !(lhs == rhs);
}

inline std::ostream & operator << ( std::ostream &stream, const ShaderNetwork::Connection &connection )
{
	stream << "Connection( " << connection.source << ", " << connection.destination << " )";
	return stream;
}

} // namespace IECoreScene

namespace std
{

template <>
struct hash<IECoreScene::ShaderNetwork::Parameter>
{
	size_t operator()( const IECoreScene::ShaderNetwork::Parameter &parameter ) const
	{
		return hash_value( parameter );
	}
};

} // namespace std

#endif // IECORESCENE_SHADERNETWORK_INL
