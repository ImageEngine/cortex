//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include <cassert>
#include <string>

#include "IECore/RefCounted.h"
#include "IECore/BoundedKDTree.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECore/bindings/BoundedKDTreeBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECore
{

template<typename T>
void bindBoundedKDTree(const char *bindName);

void bindBoundedKDTree()
{
	bindBoundedKDTree< Box2fTree >("Box2fTree");
	bindBoundedKDTree< Box2dTree >("Box2dTree");
	bindBoundedKDTree< Box3fTree >("Box3fTree");
	bindBoundedKDTree< Box3dTree >("Box3dTree");
}

template<typename T>
struct BoundedKDTreeWrapper
{
	typedef TypedData<std::vector<typename T::Bound> > BoundData;
	IE_CORE_DECLAREPTR( BoundData )

	T* m_tree;

	BoundDataPtr m_bounds;

	BoundedKDTreeWrapper(BoundDataPtr bounds)
	{
		m_bounds = bounds->copy();
		m_tree = new T(m_bounds->readable().begin(), m_bounds->readable().end());
	}

	virtual ~BoundedKDTreeWrapper()
	{
		assert(m_tree);
		delete m_tree;
	}

	template<typename S>
	IntVectorDataPtr intersectingBounds(const S &b)
	{
		assert(m_tree);

		typedef std::vector<typename T::Iterator> BoundArray;

		BoundArray bounds;

		unsigned int num = m_tree->intersectingBounds(b, bounds);

		IntVectorDataPtr indices = new IntVectorData();

		indices->writable().reserve( num );

		for (typename BoundArray::const_iterator it = bounds.begin(); it != bounds.end(); ++it)
		{
			indices->writable().push_back(  std::distance( m_bounds->readable().begin(), *it ) );
		}

		return indices;

	}

};


template<typename T>
void bindBoundedKDTree(const char *bindName)
{
	class_<BoundedKDTreeWrapper<T>, boost::noncopyable>(bindName, no_init)
		.def(init< typename BoundedKDTreeWrapper<T>::BoundDataPtr >() )
		.def("intersectingBounds", &BoundedKDTreeWrapper<T>::template intersectingBounds<typename T::Bound> )
		.def("intersectingBounds", &BoundedKDTreeWrapper<T>::template intersectingBounds<typename T::BaseType> )
	;

}

}
