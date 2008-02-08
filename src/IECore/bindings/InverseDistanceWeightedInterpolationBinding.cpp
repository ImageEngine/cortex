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
#include "IECore/InverseDistanceWeightedInterpolation.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECore/bindings/InverseDistanceWeightedInterpolationBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECore
{

template<typename T>
void bindInverseDistanceWeightedInterpolation(const char *bindName);
	
void bindInverseDistanceWeightedInterpolation()
{
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV2ff >("InverseDistanceWeightedInterpolationV2ff");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV2dd >("InverseDistanceWeightedInterpolationV2dd");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV3ff >("InverseDistanceWeightedInterpolationV3ff");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV3dd >("InverseDistanceWeightedInterpolationV3dd");
	
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV2fV2f >("InverseDistanceWeightedInterpolationV2fV2f");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV2dV2d >("InverseDistanceWeightedInterpolationV2dV2d");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV3fV3f >("InverseDistanceWeightedInterpolationV3fV3f");
	bindInverseDistanceWeightedInterpolation< InverseDistanceWeightedInterpolationV3dV3d >("InverseDistanceWeightedInterpolationV3dV3d");	
}

template<typename T>
struct InverseDistanceWeightedInterpolationWrapper
{
	typedef TypedData<std::vector<typename T::Point> > PointData;
	IE_CORE_DECLAREPTR( PointData )
	
	typedef TypedData<std::vector<typename T::Value> > ValueData;
	IE_CORE_DECLAREPTR( ValueData )	
	
	T* m_idw;
	
	PointDataPtr m_points;
	ValueDataPtr m_values;	
		
	InverseDistanceWeightedInterpolationWrapper(PointDataPtr points, ValueDataPtr values, unsigned int numNeighbours)
	{
		m_points = points->copy();
		m_values = values->copy();		
		m_idw = new T(
			m_points->readable().begin(), 
			m_points->readable().end(),
			m_values->readable().begin(), 
			m_values->readable().end(),
			numNeighbours
		);
		assert(m_idw);
	}
	
	virtual ~InverseDistanceWeightedInterpolationWrapper()
	{
		assert(m_idw);
		delete m_idw;
	}
	
	inline typename T::Value call( const typename T::Point &p ) const
	{
		assert(m_idw);
		return (*m_idw)( p );
	}
	
	
};

template<typename T>
void bindInverseDistanceWeightedInterpolation(const char *bindName)
{
	class_<InverseDistanceWeightedInterpolationWrapper<T>, boost::noncopyable>(bindName, no_init)
		.def(init< typename InverseDistanceWeightedInterpolationWrapper<T>::PointDataPtr, typename InverseDistanceWeightedInterpolationWrapper<T>::ValueDataPtr, unsigned int >() )
		.def( "__call__", &InverseDistanceWeightedInterpolationWrapper<T>::call )
		;
}

}
