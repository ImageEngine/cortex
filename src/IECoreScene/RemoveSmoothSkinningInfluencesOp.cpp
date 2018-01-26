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

#include "IECoreScene/RemoveSmoothSkinningInfluencesOp.h"

#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/TypedObjectParameter.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

#include <algorithm>
#include <cassert>

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( RemoveSmoothSkinningInfluencesOp );

RemoveSmoothSkinningInfluencesOp::RemoveSmoothSkinningInfluencesOp()
	: ModifyOp(
		"The RemoveSmoothSkinningInfluencesOp removes influences from the SmoothSkinningData, regardless of existing weights.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	IntParameter::PresetsContainer modePresets;
	modePresets.push_back( IntParameter::Preset( "Named", RemoveSmoothSkinningInfluencesOp::Named ) );
	modePresets.push_back( IntParameter::Preset( "Indexed", RemoveSmoothSkinningInfluencesOp::Indexed ) );
	modePresets.push_back( IntParameter::Preset( "Weightless", RemoveSmoothSkinningInfluencesOp::Weightless ) );

	m_modeParameter = new IntParameter(
		"mode",
		"The mode of influence removal. Options are to remove by name, index, or to remove influences with no weights",
		RemoveSmoothSkinningInfluencesOp::Named,
		RemoveSmoothSkinningInfluencesOp::Named,
		RemoveSmoothSkinningInfluencesOp::Weightless,
		modePresets,
		true
	);

	m_influenceNamesParameter = new StringVectorParameter(
		"influenceNames",
		"Names of the influences to remove. This parameter is only used in Named mode.",
		new StringVectorData
	);

	m_indicesParameter = new IntVectorParameter(
		"indices",
		"Indices of the influences to remove. This parameter is only used in Indexed mode.",
		new IntVectorData
	);

	parameters()->addParameter( m_modeParameter );
	parameters()->addParameter( m_influenceNamesParameter );
	parameters()->addParameter( m_indicesParameter );
}

RemoveSmoothSkinningInfluencesOp::~RemoveSmoothSkinningInfluencesOp()
{
}

void RemoveSmoothSkinningInfluencesOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<std::string> &influenceNames = skinningData->influenceNames()->readable();
	const std::vector<Imath::M44f> &influencePoseData = skinningData->influencePose()->readable();
	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	const std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->readable();

	// gather the influence indices
	std::vector<int> indicesToRemove;
	const unsigned numInfluences = influenceNames.size();
	const int mode = m_modeParameter->getNumericValue();
	if ( mode == RemoveSmoothSkinningInfluencesOp::Named )
	{
		const std::vector<std::string> &removeNames = m_influenceNamesParameter->getTypedValue();
		for ( unsigned i=0; i < removeNames.size(); i++ )
		{
			std::string name = removeNames[i];
			const std::vector<std::string>::const_iterator location = find( influenceNames.begin(), influenceNames.end(), name );
			if ( location == influenceNames.end() )
			{
				throw IECore::Exception( ( boost::format( "RemoveSmoothSkinningInfluencesOp: \"%d\" is not a valid influence name" ) % name ).str() );
			}

			indicesToRemove.push_back( location - influenceNames.begin() );
		}
	}
	else if ( mode == RemoveSmoothSkinningInfluencesOp::Indexed )
	{
		indicesToRemove = m_indicesParameter->getTypedValue();
		for ( unsigned i=0; i < indicesToRemove.size(); i++ )
		{
			if ( indicesToRemove[i] > (int)numInfluences - 1 )
			{
				throw IECore::Exception( ( boost::format( "RemoveSmoothSkinningInfluencesOp: \"%d\" is not a valid index" ) % indicesToRemove[i] ).str() );
			}
		}
	}
	else if ( mode == RemoveSmoothSkinningInfluencesOp::Weightless )
	{
		std::set<int> indicesToKeep;
		for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
		{
			for ( int j=0; j < pointInfluenceCounts[i]; j++ )
			{
				int current = pointIndexOffsets[i] + j;

				if ( pointInfluenceWeights[current] > 0.0f )
				{
					indicesToKeep.insert( pointInfluenceIndices[current] );
				}
			}

			if ( indicesToKeep.size() == numInfluences )
			{
				break;
			}
		}

		for ( unsigned i=0; i < numInfluences; i++ )
		{
			if ( indicesToKeep.find( i ) == indicesToKeep.end() )
			{
				indicesToRemove.push_back( i );
			}
		}
	}
	else
	{
		throw IECore::Exception( ( boost::format( "RemoveSmoothSkinningInfluencesOp: \"%d\" is not a recognized mode" ) % mode ).str() );
	}

	std::vector<int> indexMap;
	std::vector<std::string> keepNames;
	std::vector<Imath::M44f> keepPoseData;
	std::vector<int> newOffsets;
	std::vector<int> newCounts;
	std::vector<int> newIndices;
	std::vector<float> newWeights;

	// calculate the map between old and new influence indices
	for ( int i=0; i < (int)numInfluences; i++ )
	{
		if ( find( indicesToRemove.begin(), indicesToRemove.end(), i ) == indicesToRemove.end() )
		{
			indexMap.push_back( keepNames.size() );
			keepNames.push_back( influenceNames[i] );
			keepPoseData.push_back( influencePoseData[i] );
		}
		else
		{
			indexMap.push_back( -1 );
		}
	}

	// adjust the data vectors
	int offset = 0;
	for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
	{
		int count = 0;

		for ( int j=0; j < pointInfluenceCounts[i]; j++ )
		{
			int current = pointIndexOffsets[i] + j;

			if ( indexMap[ pointInfluenceIndices[current] ] != -1 )
			{
				newIndices.push_back( indexMap[ pointInfluenceIndices[current] ] );
				newWeights.push_back( pointInfluenceWeights[current] );
				count++;
			}
		}

		newOffsets.push_back( offset );
		newCounts.push_back( count );
		offset += count;
	}

	// replace the vectors on the SmoothSkinningData
	skinningData->influenceNames()->writable().swap( keepNames );
	skinningData->influencePose()->writable().swap( keepPoseData );
	skinningData->pointIndexOffsets()->writable().swap( newOffsets );
	skinningData->pointInfluenceCounts()->writable().swap( newCounts );
	skinningData->pointInfluenceIndices()->writable().swap( newIndices );
	skinningData->pointInfluenceWeights()->writable().swap( newWeights );
}
