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

#include "IECore/MixSmoothSkinningWeightsOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompressSmoothSkinningDataOp.h"
#include "IECore/DecompressSmoothSkinningDataOp.h"
#include "IECore/Interpolator.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( MixSmoothSkinningWeightsOp );

MixSmoothSkinningWeightsOp::MixSmoothSkinningWeightsOp()
	: ModifyOp(
		"The MixSmoothSkinningWeightsOp mixes two sets of SmoothSkinningData using FloatVectorData to interpolate between the weights.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_skinningDataParameter = new SmoothSkinningDataParameter(
		"skinningDataToMix",
		"The SmoothSkinningData to mix with the input SmoothSkinningData",
		new SmoothSkinningData
	);

	m_mixingWeightsParameter = new FloatVectorParameter(
		"mixingWeights",
		"The per-influence weights for mixing input with skinningDataToMix",
		new FloatVectorData
	);

	parameters()->addParameter( m_skinningDataParameter );
	parameters()->addParameter( m_mixingWeightsParameter );
}

MixSmoothSkinningWeightsOp::~MixSmoothSkinningWeightsOp()
{
}

void MixSmoothSkinningWeightsOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	SmoothSkinningData *origMixingData = runTimeCast<SmoothSkinningData>( m_skinningDataParameter->getValidatedValue() );
	if ( !origMixingData )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix is not valid" );
	}

	assert( origMixingData );

	// make sure the number of influences matches
	if ( origMixingData->influenceNames()->readable().size() != skinningData->influenceNames()->readable().size() )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix and input have different numbers of influences" );
	}

	std::vector<float> &mixingWeights = m_mixingWeightsParameter->getTypedValue();

	// make sure there is one mixing weight per influence
	if ( mixingWeights.size() != skinningData->influenceNames()->readable().size() )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: There must be exactly one mixing weight per influence" );
	}

	// decompress both sets of skinning data
	DecompressSmoothSkinningDataOp decompressionOp;
	decompressionOp.inputParameter()->setValidatedValue( skinningData );
	decompressionOp.copyParameter()->setTypedValue( false );
	decompressionOp.operate();
	decompressionOp.inputParameter()->setValidatedValue( origMixingData );
	decompressionOp.copyParameter()->setTypedValue( true );
	decompressionOp.operate();
	const SmoothSkinningData *mixingData = runTimeCast<const SmoothSkinningData>( decompressionOp.resultParameter()->getValidatedValue() );
	if ( !mixingData )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix did not decompress correctly" );
	}

	// make sure everything matches except the weights
	if ( *(mixingData->pointIndexOffsets()) != *(skinningData->pointIndexOffsets()) )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix and input have different pointIndexOffsets when decompressed" );
	}
	if ( *(mixingData->pointInfluenceCounts()) != *(skinningData->pointInfluenceCounts()) )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix and input have different pointInfluenceCounts when decompressed" );
	}
	if ( *(mixingData->pointInfluenceIndices()) != *(skinningData->pointInfluenceIndices()) )
	{
		throw IECore::Exception( "MixSmoothSkinningWeightsOp: skinningDataToMix and input have different pointInfluenceIndices when decompressed" );
	}

	const std::vector<int> &inputIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &inputInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &inputInfluenceIndices = skinningData->pointInfluenceIndices()->readable();

	std::vector<float> &inputInfluenceWeights = skinningData->pointInfluenceWeights()->writable();
	const std::vector<float> &mixingInfluenceWeights = mixingData->pointInfluenceWeights()->readable();

	LinearInterpolator<float> lerp;

	// mix the weights
	for ( unsigned i=0; i < inputIndexOffsets.size(); i++ )
	{
		for ( int j=0; j < inputInfluenceCounts[i]; j++ )
		{
			int current = inputIndexOffsets[i] + j;

			lerp( mixingInfluenceWeights[current], inputInfluenceWeights[current], mixingWeights[ inputInfluenceIndices[current] ], inputInfluenceWeights[current] );
		}
	}

	// re-compress the input data
	CompressSmoothSkinningDataOp compressionOp;
	compressionOp.inputParameter()->setValidatedValue( skinningData );
	compressionOp.copyParameter()->setTypedValue( false );
	compressionOp.operate();
}
