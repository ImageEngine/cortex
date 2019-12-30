//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaSkinClusterWeightsConverter.h"

#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/CompoundData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/PrimitiveVariable.h"

#include "maya/MFnSkinCluster.h"
#include "maya/MFnIkJoint.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnMatrixData.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MDGModifier.h"
#include "maya/MGlobal.h"
#include "maya/MItGeometry.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MMatrixArray.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MPlugArray.h"
#include "maya/MSelectionList.h"

#include "boost/format.hpp"

#include <cassert>

using namespace IECoreMaya;

ToMayaSkinClusterWeightsConverter::Description ToMayaSkinClusterWeightsConverter::g_skinClusterDescription( IECore::CompoundObject::staticTypeId(), MFn::kSkinClusterFilter );

ToMayaSkinClusterWeightsConverter::ToMayaSkinClusterWeightsConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts a skinCluster weights CompoundObject to Maya skinCluster weights.", object)
{
}

bool ToMayaSkinClusterWeightsConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	IECore::ConstCompoundObjectPtr weightDataPtr = IECore::runTimeCast<const IECore::CompoundObject>( from );
	assert( weightDataPtr );

	IECore::ConstIntVectorDataPtr pointInfluenceIndicesData = weightDataPtr->member<IECore::IntVectorData>("pointInfluenceIndices", true );
	IECore::ConstIntVectorDataPtr pointIndexOffsetsData = weightDataPtr->member<IECore::IntVectorData>("pointIndexOffsets",  true );
	IECore::ConstIntVectorDataPtr pointInfluenceCountsData = weightDataPtr->member<IECore::IntVectorData>("pointInfluenceCounts", true );
	IECore::ConstFloatVectorDataPtr pointInfluenceWeightsData = new IECore::FloatVectorData();

	if ( weightDataPtr->member<IECore::FloatVectorData>("pointInfluenceWeights") )
	{
		pointInfluenceWeightsData = weightDataPtr->member<IECore::FloatVectorData>("pointInfluenceWeights");
	}
	else if ( weightDataPtr->member<IECore::UShortVectorData>("pointInfluenceWeights") )
	{
		IECore::ConstUShortVectorDataPtr weightsShortData = weightDataPtr->member<IECore::UShortVectorData>("pointInfluenceWeights" );
		IECore::DataConvert< IECore::UShortVectorData, IECore::FloatVectorData, IECore::ScaledDataConversion< unsigned short, float > >converter;
		pointInfluenceWeightsData = converter( weightsShortData );
	}

	const auto &pointInfluenceWeights = pointInfluenceWeightsData->readable();
	const auto &pointInfluenceIndices = pointInfluenceIndicesData->readable();
	const auto &pointIndexOffsets = pointIndexOffsetsData->readable();
	const auto &pointInfluenceCounts = pointInfluenceCountsData->readable();

	MFnDependencyNode fnSkinClusterNode( to, &s );
	MFnSkinCluster fnSkinCluster( to, &s );
	if ( s != MS::kSuccess )
	{
		/// \todo: optional parameter to allow custom node types and checks for the necessary attributes
		/// \todo: create a new skinCluster if we want a kSkinClusterFilter and this isn't one
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterWeightsConverter: \"%s\" is not a valid skinCluster" ) % fnSkinClusterNode.name() ).str() );
	}


	// get the geometry
	MObjectArray outputGeoObjs;
	if ( !fnSkinCluster.getOutputGeometry( outputGeoObjs ) )
	{
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterWeightsConverter: skinCluster \"%s\" does not have any output geometry!" ) % fnSkinCluster.name() ).str() );
	}

	// loop through all the points of the geometry and set the weights
	MItGeometry geoIt( outputGeoObjs[0] );

	size_t pointCount = geoIt.exactCount();
	if( pointCount != pointIndexOffsets.size() )
	{
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterWeightsConverter: topology of skinCluster \"%s\"'s output geometry has changed!" ) % fnSkinCluster.name() ).str() );
	}

	MPlug weightListArrayPlug = fnSkinClusterNode.findPlug( "weightList", false, &s );
	for ( unsigned pIndex=0; !geoIt.isDone(); geoIt.next(), pIndex++ )
	{
		MPlug pointWeightsPlug = weightListArrayPlug.elementByLogicalIndex( pIndex, &s ).child( 0 );

		// remove existing influence weight plugs
		MIntArray existingInfluenceIndices;
		pointWeightsPlug.getExistingArrayAttributeIndices( existingInfluenceIndices );
		for( unsigned i=0; i < existingInfluenceIndices.length(); i++ )
		{
			MPlug influenceWeightPlug = pointWeightsPlug.elementByLogicalIndex( existingInfluenceIndices[i], &s );
			MGlobal::executeCommand( ( boost::format( "removeMultiInstance -break 1 %s" ) % influenceWeightPlug.name() ).str().c_str() );
		}

		// add new influence weight plugs
		int firstIndex = pointIndexOffsets[pIndex];
		for( int i=0; i < pointInfluenceCounts[pIndex]; i++ )
		{
			int influenceIndex = pointInfluenceIndices[ firstIndex + i ];

			// int skinClusterInfluenceIndex = fnSkinCluster.indexForInfluenceObject( influencePaths[ indexMap[ influenceIndex ] ] );
			MPlug influenceWeightPlug = pointWeightsPlug.elementByLogicalIndex( influenceIndex, &s );
			influenceWeightPlug.setValue( pointInfluenceWeights[ firstIndex + i ] );
		}
	}

	return true;
}
