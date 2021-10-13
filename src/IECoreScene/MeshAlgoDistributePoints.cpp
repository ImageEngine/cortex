//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshPrimitiveEvaluator.h"
#include "IECoreScene/PrimitiveVariable.h"

#include "IECore/PointDistribution.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TriangleAlgo.h"

#include "tbb/blocked_range.h"
#include "tbb/parallel_reduce.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Distribute Points
//////////////////////////////////////////////////////////////////////////

namespace
{

MeshPrimitivePtr processMesh( const MeshPrimitive *mesh, const std::string &densityMask, const std::string &uvSet, const std::string &position, const Canceller *canceller )
{
	if( !mesh )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh is not valid" );
	}

	const V3fVectorData *positions = mesh->variableData<const V3fVectorData>( position );
	if ( !positions )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::distributePoints : MeshPrimitive has no suitable \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	MeshPrimitivePtr result = MeshAlgo::triangulate( mesh, canceller );

	if ( !result || !result->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh could not be triangulated" );
	}

	result->variables["faceArea"] = MeshAlgo::calculateFaceArea( result.get(), position, canceller );
	if( result->variables.find( uvSet ) != result->variables.end() )
	{
		result->variables["textureArea"] = MeshAlgo::calculateFaceTextureArea( result.get(), uvSet, position, canceller );
	}

	if( result->variables.find( densityMask ) == result->variables.end() )
	{
		result->variables[densityMask] = PrimitiveVariable( PrimitiveVariable::Constant, new FloatData( 1.0 ) );
	}

	return result;
}

struct Emitter
{
	public :

		Emitter( MeshPrimitiveEvaluator *evaluator, const PrimitiveVariable &densityVar, std::vector<Imath::V3f> &positions, size_t triangleIndex, const Imath::V2f &v0, const Imath::V2f &v1, const Imath::V2f &v2, const Canceller *canceller )
			: m_meshEvaluator( evaluator ), m_densityVar( densityVar ), m_p( positions ), m_triangleIndex( triangleIndex ), m_v0( v0 ), m_v1( v1 ), m_v2( v2 ), m_canceller( canceller ), m_cancelCounter( 0 )
		{

			m_evaluatorResult = boost::static_pointer_cast<MeshPrimitiveEvaluator::Result>( m_meshEvaluator->createResult() );
		}

		void operator() ( const Imath::V2f pos, float densityThreshold )
		{
			Imath::V3f bary;
			m_cancelCounter++;
			if( m_canceller && ( m_cancelCounter % 1000 ) == 0 )
			{
				Canceller::check( m_canceller );
			}
			if( triangleContainsPoint( m_v0, m_v1, m_v2, pos, bary ) )
			{
				m_meshEvaluator->barycentricPosition( m_triangleIndex, bary, m_evaluatorResult.get() );
				if ( m_evaluatorResult->floatPrimVar( m_densityVar ) >= densityThreshold )
				{
					m_p.push_back( m_evaluatorResult->point() );
				}
			}
		}

	private :

		MeshPrimitiveEvaluator *m_meshEvaluator;
		const PrimitiveVariable &m_densityVar;
		std::vector<Imath::V3f> &m_p;
		size_t m_triangleIndex;
		Imath::V2f m_v0;
		Imath::V2f m_v1;
		Imath::V2f m_v2;
		const Canceller *m_canceller;
		int m_cancelCounter;

		MeshPrimitiveEvaluator::ResultPtr m_evaluatorResult;

};

struct Generator
{
	public :

		Generator( MeshPrimitiveEvaluator *evaluator, const PrimitiveVariable::IndexedView<Imath::V2f> &uvs, bool faceVaryingUVs, const std::vector<float> &faceArea, const std::vector<float> &textureArea, float density, const PrimitiveVariable &densityVar, Imath::V2f offset, const Canceller *canceller )
			: m_meshEvaluator( evaluator ), m_uvs( uvs ), m_faceVaryingUVs( faceVaryingUVs ), m_faceArea( faceArea ), m_textureArea( textureArea ), m_density( density ), m_densityVar( densityVar ), m_offset( offset ), m_canceller( canceller ), m_positions()
		{
		}

		Generator( Generator &that, tbb::split )
			: m_meshEvaluator( that.m_meshEvaluator ), m_uvs( that.m_uvs ), m_faceVaryingUVs( that.m_faceVaryingUVs ), m_faceArea( that.m_faceArea ), m_textureArea( that.m_textureArea ), m_density( that.m_density ), m_densityVar( that.m_densityVar ), m_offset( that.m_offset ), m_canceller( that.m_canceller ), m_positions()
		{
		}

