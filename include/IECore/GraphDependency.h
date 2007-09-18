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

#ifndef IE_CORE_GRAPHDEPENDENCY_H
#define IE_CORE_GRAPHDEPENDENCY_H

#include <IECore/RefCounted.h>

namespace IECore
{

///Abstract template class for lazy computation of dependencies on any kind of graph.
///This class is not supposed to hold the graph itself. It actually should be used by graph objects that
///contain implicit or explicit dependency connections. The compute virtual method is dependent on the graph it's being used.
///The templated type T specifies the key used to identify graph nodes.
template< typename T>
class GraphDependency : public RefCounted
{
	public:

		///Triggers recursive computation on all dirty nodes.
		virtual void update( ) = 0;
				
		///Triggers recursive computation on all dirty nodes dependent on the given node including itself.
		virtual void update( const T &node ) = 0;

		///Set the dirty flag for the given node.
		virtual void setDirty( const T &node ) = 0;

		///Verifies if a node is set dirty.
		virtual bool getDirty( const T &node ) = 0;

		///Clear all dirty node flags.
		virtual void clear() = 0;

	protected:

		///Updates a node. It's guarantee that all dependent nodes are updated.
		virtual void compute( const T &node ) = 0;
};


} // namespace IECore

#endif //IE_CORE_GRAPHDEPENDENCY_H
