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

#include "IECore/ReorderSmoothSkinningInfluencesOp.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypedObjectParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ReorderSmoothSkinningInfluencesOp );

ReorderSmoothSkinningInfluencesOp::ReorderSmoothSkinningInfluencesOp()
	: ModifyOp(
		"The ReorderSmoothSkinningInfluencesOp changes the order of the influences in SmoothSkinningData.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_reorderedInfluencesParameter = new StringVectorParameter(
		"reorderedInfluenceNames",
		"The influenceNames in a new order",
		new StringVectorData
	);

	parameters()->addParameter( m_reorderedInfluencesParameter );
}

ReorderSmoothSkinningInfluencesOp::~ReorderSmoothSkinningInfluencesOp()
{
}

void ReorderSmoothSkinningInfluencesOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<std::string> &newOrder = m_reorderedInfluencesParameter->getTypedValue();
	const std::vector<std::string> &originalOrder = skinningData->influenceNames()->readable();
	const std::vector<Imath::M44f> &originalPoseData = skinningData->influencePose()->readable();

	std::vector<int> orderMap( originalOrder.size() );
	std::vector<std::string> finalOrder;
	std::vector<Imath::M44f> finalPoseData;
	std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->writable();

	if ( newOrder.size() != originalOrder.size() )
	{
		throw IECore::Exception( "ReorderSmoothSkinningInfluencesOp: reorderedInfluenceNames and input.influenceNames must contain the same names" );
	}

	// create the mapping between originalOrder and newOrder
	for ( unsigned i=0; i < newOrder.size(); i++ )
	{
		std::string name = newOrder[i];

		const std::vector<std::string>::const_iterator location = find( originalOrder.begin(), originalOrder.end(), name );
		if ( location == originalOrder.end() )
		{
			throw IECore::Exception( ( boost::format( "ReorderSmoothSkinningInfluencesOp: \"%s\" is not an original influenceName" ) % name ).str() );
		}

		unsigned originalIndex = location - originalOrder.begin();
		orderMap[originalIndex] = i;
		finalOrder.push_back( name );
		finalPoseData.push_back( originalPoseData[originalIndex] );
	}

	// update the pointInfluenceIndices
	for ( unsigned i=0; i < pointInfluenceIndices.size(); i++ )
	{
		int newIndex = orderMap[ pointInfluenceIndices[i] ];
		pointInfluenceIndices[i] = newIndex;

	}

	// swap the names and poses
	skinningData->influenceNames()->writable().swap( finalOrder );
	skinningData->influencePose()->writable().swap( finalPoseData );
}
