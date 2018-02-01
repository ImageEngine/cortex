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

#include "IECoreScene/NormalizeSmoothSkinningWeightsOp.h"

#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/TypedObjectParameter.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

#include <algorithm>
#include <cassert>

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( NormalizeSmoothSkinningWeightsOp );

NormalizeSmoothSkinningWeightsOp::NormalizeSmoothSkinningWeightsOp()
	: ModifyOp(
		"The NormalizeSmoothSkinningWeightsOp normalizes SmoothSkinningData weights between the existing influences for each point",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_useLocksParameter = new BoolParameter(
		"applyLocks",
		"Whether or not influenceLocks should be applied",
		false
	);

	m_influenceLocksParameter = new BoolVectorParameter(
		"influenceLocks",
		"A per-influence list of lock values",
		new BoolVectorData
	);

	parameters()->addParameter( m_useLocksParameter );
	parameters()->addParameter( m_influenceLocksParameter );
}

NormalizeSmoothSkinningWeightsOp::~NormalizeSmoothSkinningWeightsOp()
{
}

void NormalizeSmoothSkinningWeightsOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();

	std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->writable();

	bool useLocks = m_useLocksParameter->getTypedValue();
	std::vector<bool> &locks = m_influenceLocksParameter->getTypedValue();
	std::vector<int> unlockedIndices;

	// make sure there is one lock per influence
	if ( useLocks && ( locks.size() != skinningData->influenceNames()->readable().size() ) )
	{
		throw IECore::Exception( "NormalizeSmoothSkinningWeightsOp: There must be exactly one lock per influence" );
	}

	if ( !useLocks )
	{
		locks.clear();
		locks.resize( skinningData->influenceNames()->readable().size(), false );
	}

	for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
	{
		unlockedIndices.clear();
		float totalLockedWeights = 0.0f;
		float totalUnlockedWeights = 0.0f;

		for ( int j=0; j < pointInfluenceCounts[i]; j++ )
		{
			int current = pointIndexOffsets[i] + j;

			if ( locks[ pointInfluenceIndices[current] ] )
			{
				totalLockedWeights += pointInfluenceWeights[current];
			}
			else
			{
				totalUnlockedWeights += pointInfluenceWeights[current];
				unlockedIndices.push_back( current );
			}
		}

		float remainingWeight = 1.0f - totalLockedWeights;

		if ( (remainingWeight == 0.0f) || (totalUnlockedWeights == 0.0f) )
		{
			for ( unsigned j=0; j < unlockedIndices.size(); j++ )
			{
				pointInfluenceWeights[ unlockedIndices[j] ] = 0.0f;
			}
		}
		else
		{
			for ( unsigned j=0; j < unlockedIndices.size(); j++ )
			{
				pointInfluenceWeights[ unlockedIndices[j] ] = (pointInfluenceWeights[ unlockedIndices[j] ] * remainingWeight) / totalUnlockedWeights;
			}
		}
	}
}
