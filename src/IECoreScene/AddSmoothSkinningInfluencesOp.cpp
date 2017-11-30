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

#include "IECoreScene/AddSmoothSkinningInfluencesOp.h"
#include "IECoreScene/ReorderSmoothSkinningInfluencesOp.h"
#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/TypedObjectParameter.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( AddSmoothSkinningInfluencesOp );

AddSmoothSkinningInfluencesOp::AddSmoothSkinningInfluencesOp()
	: ModifyOp(
		"The AddSmoothSkinningInfluencesOp adds zero-weighted influences to the SmoothSkinningData.",
		new SmoothSkinningDataParameter( "result", "The result", new SmoothSkinningData ),
		new SmoothSkinningDataParameter( "input", "The SmoothSkinningData to modify", new SmoothSkinningData )
	)
{
	m_influenceNamesParameter = new StringVectorParameter(
		"influenceNames",
		"Names of the new influences",
		new StringVectorData
	);

	m_influencePoseParameter = new M44fVectorParameter(
		"influencePose",
		"Pose matrices for the new influences",
		new M44fVectorData
	);

	m_indicesParameter = new IntVectorParameter(
		"indices",
		"Per-new-influence indices into the running influence list at the time each new influence is added",
		new IntVectorData
	);

	parameters()->addParameter( m_influenceNamesParameter );
	parameters()->addParameter( m_influencePoseParameter );
	parameters()->addParameter( m_indicesParameter );
}

AddSmoothSkinningInfluencesOp::~AddSmoothSkinningInfluencesOp()
{
}

void AddSmoothSkinningInfluencesOp::modify( Object * object, const CompoundObject * operands )
{
	SmoothSkinningData *skinningData = static_cast<SmoothSkinningData *>( object );
	assert( skinningData );

	const std::vector<std::string> &newNames = m_influenceNamesParameter->getTypedValue();
	const std::vector<Imath::M44f> &newPoseData = m_influencePoseParameter->getTypedValue();
	const std::vector<int> &indices = m_indicesParameter->getTypedValue();

	std::vector<std::string> &influenceNames = skinningData->influenceNames()->writable();
	std::vector<Imath::M44f> &influencePoseData = skinningData->influencePose()->writable();

	// make sure the parameter values are the same length
	if ( newNames.size() != newPoseData.size() )
	{
		throw IECore::Exception( "AddSmoothSkinningInfluencesOp: the influenceNames and influencePose parameters are not the same size" );
	}

	if ( newNames.size() != indices.size() )
	{
		throw IECore::Exception( "AddSmoothSkinningInfluencesOp: the influenceNames and indices parameters are not the same size" );
	}

	// determine the proper order of influences
	std::vector<std::string> finalOrder( influenceNames.begin(), influenceNames.end() );
	for ( unsigned i=0; i < newNames.size(); i++ )
	{
		std::string name = newNames[i];

		if ( find( influenceNames.begin(), influenceNames.end(), name ) != influenceNames.end() )
		{
			throw IECore::Exception( ( boost::format( "AddSmoothSkinningInfluencesOp: \"%s\" is already an influence" ) % name ).str() );
		}

		if ( indices[i] > (int)finalOrder.size() )
		{
			throw IECore::Exception( ( boost::format( "AddSmoothSkinningInfluencesOp: \"%d\" is outside the range of valid indices" ) % indices[i] ).str() );
		}
		else if ( indices[i] == (int)finalOrder.size() )
		{
			finalOrder.push_back( name );
		}
		else
		{
			finalOrder.insert( finalOrder.begin() + indices[i], name );
		}
	}

	// add new influences to the end
	for ( unsigned i=0; i < newNames.size(); i++ )
	{
		influenceNames.push_back( newNames[i] );
		influencePoseData.push_back( newPoseData[i] );
	}

	// reorder the influences
	ReorderSmoothSkinningInfluencesOp reorderOp;
	reorderOp.inputParameter()->setValidatedValue( skinningData );
	reorderOp.copyParameter()->setTypedValue( false );
	reorderOp.parameters()->parameter<StringVectorParameter>( "reorderedInfluenceNames" )->setTypedValue( finalOrder );
	reorderOp.operate();
}
