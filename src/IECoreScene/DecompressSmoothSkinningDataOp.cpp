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

#include "IECoreScene/DecompressSmoothSkinningDataOp.h"
#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/TypedObjectParameter.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( DecompressSmoothSkinningDataOp );

DecompressSmoothSkinningDataOp::DecompressSmoothSkinningDataOp()
	: ModifyOp(
		"The DecompressSmoothSkinningDataOp decompresses SmoothSkinningData by adding 0 value weights for all missing influences",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
}

DecompressSmoothSkinningDataOp::~DecompressSmoothSkinningDataOp()
{
}

void DecompressSmoothSkinningDataOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<std::string> &influenceNames = skinningData->influenceNames()->readable();
	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	const std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->readable();

	std::vector<int> newOffsets;
	std::vector<int> newCounts;
	std::vector<int> newIndices;
	std::vector<float> newWeights;

	int offset = 0;
	int numInfluences = influenceNames.size();

	for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
	{
		const std::vector<int>::const_iterator first = pointInfluenceIndices.begin() + pointIndexOffsets[i];
		const std::vector<int>::const_iterator last = first + pointInfluenceCounts[i];

		for ( int j=0; j < numInfluences; j++ )
		{
			newIndices.push_back( j );

			const std::vector<int>::const_iterator location = find( first, last, j );
			if ( location != last )
			{
				newWeights.push_back( pointInfluenceWeights[ location - pointInfluenceIndices.begin() ] );
			}
			else
			{
				newWeights.push_back( 0.0f );
			}
		}

		newOffsets.push_back( offset );
		newCounts.push_back( numInfluences );
		offset += numInfluences;
	}

	// replace the vectors on the SmoothSkinningData
	if ( newWeights.size() != pointInfluenceWeights.size() )
	{
		skinningData->pointIndexOffsets()->writable().swap( newOffsets );
		skinningData->pointInfluenceCounts()->writable().swap( newCounts );
		skinningData->pointInfluenceIndices()->writable().swap( newIndices );
		skinningData->pointInfluenceWeights()->writable().swap( newWeights );
	}
}
