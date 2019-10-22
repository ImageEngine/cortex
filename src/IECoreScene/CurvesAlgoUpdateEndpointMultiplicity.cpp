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

// replicate the first and last values an additional two times at the begining and end of the vector.
template<typename T>
void expandVertex( const std::vector<T> &in, std::vector<T> &out, const CurvesPrimitive *curves, const PrimitiveVariable &primVar, const IECore::CubicBasisf &targetBasis )
{
	size_t numCurves = curves->numCurves();
	out.reserve( in.size() + numCurves * 4 );

	size_t index = 0;
	for( size_t i = 0; i < numCurves; ++i )
	{
		size_t numSegments = CurvesPrimitive::numSegments( targetBasis, curves->periodic(), curves->variableSize( PrimitiveVariable::Vertex, i ) + 4 );

		// duplicate the first 2 values
		out.push_back( in[index] );
		out.push_back( in[index] );

		for( size_t j = 1; j < numSegments; ++j, ++index )
		{
			out.push_back( in[index] );
		}

		// duplicate the last 2 values (note we've already incremented index for the next curve)
		out.push_back( in[index-1] );
		out.push_back( in[index-1] );
	}
}

// remove the replicated first and last values reversing the 'expand' function above
template<typename T>
void compressVertex( const std::vector<T> &in, std::vector<T> &out, const CurvesPrimitive *curves, const PrimitiveVariable &primVar, const IECore::CubicBasisf &targetBasis )
{
	size_t numCurves = curves->numCurves();
	out.reserve( in.size() - numCurves * 4 );

	size_t index = 0;
	for( size_t i = 0; i < numCurves; ++i )
	{
		size_t numSegments = CurvesPrimitive::numSegments( targetBasis, curves->periodic(), curves->variableSize( PrimitiveVariable::Vertex, i ) - 4 );

		// skip the first two values
		index += 2;

		for( size_t j = 0; j < numSegments + 1; ++j, ++index )
		{
			out.push_back( in[index] );
		}

		// skip the last two values
		index += 2;
	}
}

template<typename T>
void expandVarying( const std::vector<T> &in, std::vector<T> &out, const CurvesPrimitive *curves, const PrimitiveVariable &primVar, const IECore::CubicBasisf &targetBasis )
{
	size_t numCurves = curves->numCurves();
	out.reserve( in.size() + numCurves * 2 );

	size_t index = 0;
	for( size_t i = 0; i < numCurves; ++i )
	{
		size_t numSegments = CurvesPrimitive::numSegments( targetBasis, curves->periodic(), curves->variableSize( PrimitiveVariable::Vertex, i ) + 4 );

		// duplicate the first segment value
		out.push_back( in[index] );

		for( size_t j = 1; j < numSegments; ++j, ++index )
		{
			out.push_back( in[index] );
		}

		// duplicate the last segment value (note we've already incremented index for the next curve)
		out.push_back( in[index-1] );
	}
}

// remove the replicated first and last values reversing the 'expand' function above
template<typename T>
void compressVarying( const std::vector<T> &in, std::vector<T> &out, const CurvesPrimitive *curves, const PrimitiveVariable &primVar, const IECore::CubicBasisf &targetBasis )
{
	size_t numCurves = curves->numCurves();
	out.reserve( in.size() - numCurves * 2 );

	size_t index = 0;
	for( size_t i = 0; i < numCurves; ++i )
	{
		size_t numSegments = CurvesPrimitive::numSegments( targetBasis, curves->periodic(), curves->variableSize( PrimitiveVariable::Vertex, i ) - 4 );

		// skip the first segment value
		++index;

		for( size_t j = 0; j < numSegments + 1; ++j, ++index )
		{
			out.push_back( in[index] );
		}

		// skip the last segment value
		++index;
	}
}

struct DuplicateEndPoints
{
	DuplicateEndPoints( const IECore::CubicBasisf &targetBasis, bool expand ) : m_targetBasis( targetBasis ), m_expand( expand )
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

		if( primVar.interpolation == PrimitiveVariable::Vertex )
		{
			if( m_expand )
			{
				expandVertex( in, newOut->writable(), curves, primVar, m_targetBasis );
			}
			else
			{
				compressVertex( in, newOut->writable(), curves, primVar, m_targetBasis );
			}
		}
		else if( primVar.interpolation == PrimitiveVariable::Varying || primVar.interpolation == PrimitiveVariable::FaceVarying )
		{
			if( m_expand )
			{
				expandVarying( in, newOut->writable(), curves, primVar, m_targetBasis );
			}
			else
			{
				compressVarying( in, newOut->writable(), curves, primVar, m_targetBasis );
			}
		}

		setGeometricInterpretation( newOut.get(), getGeometricInterpretation( data ) );

		return newOut;
	}

	IECore::DataPtr operator()( const Data *data, const CurvesPrimitive *curves, const PrimitiveVariable & ) const
	{
		throw IECore::Exception( "DuplicateEndPoints : Unsupported Data type" );
	}

	IECore::CubicBasisf m_targetBasis;
	bool m_expand;

};

} // namespace

CurvesPrimitivePtr IECoreScene::CurvesAlgo::updateEndpointMultiplicity( const IECoreScene::CurvesPrimitive *curves, const IECore::CubicBasisf &cubicBasis )
{
	CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive();

	bool expand;
	if( ( cubicBasis == IECore::CubicBasisf::catmullRom() || cubicBasis == IECore::CubicBasisf::bezier() || cubicBasis == IECore::CubicBasisf::bSpline() ) &&
		curves->basis() == IECore::CubicBasisf::linear() )
	{
		expand = true;
	}
	else if( (
		curves->basis() == IECore::CubicBasisf::catmullRom() ||
			curves->basis() == IECore::CubicBasisf::bezier() ||
			curves->basis() == IECore::CubicBasisf::bSpline()
	) && cubicBasis == IECore::CubicBasisf::linear() )
	{
		expand = false;
	}
	else
	{
		return curves->copy();
	}

	DuplicateEndPoints endPointDuplicator( cubicBasis, expand );

	// enqueue a task for each primitive variable and another for the topology update
	// note we have to write the new primvars to tbb concurrent_unordered map before updating the primitives primvars in serial
	// todo : we should be able to thread the expand and compress functions but this would require a parallel reduce to calculate
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
			auto f = [it, &endPointDuplicator, curves, &newPrimVars]()
			{
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
	auto updateTopology = [&newTopology, expand]()
	{
		for( auto &i : newTopology->writable() )
		{
			i += expand ? 4 : -4;
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