		void operator()( const tbb::blocked_range<size_t> &r )
		{
			Canceller::check( m_canceller );
			for ( size_t i=r.begin(); i!=r.end(); ++i )
			{
				float textureDensity = m_density * m_faceArea[i] / m_textureArea[i];

				size_t v0I = i * 3;
				size_t v1I = v0I + 1;
				size_t v2I = v1I + 1;

				Imath::V2f uv0, uv1, uv2;
			
				if( m_faceVaryingUVs )
				{
					uv0 = m_uvs[v0I];
					uv1 = m_uvs[v1I];
					uv2 = m_uvs[v2I];
				}
				else
				{
					const std::vector<int> &ids = m_meshEvaluator->mesh()->vertexIds()->readable();
					uv0 = m_uvs[ids[v0I]];
					uv1 = m_uvs[ids[v1I]];
					uv2 = m_uvs[ids[v2I]];
				}

				uv0 += m_offset;
				uv1 += m_offset;
				uv2 += m_offset;

				Imath::Box2f uvBounds;
				uvBounds.extendBy( uv0 );
				uvBounds.extendBy( uv1 );
				uvBounds.extendBy( uv2 );

				Emitter emitter( m_meshEvaluator, m_densityVar, m_positions, i, uv0, uv1, uv2, m_canceller );
				PointDistribution::defaultInstance()( uvBounds, textureDensity, emitter );
			}
		}

		void join( Generator &that )
		{
			size_t numThisPoints = this->m_positions.size();
			Canceller::check( m_canceller );
			this->m_positions.resize( numThisPoints + that.m_positions.size() );
			Canceller::check( m_canceller );
			std::copy( that.m_positions.begin(), that.m_positions.end(), this->m_positions.begin() + numThisPoints );
		}

		std::vector<Imath::V3f> &positions()
		{
			return m_positions;
		}

	private :

		MeshPrimitiveEvaluator *m_meshEvaluator;
		const PrimitiveVariable::IndexedView<Imath::V2f> &m_uvs;
		bool m_faceVaryingUVs;
		const std::vector<float> &m_faceArea;
		const std::vector<float> &m_textureArea;
		float m_density;
		const PrimitiveVariable &m_densityVar;
		Imath::V2f m_offset;
		const Canceller *m_canceller;

		std::vector<Imath::V3f> m_positions;

};

} // namespace

PointsPrimitivePtr MeshAlgo::distributePoints( const MeshPrimitive *mesh, float density, const Imath::V2f &offset, const std::string &densityMask, const std::string &uvSet, const std::string &position, const Canceller *canceller )
{
	if( density < 0 )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The density of the distribution cannot be negative." );
	}

	MeshPrimitivePtr updatedMesh = processMesh( mesh, densityMask, uvSet, position, canceller );
	MeshPrimitiveEvaluatorPtr meshEvaluator = new MeshPrimitiveEvaluator( updatedMesh );

	bool faceVaryingUVs = true;
	boost::optional<PrimitiveVariable::IndexedView<V2f> > uvView = updatedMesh->variableIndexedView<V2fVectorData>( uvSet, PrimitiveVariable::FaceVarying, /* throwOnInvalid */ false );
	if( !uvView )
	{
		faceVaryingUVs = false;
		uvView = updatedMesh->variableIndexedView<V2fVectorData>( uvSet, PrimitiveVariable::Vertex, /* throwOnInvalid */ false );
	}

	if( !uvView )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::distributePoints : MeshPrimitive has no uv primitive variable named \"%s\" of type FaceVarying or Vertex." ) % uvSet );
		throw InvalidArgumentException( e );
	}

	const std::vector<float> &faceArea = updatedMesh->variableData<FloatVectorData>( "faceArea", PrimitiveVariable::Uniform )->readable();
	const std::vector<float> &textureArea = updatedMesh->variableData<FloatVectorData>( "textureArea", PrimitiveVariable::Uniform )->readable();
	const PrimitiveVariable &densityVar = updatedMesh->variables.find( densityMask )->second;

	size_t numFaces = updatedMesh->verticesPerFace()->readable().size();
	Generator gen( meshEvaluator.get(), *uvView, faceVaryingUVs, faceArea, textureArea, density, densityVar, offset, canceller );

	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
	tbb::auto_partitioner partitioner;
	tbb::parallel_reduce( tbb::blocked_range<size_t>( 0, numFaces ), gen, partitioner, taskGroupContext );


	Canceller::check( canceller );
	V3fVectorDataPtr pData = new V3fVectorData();
	pData->writable().swap( gen.positions() );

	Canceller::check( canceller );
	return new PointsPrimitive( pData );
}
