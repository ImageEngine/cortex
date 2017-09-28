//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>
#include <cassert>

#include "IECore/SmoothSmoothSkinningWeightsOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompressSmoothSkinningDataOp.h"
#include "IECore/DecompressSmoothSkinningDataOp.h"
#include "IECore/Interpolator.h"
#include "IECore/NormalizeSmoothSkinningWeightsOp.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypedObjectParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( SmoothSmoothSkinningWeightsOp );

SmoothSmoothSkinningWeightsOp::SmoothSmoothSkinningWeightsOp()
	: ModifyOp(
		"The SmoothSmoothSkinningWeightsOp smooths the weights of SmoothSkinningData using the average weights from connected vertices",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh primitive corresponding to the input SmoothSkinningData",
		new MeshPrimitive
	);

	m_vertexIdsParameter = new FrameListParameter(
		"vertexIndices",
		"The indices of the vertices to smooth. All vertices will be smoothed if this parameter is empty",
		""
	);

	m_smoothingRatioParameter = new FloatParameter(
		"smoothingRatio",
		"Controls the level of smoothing. Higher values give greater weight to neighbour vertices",
		0.5,
		0.0,
		1.0
	);

	m_iterationsParameter = new IntParameter(
		"iterations",
		"The number of iterations to perform the smoothing operation",
		1,
		1
	);

	m_useLocksParameter = new BoolParameter(
		"applyLocks",
		"Whether or not influenceLocks should be applied",
		true
	);

	m_influenceLocksParameter = new BoolVectorParameter(
		"influenceLocks",
		"A per-influence list of lock values",
		new BoolVectorData
	);

	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_vertexIdsParameter );
	parameters()->addParameter( m_smoothingRatioParameter );
	parameters()->addParameter( m_iterationsParameter );
	parameters()->addParameter( m_useLocksParameter );
	parameters()->addParameter( m_influenceLocksParameter );
}

SmoothSmoothSkinningWeightsOp::~SmoothSmoothSkinningWeightsOp()
{
}

void SmoothSmoothSkinningWeightsOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	// decompress
	DecompressSmoothSkinningDataOp decompressionOp;
	decompressionOp.inputParameter()->setValidatedValue( skinningData );
	decompressionOp.copyParameter()->setTypedValue( false );
	decompressionOp.operate();

	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	int numSsdVerts = pointIndexOffsets.size();

	std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->writable();

	const MeshPrimitive *mesh = runTimeCast<const MeshPrimitive>( m_meshParameter->getValidatedValue() );
	if ( !mesh )
	{
		throw IECore::Exception( "SmoothSmoothSkinningWeightsOp: The given mesh is not valid" );
	}
	int numMeshVerts = mesh->variableSize( PrimitiveVariable::Vertex );
	const std::vector<int> &meshVertexIds = mesh->vertexIds()->readable();

	// make sure the mesh matches the skinning data
	if ( numMeshVerts != numSsdVerts )
	{
		throw IECore::Exception( "SmoothSmoothSkinningWeightsOp: The input SmoothSkinningData and mesh have a different number of vertices" );
	}

	bool useLocks = m_useLocksParameter->getTypedValue();
	std::vector<bool> &locks = m_influenceLocksParameter->getTypedValue();

	// make sure there is one lock per influence
	if ( useLocks && ( locks.size() != skinningData->influenceNames()->readable().size() ) )
	{
		throw IECore::Exception( "SmoothSmoothSkinningWeightsOp: There must be exactly one lock per influence" );
	}

	if ( !useLocks )
	{
		locks.clear();
		locks.resize( skinningData->influenceNames()->readable().size(), false );
	}

	std::vector<int64_t> vertexIds;
	m_vertexIdsParameter->getFrameListValue()->asList( vertexIds );

	// make sure all vertex ids are valid
	for ( unsigned i=0; i < vertexIds.size(); i++ )
	{
		if ( vertexIds[i] > numSsdVerts )
		{
			throw IECore::Exception( ( boost::format( "SmoothSmoothSkinningWeightsOp: VertexId \"%d\" is outside the range of the SmoothSkinningData and mesh" ) % vertexIds[i] ).str() );
		}
	}

	// an empty vertexId list means we smooth all vertices
	if ( vertexIds.size() == 0 )
	{
		for ( int i=0; i < numSsdVerts; i++ )
		{
			vertexIds.push_back( i );
		}
	}

	// add the mesh vertices to the neighbourhood graph
	/// \todo: consider moving this mesh connectivity graphing to the MeshPrimitive
	Graph meshGraph;
	for ( int i=0; i < numMeshVerts; i++ )
	{
		boost::add_vertex( meshGraph );
	}

	// add the mesh edges to the neighbourhood graph
	int v = 0, v1, v2;
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();
	unsigned numFaces = verticesPerFace.size();
	for ( unsigned f=0; f < numFaces; f++ )
	{
		for ( int fv=0; fv < verticesPerFace[f] - 1; fv++ )
		{
			v1 = meshVertexIds[v+fv];
			v2 = meshVertexIds[v+fv+1];

			Vertex source = boost::vertex( v1, meshGraph );
			Vertex target = boost::vertex( v2, meshGraph );
			if ( !boost::edge( source, target, meshGraph ).second )
			{
				boost::add_edge( source, target, meshGraph );
			}
		}

		v1 = meshVertexIds[v + verticesPerFace[f] - 1];
		v2 = meshVertexIds[v];
		Vertex source = boost::vertex( v1, meshGraph );
		Vertex target = boost::vertex( v2, meshGraph );
		if ( !boost::edge( source, target, meshGraph ).second )
		{
			boost::add_edge( source, target, meshGraph );
		}

		v += verticesPerFace[f];
	}

	// get the property map that relates each Graph Vertex to a vertexId
	VertexIdMap vertexIdMap = boost::get( boost::vertex_index, meshGraph );

	std::vector<float> smoothInfluenceWeights( skinningData->pointInfluenceWeights()->readable().size(), 0.0f );
	LinearInterpolator<float> lerp;
	float smoothingRatio = m_smoothingRatioParameter->getNumericValue();
	int numIterations = m_iterationsParameter->getNumericValue();

	NormalizeSmoothSkinningWeightsOp normalizeOp;
	normalizeOp.copyParameter()->setTypedValue( false );
	normalizeOp.parameters()->setParameterValue( "applyLocks", m_useLocksParameter->getValue() );
	normalizeOp.parameters()->setParameterValue( "influenceLocks", m_influenceLocksParameter->getValue() );

	// iterate
	for ( int iteration=0; iteration < numIterations; iteration++ )
	{
		// smooth the weights
		for ( unsigned i=0; i < vertexIds.size(); i++ )
		{
			int currentVertId = vertexIds[i];
			Vertex currentVert = boost::vertex( currentVertId, meshGraph );
			NeighbourIteratorRange neighbourhood = boost::adjacent_vertices( currentVert, meshGraph );
			float numNeighbours = boost::out_degree( currentVert, meshGraph );

			for ( int j=0; j < pointInfluenceCounts[currentVertId]; j++ )
			{
				int current = pointIndexOffsets[currentVertId] + j;

				// calculate the average neighbour weight
				float totalNeighbourWeight = 0.0f;
				for( NeighbourIterator nIt = neighbourhood.first; nIt != neighbourhood.second; nIt++ )
				{
					int neighbourId = vertexIdMap[*nIt];
					float currentNeighbourWeight = pointInfluenceWeights[ pointIndexOffsets[neighbourId] + j ];
					totalNeighbourWeight += currentNeighbourWeight;
				}
				float averageNeighbourWeight = totalNeighbourWeight / numNeighbours;

				lerp( pointInfluenceWeights[current], averageNeighbourWeight, smoothingRatio, smoothInfluenceWeights[current] );
			}
		}

		// apply the per-influence locks
		for ( unsigned i=0; i < vertexIds.size(); i++ )
		{
			int currentVertId = vertexIds[i];

			for ( int j=0; j < pointInfluenceCounts[currentVertId]; j++ )
			{
				int current = pointIndexOffsets[currentVertId] + j;

				if ( !locks[ pointInfluenceIndices[current] ] )
				{
					pointInfluenceWeights[current] = smoothInfluenceWeights[current];
				}
			}
		}

		// normalize
		normalizeOp.inputParameter()->setValidatedValue( skinningData );
		normalizeOp.operate();
	}

	// re-compress
	CompressSmoothSkinningDataOp compressionOp;
	compressionOp.inputParameter()->setValidatedValue( skinningData );
	compressionOp.copyParameter()->setTypedValue( false );
	compressionOp.operate();
}
