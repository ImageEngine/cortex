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

#include "IECore/RemoveSmoothSkinningInfluencesOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompressSmoothSkinningDataOp.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypedObjectParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( RemoveSmoothSkinningInfluencesOp );

RemoveSmoothSkinningInfluencesOp::RemoveSmoothSkinningInfluencesOp()
	: ModifyOp(
		"The RemoveSmoothSkinningInfluencesOp zeros the weight values of SmoothSkinningData for certain influences.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	IntParameter::PresetsContainer modePresets;
	modePresets.push_back( IntParameter::Preset( "WeightLimit", RemoveSmoothSkinningInfluencesOp::WeightLimit ) );
	modePresets.push_back( IntParameter::Preset( "MaxInfluences", RemoveSmoothSkinningInfluencesOp::MaxInfluences ) );
	modePresets.push_back( IntParameter::Preset( "Indexed", RemoveSmoothSkinningInfluencesOp::Indexed ) );
	
	m_modeParameter = new IntParameter(
		"mode",
		"The mode of removal. Options are to impose a minimum weight, a maximum number of influences per point, or to remove specific influences from all points",
		RemoveSmoothSkinningInfluencesOp::WeightLimit,
		RemoveSmoothSkinningInfluencesOp::WeightLimit,
		RemoveSmoothSkinningInfluencesOp::Indexed,
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
		"The indices of influences to remove corresponding to the names in input.influenceNames(). This parameter is only used in Indexed mode",
		""
	);
	
	parameters()->addParameter( m_modeParameter );
	parameters()->addParameter( m_compressionParameter );
	parameters()->addParameter( m_minWeightParameter );
	parameters()->addParameter( m_maxInfluencesParameter );
	parameters()->addParameter( m_influenceIndicesParameter );
}

RemoveSmoothSkinningInfluencesOp::~RemoveSmoothSkinningInfluencesOp()
{
}

void RemoveSmoothSkinningInfluencesOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );
	assert( skinningData->validate() );
		
	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	
	std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->writable();
	
	int mode = m_modeParameter->getNumericValue();
	
	// remove influences based on minumum allowable weight
	if ( mode == RemoveSmoothSkinningInfluencesOp::WeightLimit )
	{
		float minWeight = m_minWeightParameter->getNumericValue();
		
		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			for ( int j=0; j < pointInfluenceCounts[i]; j++ )
			{
				int current = pointIndexOffsets[i] + j;
				
				if ( pointInfluenceWeights[current] < minWeight )
				{
					pointInfluenceWeights[current] = 0.0f;
				}
			}
		}
	}
	// remove influences by limiting the number of influences per point
	else if ( mode == RemoveSmoothSkinningInfluencesOp::MaxInfluences )
	{
		int maxInfluences = m_maxInfluencesParameter->getNumericValue();
		std::vector<int> influencesToRemove;
		
		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			int numToRemove = pointInfluenceCounts[i] - maxInfluences;
			if ( numToRemove <= 0 )
			{
				continue;
			}
			
			influencesToRemove.clear();
			
			for ( int j=0; j < numToRemove; j++ )
			{				
				int indexOfMin;
				float minWeight = Imath::limits<float>::max();
				
				for ( int k=0; k < pointInfluenceCounts[i]; k++ )
				{
					int current = pointIndexOffsets[i] + k;
					
					if ( find( influencesToRemove.begin(), influencesToRemove.end(), current ) != influencesToRemove.end() )
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
				
				influencesToRemove.push_back( indexOfMin );
			}
			
			for ( unsigned j=0; j < influencesToRemove.size(); j++ )
			{
				pointInfluenceWeights[ influencesToRemove[j] ] = 0.0f;
			}
		}
	}
	// remove specific influences from all points
	else if ( mode == RemoveSmoothSkinningInfluencesOp::Indexed )
	{
		std::vector<long int> indicesToRemove;
		m_influenceIndicesParameter->getFrameListValue()->asList( indicesToRemove );
		std::vector<bool> removeIndex( skinningData->influenceNames()->readable().size(), false );
		for ( unsigned i=0; i < indicesToRemove.size(); i++ )
		{
			removeIndex[ indicesToRemove[i] ] = true;
		}
		
		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			for ( int j=0; j < pointInfluenceCounts[i]; j++ )
			{
				int current = pointIndexOffsets[i] + j;
				
				if ( removeIndex[ pointInfluenceIndices[current] ] )
				{
					pointInfluenceWeights[current] = 0.0f;
				}
			}
		}
	}
	else
	{
		throw IECore::Exception( ( boost::format( "RemoveSmoothSkinningInfluencesOp: \"%d\" is not a recognized mode" ) % mode ).str() );
	}
	
	if ( m_compressionParameter->getTypedValue() )
	{
		CompressSmoothSkinningDataOp compressionOp;
		compressionOp.inputParameter()->setValidatedValue( skinningData );
		compressionOp.copyParameter()->setTypedValue( false );
		compressionOp.operate();
	}
}
