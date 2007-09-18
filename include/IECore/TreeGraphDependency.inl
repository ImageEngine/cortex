//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

template< typename T >
void TreeGraphDependency<T>::update( )
{
	update( rootNode(), m_dirtyNodes.end() );
}

template< typename T >
void TreeGraphDependency<T>::update( const T &node )
{
	typename DirtyList::iterator i = lower_bound(  m_dirtyNodes.begin(), m_dirtyNodes.end(), node, std::greater< T >() );
	update( node, i );
}

template< typename T >
void TreeGraphDependency<T>::update( const T &node, typename DirtyList::iterator pos )
{

	if ( !m_dirtyNodes.size() )
	{
		return;
	}
	
	// update child nodes first.
	while( pos != m_dirtyNodes.begin() )
	{
		// check if the previous item in the ordered list is a child of the given node.
		typename DirtyList::iterator childIt( --typename DirtyList::iterator( pos ) );
		if ( *childIt == node )
		{
			// if they are the same then the requested node was inserted while updating child nodes...
			pos = childIt;
		}
		else if ( isDescendant( *childIt, node ) )
		{
			// descendant items should be updated first.
			update( *childIt, childIt );
		}
		else
		{
			// no, so let's stop this loop.
			break;
		}
	}
	
	if ( pos == m_dirtyNodes.end() || *pos != node )
	{
		// If this node wasn't actually dirty, return right now.
		return;
	}

	compute( node );

	// clean up dirty flag on this node.
	m_dirtyNodes.erase( pos );
}

template< typename T >
void TreeGraphDependency<T>::setDirty( const T &node )
{
	setDirty( node, m_dirtyNodes.begin() );
}

template< typename T >
void TreeGraphDependency<T>::setDirty( const T &node, typename DirtyList::iterator begin )
{
	typename DirtyList::iterator i = lower_bound(  begin, m_dirtyNodes.end(), node, std::greater< T >() );

	if ( i != m_dirtyNodes.end() )
	{
		if ( *i == node )
		{
			return;
		}
	}
	m_dirtyNodes.insert( i, node );

}

template< typename T >
bool TreeGraphDependency<T>::getDirty( const T &node )
{
	if ( !m_dirtyNodes.size() )
	{
		return false;
	}
	typename DirtyList::iterator it = lower_bound(  m_dirtyNodes.begin(), m_dirtyNodes.end(), node, std::greater< T >() );
	if ( it != m_dirtyNodes.end() && *it == node )
	{
		return true;
	}

	if ( it == m_dirtyNodes.begin() )
	{
		// could not find the node and there's no one before it. So it's not dirty.
		return false;
	}

	// check if the previous item in the ordered list is a child of the given node.
	--it;
	if ( isDescendant( *it, node ) )
	{
		return true;
	}

	return false;
}

template< typename T >
void TreeGraphDependency<T>::clear( const T &node )
{
	if ( !m_dirtyNodes.size() )
	{
		return;
	}

	typename DirtyList::iterator pos = lower_bound(  m_dirtyNodes.begin(), m_dirtyNodes.end(), node, std::greater< T >() );
	while( pos != m_dirtyNodes.begin() )
	{
		// check if the given node is connected to the previous item in the ordered list.
		typename DirtyList::iterator childIt( --typename DirtyList::iterator( pos ) );
		if ( isDescendant( *childIt, node ) )
		{
			// descendant items should be cleaned first.
			m_dirtyNodes.erase( childIt );
		}
		else
		{
			// no, so let's stop this loop.
			break;
		}
	}

	if ( pos != m_dirtyNodes.end() && *pos == node )
	{
		m_dirtyNodes.erase( pos );
	}
}

template< typename T >
void TreeGraphDependency<T>::clear()
{
	m_dirtyNodes.clear();
}

