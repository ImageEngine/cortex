//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#include "tbb/concurrent_unordered_map.h"
#include "tbb/task_group.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace
{

// For each curve of the vertex or varying primvars, change the number of replicated values at
// the end by "adjustment", either adding or removing values at both ends, depending on the sign of
// "adjustment"
template<typename T>
void adjustEndPoints( const std::vector<T> &in, std::vector<T> &out, const CurvesPrimitive *curves, PrimitiveVariable::Interpolation interpolation, int adjustment )
{
	size_t numCurves = curves->numCurves();
	out.reserve( in.size() + numCurves * 2 * adjustment );

	size_t curveOffset = 0;
	for( size_t i = 0; i < numCurves; ++i )
	{
		size_t size = curves->variableSize( interpolation, i );

		if( adjustment < 0 )
		{
			// Skip -adjustment vertices at the start and end
			for( size_t j = -adjustment; j < size + adjustment; ++j )
			{
				out.push_back( in[curveOffset + j] );
			}
		}
		else
		{
			// duplicate the start point
			for( size_t j = 0; j < (size_t)adjustment; ++j )
			{
				out.push_back( in[curveOffset] );
			}

			for( size_t j = 0; j < size; ++j )
			{
				out.push_back( in[curveOffset + j] );
			}

			// duplicate the end point 
			for( size_t j = 0; j < (size_t)adjustment; ++j )
			{
				out.push_back( in[curveOffset + size - 1] );
			}
		}

		curveOffset += size;
	}
}

// How many end points need to be duplicated in order to reach the final vert with different basis
int requiredMultiplicity( const IECore::CubicBasisf &cubicBasis )
{
	if( cubicBasis == IECore::CubicBasisf::bSpline() )
	{
		return 3;
	}
	else if( cubicBasis == IECore::CubicBasisf::catmullRom() )
	{
		return 2;
	}
	else if( cubicBasis == IECore::CubicBasisf::linear() )
	{
		return 1;
	}
	else
	{
		throw IECore::Exception( "updateEndPointMultiplicity : Unsupported curve basis" );
	}
}

struct DuplicateEndPoints
{
	DuplicateEndPoints( int vertexAdjustment, int varyingAdjustment ) : m_vertexAdjustment( vertexAdjustment ), m_varyingAdjustment( varyingAdjustment )
	{
	}

	// template template parameter 'S' to capture if the input type is either TypedData or GeometricTypedData
	template<typename T, template<typename> class S >
	IECore::DataPtr operator()(
		const S<std::vector<T>> *data, const CurvesPrimitive *curves, const PrimitiveVariable &primVar
	) const
	{
		const std::vector<T> &in = data->readable();
		typename S<std::vector<T>>::Ptr newOut = new S<std::vector<T>>();


		int adjustment = primVar.interpolation == PrimitiveVariable::Vertex ? m_vertexAdjustment : m_varyingAdjustment;

		adjustEndPoints( in, newOut->writable(), curves, primVar.interpolation, adjustment );

		setGeometricInterpretation( newOut.get(), getGeometricInterpretation( data ) );

		return newOut;
	}

	IECore::DataPtr operator()( const Data *data, const CurvesPrimitive *curves, const PrimitiveVariable & ) const
	{
		throw IECore::Exception( "DuplicateEndPoints : Unsupported Data type" );
	}

	int m_vertexAdjustment, m_varyingAdjustment;

};


} // namespace

CurvesPrimitivePtr IECoreScene::CurvesAlgo::updateEndpointMultiplicity( const IECoreScene::CurvesPrimitive *curves, const IECore::CubicBasisf &cubicBasis, const Canceller *canceller )
{
	CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive();

	int vertexAdjustment = requiredMultiplicity( cubicBasis ) - requiredMultiplicity( curves->basis() );

	int segmentsRequiredChange =
		(int)CurvesPrimitive::numSegments( cubicBasis, curves->periodic(), 4 ) -
		(int)CurvesPrimitive::numSegments( curves->basis(), curves->periodic(), 4 );

	int varyingAdjustment = vertexAdjustment + segmentsRequiredChange / 2;

	if( vertexAdjustment == 0 && varyingAdjustment == 0 )
	{
		return curves->copy();
	}

	DuplicateEndPoints endPointDuplicator( vertexAdjustment, varyingAdjustment );

	// enqueue a task for each primitive variable and another for the topology update
	// note we have to write the new primvars to tbb concurrent_unordered map before updating the primitives primvars in serial
	// todo : we should be able to thread the adjustEndPoint function but this would require a parallel reduce to calculate
	// offsets.
	tbb::task_group taskGroup;
	tbb::concurrent_unordered_map<std::string, IECoreScene::PrimitiveVariable> newPrimVars;
	for( const auto &it : curves->variables )
	{
		if(
			it.second.interpolation == IECoreScene::PrimitiveVariable::Vertex ||
			it.second.interpolation == PrimitiveVariable::Varying ||
			it.second.interpolation == PrimitiveVariable::FaceVarying
		)
		{
			auto f = [it, &endPointDuplicator, curves, &newPrimVars, canceller]()
			{
				Canceller::check( canceller );
				if( it.second.indices )
				{
					auto newIndices = IECore::runTimeCast<IECore::IntVectorData>( IECore::dispatch( it.second.indices.get(), endPointDuplicator, curves, it.second ) );
					newPrimVars[it.first] = IECoreScene::PrimitiveVariable( it.second.interpolation, it.second.data, newIndices );
				}
				else
				{
					auto newVertexData = IECore::dispatch( it.second.data.get(), endPointDuplicator, curves, it.second );
					newPrimVars[it.first] = IECoreScene::PrimitiveVariable( it.second.interpolation, newVertexData );
				}
			};

			taskGroup.run( f );
		}
		else
		{
			newCurves->variables[it.first] = it.second;
		}
	}

	auto newTopology = curves->verticesPerCurve()->copy();
	auto updateTopology = [&newTopology, vertexAdjustment, canceller]()
	{
		Canceller::check( canceller );
		for( auto &i : newTopology->writable() )
		{
			i += 2 * vertexAdjustment;
		}
	};

	taskGroup.run( updateTopology );
	taskGroup.wait();

	for( const auto &i : newPrimVars )
	{
		newCurves->variables[i.first] = i.second;
	}

	newCurves->setTopology( newTopology, cubicBasis, curves->periodic() );

	return newCurves;
}
