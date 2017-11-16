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

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreScene/LimitSmoothSkinningInfluencesOp.h"
#include "IECoreScene/CompressSmoothSkinningDataOp.h"
#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/TypedObjectParameter.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( LimitSmoothSkinningInfluencesOp );

LimitSmoothSkinningInfluencesOp::LimitSmoothSkinningInfluencesOp()
	: ModifyOp(
		"The LimitSmoothSkinningInfluencesOp zeros the weight values of SmoothSkinningData for certain influences.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	IntParameter::PresetsContainer modePresets;
	modePresets.push_back( IntParameter::Preset( "WeightLimit", LimitSmoothSkinningInfluencesOp::WeightLimit ) );
	modePresets.push_back( IntParameter::Preset( "MaxInfluences", LimitSmoothSkinningInfluencesOp::MaxInfluences ) );
	modePresets.push_back( IntParameter::Preset( "Indexed", LimitSmoothSkinningInfluencesOp::Indexed ) );

	m_modeParameter = new IntParameter(
		"mode",
		"The mode of influence limiting. Options are to impose a minimum weight, a maximum number of influences per point, or to zero specific influences for all points",
		LimitSmoothSkinningInfluencesOp::WeightLimit,
		LimitSmoothSkinningInfluencesOp::WeightLimit,
		LimitSmoothSkinningInfluencesOp::Indexed,
		modePresets,
		true
	);

	m_compressionParameter = new BoolParameter(
		"compressResult",
		"True if the result should be compressed using the CompressSmoothSkinningDataOp",
		true
	);

	m_minWeightParameter = new FloatParameter(
		"minWeight",
		"The minimum weight an influence is allowed per point. This parameter is only used in WeightLimit mode",
		0.001f,
		0.0f
	);

	m_maxInfluencesParameter = new IntParameter(
		"maxInfluences",
		"The maximum number of influences per point. This parameter is only used in MaxInfluences mode",
		3,
		0
	);

	m_influenceIndicesParameter = new FrameListParameter(
		"influenceIndices",
		"The indices of influences to zero corresponding to the names in input.influenceNames(). This parameter is only used in Indexed mode",
		""
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

	parameters()->addParameter( m_modeParameter );
	parameters()->addParameter( m_compressionParameter );
	parameters()->addParameter( m_minWeightParameter );
	parameters()->addParameter( m_maxInfluencesParameter );
	parameters()->addParameter( m_influenceIndicesParameter );
	parameters()->addParameter( m_useLocksParameter );
	parameters()->addParameter( m_influenceLocksParameter );
}

LimitSmoothSkinningInfluencesOp::~LimitSmoothSkinningInfluencesOp()
{
}

void LimitSmoothSkinningInfluencesOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();

	std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->writable();

	bool useLocks = m_useLocksParameter->getTypedValue();
	std::vector<bool> &locks = m_influenceLocksParameter->getTypedValue();

	int mode = m_modeParameter->getNumericValue();

	// make sure there is one lock per influence
	if ( useLocks && ( locks.size() != skinningData->influenceNames()->readable().size() ) && ( mode != LimitSmoothSkinningInfluencesOp::Indexed ) )
	{
		throw IECore::Exception( "LimitSmoothSkinningInfluencesOp: There must be exactly one lock per influence" );
	}

	if ( !useLocks )
	{
		locks.clear();
		locks.resize( skinningData->influenceNames()->readable().size(), false );
	}

	// Limit influences based on minumum allowable weight
	if ( mode == LimitSmoothSkinningInfluencesOp::WeightLimit )
	{
		float minWeight = m_minWeightParameter->getNumericValue();

		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			for ( int j=0; j < pointInfluenceCounts[i]; j++ )
			{
				int current = pointIndexOffsets[i] + j;

				if ( !locks[ pointInfluenceIndices[current] ] && (pointInfluenceWeights[current] < minWeight) )
				{
					pointInfluenceWeights[current] = 0.0f;
				}
			}
		}
	}
	// Limit the number of influences per point
	else if ( mode == LimitSmoothSkinningInfluencesOp::MaxInfluences )
	{
		int maxInfluences = m_maxInfluencesParameter->getNumericValue();
		std::vector<int> influencesToLimit;

		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			int numToLimit = pointInfluenceCounts[i] - maxInfluences;
			if ( numToLimit <= 0 )
			{
				continue;
			}

			influencesToLimit.clear();

			for ( int j=0; j < numToLimit; j++ )
			{
				int indexOfMin = -1;
				float minWeight = Imath::limits<float>::max();

				for ( int k=0; k < pointInfluenceCounts[i]; k++ )
				{
					int current = pointIndexOffsets[i] + k;

					if ( locks[ pointInfluenceIndices[current] ] || find( influencesToLimit.begin(), influencesToLimit.end(), current ) != influencesToLimit.end() )
					{
						continue;
					}

					float weight = pointInfluenceWeights[current];

					if ( weight < minWeight )
					{
						minWeight = weight;
						indexOfMin = current;
					}
				}

				if ( indexOfMin == -1 )
				{
					break;
				}

				influencesToLimit.push_back( indexOfMin );
			}

			for ( unsigned j=0; j < influencesToLimit.size(); j++ )
			{
				pointInfluenceWeights[ influencesToLimit[j] ] = 0.0f;
			}
		}
	}
	// Zero specific influences for all points
	else if ( mode == LimitSmoothSkinningInfluencesOp::Indexed )
	{
		std::vector<int64_t> indicesToLimit;
		m_influenceIndicesParameter->getFrameListValue()->asList( indicesToLimit );
		std::vector<bool> limitIndex( skinningData->influenceNames()->readable().size(), false );
		for ( unsigned i=0; i < indicesToLimit.size(); i++ )
		{
			limitIndex[ indicesToLimit[i] ] = true;
		}

		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			for ( int j=0; j < pointInfluenceCounts[i]; j++ )
			{
				int current = pointIndexOffsets[i] + j;

				if ( limitIndex[ pointInfluenceIndices[current] ] )
				{
					pointInfluenceWeights[current] = 0.0f;
				}
			}
		}
	}
	else
	{
		throw IECore::Exception( ( boost::format( "LimitSmoothSkinningInfluencesOp: \"%d\" is not a recognized mode" ) % mode ).str() );
	}

	if ( m_compressionParameter->getTypedValue() )
	{
		CompressSmoothSkinningDataOp compressionOp;
		compressionOp.inputParameter()->setValidatedValue( skinningData );
		compressionOp.copyParameter()->setTypedValue( false );
		compressionOp.operate();
	}
}
