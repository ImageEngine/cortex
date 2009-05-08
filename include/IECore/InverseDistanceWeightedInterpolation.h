//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_INVERSEDISTANCEWEIGHTEDINTERPOLATION_H
#define IE_CORE_INVERSEDISTANCEWEIGHTEDINTERPOLATION_H

#include <map>

#include "IECore/KDTree.h"

namespace IECore
{

/// The InverseDistanceWeightedInterpolation class provides interpolation of scattered data. It is
/// templated so that it can operate on a wide variety of point/value types, and uses
/// the VectorTraits.h and VectorOps.h functionality to assist in this.
/// NB. The Value must be default constructible, and define sensible value=value+value, and value=value*scalar operators
template< typename PointIterator, typename ValueIterator >
class InverseDistanceWeightedInterpolation
{
	public :

		typedef typename std::iterator_traits<PointIterator>::value_type Point;
		typedef typename VectorTraits<Point>::BaseType PointBaseType;

		typedef typename std::iterator_traits<ValueIterator>::value_type Value;

		/// Creates the interpolator. Note that it does not own the passed points or values -
		/// it is up to you to ensure that they remain valid and unchanged as long as the
		/// interpolator is in use.
		/// \param firstPoint Iterator to first point
		/// \param lastPoint Iterator to last point
		/// \param firstValue Iterator to first value
		/// \param lastValue Iterator to last value
		/// \param numNeighbours The amount of nearest-neighbour points to consider when performing interpolation. More usually yields slower, but better results.
		/// \param maxLeafSize The number of points to store in each KDTree bucket
		InverseDistanceWeightedInterpolation(
			PointIterator firstPoint,
			PointIterator lastPoint,
			ValueIterator firstValue,
			ValueIterator lastValue,
			unsigned int numNeighbours,
			int maxLeafSize=4
		);

		virtual ~InverseDistanceWeightedInterpolation();

		/// Evaluate the interpolated value for the specified point
		Value operator()( const Point &p ) const;


	private :

		typedef KDTree<PointIterator> Tree;
		Tree *m_tree;

		typedef std::map< PointIterator, ValueIterator > ValueMap;
		ValueMap m_map;

		unsigned int m_numNeighbours;
};

typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V2f>::const_iterator, std::vector<float>::const_iterator  > InverseDistanceWeightedInterpolationV2ff;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V2d>::const_iterator, std::vector<double>::const_iterator > InverseDistanceWeightedInterpolationV2dd;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V3f>::const_iterator, std::vector<float>::const_iterator  >  InverseDistanceWeightedInterpolationV3ff;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V3d>::const_iterator, std::vector<double>::const_iterator > InverseDistanceWeightedInterpolationV3dd;

typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V2f>::const_iterator, std::vector<Imath::V2f>::const_iterator > InverseDistanceWeightedInterpolationV2fV2f;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V2d>::const_iterator, std::vector<Imath::V2d>::const_iterator > InverseDistanceWeightedInterpolationV2dV2d;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V3f>::const_iterator, std::vector<Imath::V3f>::const_iterator > InverseDistanceWeightedInterpolationV3fV3f;
typedef InverseDistanceWeightedInterpolation< std::vector<Imath::V3d>::const_iterator, std::vector<Imath::V3d>::const_iterator > InverseDistanceWeightedInterpolationV3dV3d;


} // namespace IECore

#include "InverseDistanceWeightedInterpolation.inl"

#endif // IE_CORE_INVERSEDISTANCEWEIGHTEDINTERPOLATION_H
