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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include <boost/python.hpp>

#include <cassert>
#include <string>

#include "IECore/RefCounted.h"
#include "IECore/KDTree.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECore/bindings/KDTreeBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECore
{

template<typename T>
void bindKDTree(const char *bindName);
	
void bindKDTree()
{
	bindKDTree< V2fTree >("V2fTree");
	bindKDTree< V2dTree >("V2dTree");
	bindKDTree< V3fTree >("V3fTree");
	bindKDTree< V3dTree >("V3dTree");
}

template<typename T>
struct KDTreeWrapper
{
	typedef TypedData<std::vector<typename T::Point> > PointData;
	IE_CORE_DECLAREPTR( PointData )
	
	T* m_tree;
	
	PointDataPtr m_points;
		
	KDTreeWrapper(PointDataPtr points)
	{
		m_points = points->copy();
		m_tree = new T(m_points->readable().begin(), m_points->readable().end());
	}
	
	virtual ~KDTreeWrapper()
	{
		assert(m_tree);
		delete m_tree;
	}
	
	long nearestNeighbour( const typename T::Point &p)
	{
		assert(m_tree);
		
		typename T::Iterator it = m_tree->nearestNeighbour(p);
		
		return std::distance( m_points->readable().begin(), it );
	}
	
	IntVectorDataPtr nearestNeighbours(const typename T::Point &p, typename T::Point::BaseType r)
	{
		assert(m_tree);
		
		typedef std::vector<typename T::Iterator> PointArray;
		
		PointArray points;
		
		unsigned int num = m_tree->nearestNeighbours(p, r, points);
		
		IntVectorDataPtr indices = new IntVectorData();
		
		indices->writable().reserve( num );
		
		for (typename PointArray::const_iterator it = points.begin(); it != points.end(); ++it)
		{
			indices->writable().push_back(  std::distance( m_points->readable().begin(), *it ) );
		}
		
		return indices;
		
	}
	
	IntVectorDataPtr nearestNNeighbours(const typename T::Point &p, unsigned int numNeighbours)
	{
		assert(m_tree);
		
		typedef std::vector<typename T::Iterator> PointArray;
		
		PointArray points;
		
		unsigned int num = m_tree->nearestNNeighbours(p, numNeighbours, points);
		
		IntVectorDataPtr indices = new IntVectorData();
		
		indices->writable().reserve( num );
		
		for (typename PointArray::const_iterator it = points.begin(); it != points.end(); ++it)
		{
			indices->writable().push_back(  std::distance( m_points->readable().begin(), *it ) );
		}
		
		return indices;
		
	}	
	
};


template<typename T>
void bindKDTree(const char *bindName)
{
	class_<KDTreeWrapper<T>, boost::noncopyable>(bindName, no_init)
		.def(init< typename KDTreeWrapper<T>::PointDataPtr >() )
		.def("nearestNeighbour", &KDTreeWrapper<T>::nearestNeighbour )
		.def("nearestNeighbours", &KDTreeWrapper<T>::nearestNeighbours )
		;
}

}
