//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreScene/CurvesAlgo.h"

#include "IECore/DataAlgo.h"
#include "IECore/Interpolator.h"
#include "IECore/TypeTraits.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

namespace
{

template<typename DataType, bool Interpolate = true>
typename DataType::Ptr addPhantomPoints( const DataType *sourceData, const vector<int> &verticesPerCurve )
{
	const auto &source = sourceData->readable();
	typename DataType::Ptr resultData = new DataType;
	auto &result = resultData->writable();
	result.reserve( source.size() + verticesPerCurve.size() * 2 );

	using ElementType = typename DataType::ValueType::value_type;
	LinearInterpolator<ElementType> interpolator;
	ElementType phantomPoint;
	auto sourceIt = source.begin();

	for( int numVertices : verticesPerCurve )
	{
		if constexpr( Interpolate && TypeTraits::IsInterpolable<ElementType>::value )
		{
			interpolator( *(sourceIt+1), *sourceIt, 2, phantomPoint );
			result.push_back( phantomPoint );
		}
		else
		{
			result.push_back( *sourceIt );
		}

		std::copy( sourceIt, sourceIt + numVertices, std::back_inserter( result ) );
		sourceIt += numVertices;

		if constexpr( Interpolate && TypeTraits::IsInterpolable<ElementType>::value )
		{
			interpolator( *(sourceIt-2), *(sourceIt-1), 2, phantomPoint );
			result.push_back( phantomPoint );
		}
		else
		{
			result.push_back( *(sourceIt-1) );
		}
	}

	if constexpr( TypeTraits::IsGeometricTypedData<DataType>::value )
	{
		resultData->setInterpretation( sourceData->getInterpretation() );
	}

	return resultData;
}

} // namespace

bool CurvesAlgo::isPinned( const CurvesPrimitive *curves )
{
	if( curves->wrap() != CurvesPrimitive::Wrap::Pinned )
	{
		return false;
	}
	auto basis = curves->basis().standardBasis();
	return basis == StandardCubicBasis::BSpline || basis == StandardCubicBasis::CatmullRom;
}

void CurvesAlgo::convertPinnedToNonPeriodic( CurvesPrimitive *curves, const IECore::Canceller *canceller )
{
	if( curves->wrap() != CurvesPrimitive::Wrap::Pinned )
	{
		return;
	}

	const StandardCubicBasis standardBasis = curves->basis().standardBasis();
	if( standardBasis != StandardCubicBasis::BSpline && standardBasis != StandardCubicBasis::CatmullRom )
	{
		IECore::Canceller::check( canceller );
		curves->setTopology( curves->verticesPerCurve(), curves->basis(), CurvesPrimitive::Wrap::NonPeriodic );
		return;
	}

	for( auto &[name, primitiveVariable] : curves->variables )
	{
		IECore::Canceller::check( canceller );
		if( primitiveVariable.interpolation == PrimitiveVariable::Vertex )
		{
			if( primitiveVariable.indices )
			{
				// It seems highly unlikely that we'll encounter an indexed
				// curve. The only hypothetical use case I can recall
				// considering was one where `P` was indexed to indicate sharing
				// of vertices between connected curves.
				//
				// We have two choices :
				//
				// 1. Add additional _values_ with the true extrapolated
				//    position of the phantom points, and add indices to
				//    reference those. This gives the exact curve shape we'd get
				//    for a non-indexed curve, but at additional cost.
				// 2. Add additional _indices_ that reference the existing
				//    vertices. This gives a subtly different curve shape, but
				//    is cheaper and preserves connectivity of shared vertices.
				//
				// For simplicity, and because we think this is hypothetical, we
				// choose option 2 for now.
				primitiveVariable.indices = addPhantomPoints<IntVectorData, false>(
					primitiveVariable.indices.get(), curves->verticesPerCurve()->readable()
				);
			}
			else
			{
				dispatch(
					primitiveVariable.data.get(),
					/// \todo Capture can just be `[&]` when we get C++20.
					[&, &primitiveVariable=primitiveVariable] ( auto data ) {
						using DataType = typename std::remove_const_t<std::remove_pointer_t<decltype( data )>>;
						if constexpr( TypeTraits::IsVectorTypedData<DataType>::value )
						{
							primitiveVariable.data = addPhantomPoints( data, curves->verticesPerCurve()->readable() );
						}

					}
				);
			}
		}
	}

	IECore::Canceller::check( canceller );
	IntVectorDataPtr newVerticesPerCurveData = new IntVectorData;
	vector<int> &newVerticesPerCurve = newVerticesPerCurveData->writable();
	newVerticesPerCurve.reserve( curves->numCurves() );
	for( auto numVertices : curves->verticesPerCurve()->readable() )
	{
		newVerticesPerCurve.push_back( numVertices + 2 );
	}

	IECore::Canceller::check( canceller );
	curves->setTopology( newVerticesPerCurveData.get(), curves->basis(), CurvesPrimitive::Wrap::NonPeriodic );
}
