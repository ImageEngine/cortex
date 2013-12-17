//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/TransferSmoothSkinningWeightsOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/CompressSmoothSkinningDataOp.h"
#include "IECore/DecompressSmoothSkinningDataOp.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( TransferSmoothSkinningWeightsOp );

TransferSmoothSkinningWeightsOp::TransferSmoothSkinningWeightsOp()
	: ModifyOp(
		"The TransferSmoothSkinningWeightsOp transfers all source influence weights onto a target.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_targetInfluenceNameParameter = new StringParameter(
		"targetInfluenceName",
		"The target influence name",
		new StringData
	);
	
	m_sourceInfluenceNamesParameter = new StringVectorParameter(
		"sourceInfluenceNames",
		"The source influence names",
		new StringVectorData
	);
	parameters()->addParameter( m_targetInfluenceNameParameter );
	parameters()->addParameter( m_sourceInfluenceNamesParameter );
}

TransferSmoothSkinningWeightsOp::~TransferSmoothSkinningWeightsOp()
{
}

void TransferSmoothSkinningWeightsOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );
	
	const std::string target = m_targetInfluenceNameParameter->getTypedValue();
	const std::vector<std::string> &sources = m_sourceInfluenceNamesParameter->getTypedValue();
	if ( !sources.size() )
	{
		throw IECore::Exception( "TransferSmoothSkinningWeightsOp: you need to specify source influences" );
	}
	
	const std::vector<std::string> &influenceNames = skinningData->influenceNames()->readable();
	
	const std::vector<std::string>::const_iterator foundSame = find( sources.begin(), sources.end(), target );
	if ( foundSame != sources.end() )
	{
		throw IECore::Exception( ( boost::format( "TransferSmoothSkinningWeightsOp: \"%s\" cannot be both source and target" ) % target ).str() );
	}
	
	const std::vector<std::string>::const_iterator location = find( influenceNames.begin(), influenceNames.end(), target );
	if ( location == influenceNames.end() )
	{
		throw IECore::Exception( ( boost::format( "TransferSmoothSkinningWeightsOp: \"%s\" is not a valid influence name" ) % target ).str() );
	}
	int targetIndex = location - influenceNames.begin();
	std::vector<int> sourceIndices;
	
	for ( unsigned i=0; i < sources.size(); i++ )
	{
		std::string name = sources[i];
		
		const std::vector<std::string>::const_iterator found = find( influenceNames.begin(), influenceNames.end(), name );
		if ( found == influenceNames.end() )
		{
			throw IECore::Exception( ( boost::format( "TransferSmoothSkinningWeightsOp: \"%s\" is not a valid influenceName" ) % name ).str() );
		}
		sourceIndices.push_back( found - influenceNames.begin() );
	}

	// decompress skinning data
	DecompressSmoothSkinningDataOpPtr decompressionOp = new DecompressSmoothSkinningDataOp;
	decompressionOp->inputParameter()->setValidatedValue( skinningData );
	decompressionOp->copyParameter()->setTypedValue( false );
	decompressionOp->operate();
	
	const std::vector<int> &pointIndexOffsets = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->writable();
	
	for ( unsigned i=0; i < pointIndexOffsets.size(); i++ )
	{
		float targetWeight = 0.0;
		int targetCurrentIndex = 0;
		
		for ( int j=0; j < pointInfluenceCounts[i]; j++ )
		{
			int current = pointIndexOffsets[i] + j;
			int index = pointInfluenceIndices[current];
			float weight = pointInfluenceWeights[current];
			
			if( index == targetIndex )
			{
				targetWeight += weight;
				targetCurrentIndex = current;
			}
			else
			{
				const std::vector<int>::const_iterator found = find( sourceIndices.begin(), sourceIndices.end(), index );
				if ( found != sourceIndices.end() )
				{
					targetWeight += weight;
					pointInfluenceWeights[ current ] = 0.0;
				}
			}
		}
		pointInfluenceWeights[ targetCurrentIndex ] = targetWeight;
	}
	
	// re-compress
	CompressSmoothSkinningDataOpPtr compressionOp = new CompressSmoothSkinningDataOp;
	compressionOp->inputParameter()->setValidatedValue( skinningData );
	compressionOp->copyParameter()->setTypedValue( false );
	compressionOp->operate();
}
