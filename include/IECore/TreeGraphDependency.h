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

#ifndef IE_CORE_TREEGRAPHDEPENDENCY_H
#define IE_CORE_TREEGRAPHDEPENDENCY_H

#include <IECore/GraphDependency.h>

#include <list>
#include <functional>

namespace IECore
{

///Abstract template class for lazy computation of dependencies on a tree graph.
///The templated type specifies the key used to identify graph nodes.
///It assumes the compute function will propagate the dirty flag to the parent nodes if that is the case.
///It also assumes the comparison operator "<" always return true when comparing parents to their children. And if parents A < B is true then child(A) < child(B) is also true.
///This class holds a list of ordered dirty nodes reflecting the tree dependencies. So care should be taken if the nodes change their connections possibly affecting
///the key order.
template< typename T >
class TreeGraphDependency : public GraphDependency<T>
{
	public:

		///Triggers recursive computation on all dirty nodes.
		virtual void update( );

		///Triggers recursive computation on all dirty nodes dependent and including the given one.
		virtual void update( const T &node );

		///Set the dirty flag for the given node.
		virtual void setDirty( const T &node );

		///Verifies if a node is set dirty.
		virtual bool getDirty( const T &node );

		///Clear all dirty node flags.
		virtual void clear();

		///Clear dirty flags for the given node and all nodes it depends on.
		///This function is particularly useful when deleting subtrees.
		virtual void clear( const T &node );

		///Returns the root node name
		virtual T rootNode() const = 0;

		///Returns true if node1 descends directly from node2.
		virtual bool isDescendant( const T &node1, const T &node2 ) const = 0;

	protected:

		// binary operator used for sorting dirty list in descending order.
		struct TreeOrdering : public std::binary_function< T, T, bool >
		{
				bool operator()( const T &lhs, const T &rhs ) const
				{
					return rhs < lhs;
				}

		} m_treeOrdering;

		typedef std::list< T > DirtyList;
		///Internal list of dirty nodes.
		DirtyList m_dirtyNodes;

		///Set the dirty flag for the given node.
		///Starts looking for node from the given dirty list position.
		void setDirty( const T &node, typename DirtyList::iterator begin );

		///Triggers recursive computation on all dirty nodes dependent and including the given one. 
		///Starts looking for the node on the given position.
		void update( const T &node, typename DirtyList::iterator pos );

};

#include <IECore/TreeGraphDependency.inl>

} // namespace IECore

#endif //IE_CORE_TREEGRAPHDEPENDENCY_H
