//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ContrastSmoothSkinningWeightsOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompressSmoothSkinningDataOp.h"
#include "IECore/DecompressSmoothSkinningDataOp.h"
#include "IECore/Interpolator.h"
#include "IECore/NormalizeSmoothSkinningWeightsOp.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/Math.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ContrastSmoothSkinningWeightsOp );

ContrastSmoothSkinningWeightsOp::ContrastSmoothSkinningWeightsOp()
	: ModifyOp(
		"The ContrastSmoothSkinningWeightsOp contrasts the weights of SmoothSkinningData by applying a custom step function on the weights. This op does not normalize the weights.",
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
	
	m_contrastRatioParameter = new FloatParameter(
		"contrastRatio",
		"Controls the level of contrast. Interpolates between linear function and smooth step function.",
		1,
		0.0,
		1.0
	);

	m_contrastCenterParameter = new FloatParameter(
		"contrastCenter",
		"Sets the middle value for the contrast. Weights lower than that will tend zero and the higher ones will tend to one.",
		0.5,
		1e-3,
		1.0 - 1e-3
	);
	
	m_iterationsParameter = new IntParameter(
		"iterations",
		"The number of iterations to perform the contrast operation.",
		3,
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
	parameters()->addParameter( m_contrastRatioParameter );
	parameters()->addParameter( m_contrastCenterParameter );
	parameters()->addParameter( m_iterationsParameter );
	parameters()->addParameter( m_useLocksParameter );
	parameters()->addParameter( m_influenceLocksParameter );
}

ContrastSmoothSkinningWeightsOp::~ContrastSmoothSkinningWeightsOp()
{
}

// Custom smoothstep class. Defines the neutral center point, interpolates it with the linear function and applies multiple iterations.
class ContrastSmoothSkinningWeightsOp::ContrastSmoothStep
{
	public:

		ContrastSmoothStep( int iterations, float ratio, float center ) :
								m_iterations(iterations), m_ratio(ratio), m_center(center)
		{
			m_firstHalfScale = 0.5 / m_center;
			m_secondHalfScale = 0.5 / (1.0 - m_center);
			m_norm = 0.5*m_firstHalfScale+0.5*m_secondHalfScale;
		}

		float operator()( float value )
		{
			for ( int i = 0; i < m_iterations; i++ )
			{
				float result;
				if ( value >= m_center )
				{
					result = ((smoothstep(0.0,1.0,(value-m_center)*m_secondHalfScale+0.5) - 0.5) * m_firstHalfScale + m_secondHalfScale*0.5) / m_norm;
				}
				else
				{
					result = (smoothstep(0.0,1.0,(value-m_center)*m_firstHalfScale+0.5) * m_secondHalfScale) / m_norm;
				}
				value = m_ratio*result + (1-m_ratio)*value;
			}
			return value;
		}

	private :

		int m_iterations;
		float m_ratio;
		float m_center;
		float m_firstHalfScale;
		float m_secondHalfScale;
		float m_norm;
};

void ContrastSmoothSkinningWeightsOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );
	
	// \todo: remove decompression/compression.
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
		throw IECore::Exception( "ContrastSmoothSkinningWeightsOp: The given mesh is not valid" );
	}
	int numMeshVerts = mesh->variableSize( PrimitiveVariable::Vertex );
	
	// make sure the mesh matches the skinning data
	if ( numMeshVerts != numSsdVerts )
	{
		throw IECore::Exception( "ContrastSmoothSkinningWeightsOp: The input SmoothSkinningData and mesh have a different number of vertices" );
	}
	
	bool useLocks = m_useLocksParameter->getTypedValue();
	std::vector<bool> &locks = m_influenceLocksParameter->getTypedValue();
		
	// make sure there is one lock per influence
	if ( useLocks && ( locks.size() != skinningData->influenceNames()->readable().size() ) )
	{
		throw IECore::Exception( "ContrastSmoothSkinningWeightsOp: There must be exactly one lock per influence" );
	}
	
	if ( !useLocks )
	{
		locks.clear();
		locks.resize( skinningData->influenceNames()->readable().size(), false );
	}
	
#if defined(_WIN32)
	std::vector<int> vertexIds;
#else
	std::vector<int64_t> vertexIds;
#endif
	m_vertexIdsParameter->getFrameListValue()->asList( vertexIds );
	
	// make sure all vertex ids are valid
	for ( unsigned i=0; i < vertexIds.size(); i++ )
	{
		if ( vertexIds[i] > numSsdVerts )
		{
			throw IECore::Exception( ( boost::format( "ContrastSmoothSkinningWeightsOp: VertexId \"%d\" is outside the range of the SmoothSkinningData and mesh" ) % vertexIds[i] ).str() );
		}
	}
	
	float contrastRatio = m_contrastRatioParameter->getNumericValue();
	float contrastCenter = m_contrastCenterParameter->getNumericValue();
	int numIterations = m_iterationsParameter->getNumericValue();

	ContrastSmoothStep contrastFunction( numIterations, contrastRatio, contrastCenter );
	
	// an empty vertexId list means we operate over all vertices
	if ( vertexIds.size() == 0 )
	{
		for ( unsigned int i=0; i < pointInfluenceWeights.size(); i++ )
		{
			unsigned int current = i;
			if ( !locks[ pointInfluenceIndices[current] ] )
			{
				pointInfluenceWeights[current] = contrastFunction( pointInfluenceWeights[ current ] );
			}
		}
	}
	else
	{
		// apply contrast with the per-influence locks
		for ( unsigned i=0; i < vertexIds.size(); i++ )
		{
			int currentVertId = vertexIds[i];
			for ( int j=0; j < pointInfluenceCounts[currentVertId]; j++ )
			{
				int current = pointIndexOffsets[currentVertId] + j;
				if ( !locks[ pointInfluenceIndices[current] ] )
				{
					pointInfluenceWeights[current] = contrastFunction( pointInfluenceWeights[ current ] );
				}
			}
		}
	}
	
	// re-compress
	CompressSmoothSkinningDataOp compressionOp;
	compressionOp.inputParameter()->setValidatedValue( skinningData );
	compressionOp.copyParameter()->setTypedValue( false );
	compressionOp.operate();
}
