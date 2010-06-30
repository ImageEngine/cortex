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

#include <cassert>

#include "boost/format.hpp"

#include "maya/MFnSkinCluster.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnMatrixData.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MDGModifier.h"
#include "maya/MGlobal.h"
#include "maya/MItGeometry.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MPlugArray.h"
#include "maya/MSelectionList.h"

#include "IECore/SmoothSkinningData.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/MessageHandler.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaSkinClusterConverter.h"

using namespace IECoreMaya;

ToMayaSkinClusterConverter::Description ToMayaSkinClusterConverter::g_skinClusterDescription( IECore::SmoothSkinningData::staticTypeId(), MFn::kSkinClusterFilter );

ToMayaSkinClusterConverter::ToMayaSkinClusterConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::SmoothSkinningData objects to a Maya skinCluster.", object)
{
}

bool ToMayaSkinClusterConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	IECore::ConstSmoothSkinningDataPtr skinningData = IECore::runTimeCast<const IECore::SmoothSkinningData>( from );
	assert( skinningData );
	
	const std::vector<std::string> &influenceNames = skinningData->influenceNames()->readable();
	const std::vector<Imath::M44f> &influencePoseData  = skinningData->influencePose()->readable();
	const std::vector<int> &pointIndexOffsets  = skinningData->pointIndexOffsets()->readable();
	const std::vector<int> &pointInfluenceCounts = skinningData->pointInfluenceCounts()->readable();
	const std::vector<int> &pointInfluenceIndices = skinningData->pointInfluenceIndices()->readable();
	const std::vector<float> &pointInfluenceWeights = skinningData->pointInfluenceWeights()->readable();
	
	MFnDependencyNode fnSkinClusterNode( to, &s );
	MFnSkinCluster fnSkinCluster( to, &s );
	if ( s != MS::kSuccess )
	{
		/// \todo: optional parameter to allow custom node types and checks for the necessary attributes
		/// \todo: create a new skinCluster if we want a kSkinClusterFilter and this isn't one
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: \"%s\" is not a valid skinCluster" ) % fnSkinClusterNode.name() ).str() );
	}

	// gather the influence objects
	/// \todo: optional parameter to keep existing influences
	MObject mObj;
	MDagPath path;
	MSelectionList influenceList;
	MDagPathArray influencePaths;
	for ( unsigned i=0; i < influenceNames.size(); i++ )
	{
		MString influenceName( influenceNames[i].c_str() );
		influenceList.add( influenceName );
		s = influenceList.getDependNode( i, mObj );
		if ( s != MS::kSuccess )
		{
			throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: Influence \"%s\" does not exist" ) % influenceName ).str() );
		}
		MFnDagNode fnInfluence( mObj, &s );
		fnInfluence.getPath( path );
		influencePaths.append( path );
	}
	
	/// \todo: optional parameter to reset the skinCluster's geomMatrix plug
	
	// break existing influence connections to the skinCluster
	MDGModifier dgModifier;
	MPlugArray connectedPlugs;
	MPlug matrixArrayPlug = fnSkinClusterNode.findPlug( "matrix", true, &s );
	for ( unsigned i=0; i < matrixArrayPlug.numConnectedElements(); i++ )
	{
		MPlug matrixPlug = matrixArrayPlug.connectionByPhysicalIndex( i, &s );
		matrixPlug.connectedTo( connectedPlugs, true, false );
		dgModifier.disconnect( connectedPlugs[0], matrixPlug );
	}
	MPlug lockArrayPlug = fnSkinClusterNode.findPlug( "lockWeights", true, &s );
	for ( unsigned i=0; i < lockArrayPlug.numConnectedElements(); i++ )
	{
		MPlug lockPlug = lockArrayPlug.connectionByPhysicalIndex( i, &s );
		lockPlug.connectedTo( connectedPlugs, true, false );
		dgModifier.disconnect( connectedPlugs[0], lockPlug );
	}
	MPlug paintPlug = fnSkinClusterNode.findPlug( "paintTrans", true, &s );
	paintPlug.connectedTo( connectedPlugs, true, false );
	if ( connectedPlugs.length() )
	{
		dgModifier.disconnect( connectedPlugs[0], paintPlug );
	}
	
	// break existing influence connections to the bind pose
	MPlug bindPlug = fnSkinClusterNode.findPlug( "bindPose", true, &s );
	bindPlug.connectedTo( connectedPlugs, true, false );
	MFnDependencyNode fnBindPose( connectedPlugs[0].node() );
	MPlug bindPoseMatrixArrayPlug = fnBindPose.findPlug( "worldMatrix", true, &s );
	for ( unsigned i=0; i < bindPoseMatrixArrayPlug.numConnectedElements(); i++ )
	{
		MPlug matrixPlug = bindPoseMatrixArrayPlug.connectionByPhysicalIndex( i, &s );
		matrixPlug.connectedTo( connectedPlugs, true, false );
		dgModifier.disconnect( connectedPlugs[0], matrixPlug );
	}
	MPlug bindPoseMemberArrayPlug = fnBindPose.findPlug( "members", true, &s );
	for ( unsigned i=0; i < bindPoseMemberArrayPlug.numConnectedElements(); i++ )
	{
		MPlug memberPlug = bindPoseMemberArrayPlug.connectionByPhysicalIndex( i, &s );
		memberPlug.connectedTo( connectedPlugs, true, false );
		dgModifier.disconnect( connectedPlugs[0], memberPlug );
	}
	dgModifier.doIt();
	
	// make connections from influences to skinCluster and bindPose
	for ( unsigned i=0; i < influenceNames.size(); i++ )
	{
		influenceList.getDependNode( i, mObj );
		MFnDependencyNode fnInfluence( mObj );
		MPlug influenceMatrixPlug = fnInfluence.findPlug( "worldMatrix", true, &s ).elementByLogicalIndex( 0, &s );
		MPlug influenceLockPlug = fnInfluence.findPlug( "lockInfluenceWeights", true, &s );
		MPlug influenceMessagePlug = fnInfluence.findPlug( "message", true, &s );
		MPlug influenceBindPosePlug = fnInfluence.findPlug( "bindPose", true, &s );
		
		// connect influence to the skinCluster
		MPlug matrixPlug = matrixArrayPlug.elementByLogicalIndex( i );
		MPlug lockPlug = lockArrayPlug.elementByLogicalIndex( i );
		dgModifier.connect( influenceMatrixPlug, matrixPlug );
		dgModifier.connect( influenceLockPlug, lockPlug );
		
		// connect influence to the bindPose
		MPlug bindPoseMatrixPlug = bindPoseMatrixArrayPlug.elementByLogicalIndex( i );
		MPlug memberPlug = bindPoseMemberArrayPlug.elementByLogicalIndex( i );
		dgModifier.connect( influenceMessagePlug, bindPoseMatrixPlug );
		dgModifier.connect( influenceBindPosePlug, memberPlug );
	}
	influenceList.getDependNode( 0, mObj );
	MFnDependencyNode fnInfluence( mObj );
	MPlug influenceMessagePlug = fnInfluence.findPlug( "message", true, &s );
	dgModifier.connect( influenceMessagePlug, paintPlug );
	dgModifier.doIt();
	
	// use influencePoseData as bindPreMatrix
	MPlug bindPreMatrixArrayPlug = fnSkinClusterNode.findPlug( "bindPreMatrix", true, &s );
	for ( unsigned i=0; i < influencePoseData.size(); i++ )
	{
		MPlug preMatrixPlug = bindPreMatrixArrayPlug.elementByLogicalIndex( i, &s );
		preMatrixPlug.getValue( mObj );
		MFnMatrixData matFn( mObj, &s );
		matFn.set( IECore::convert<MMatrix>( influencePoseData[i] ) );
		mObj = matFn.object();
		preMatrixPlug.setValue( mObj );
	}
	
	// get the geometry
	MObjectArray outputGeoObjs;
	if ( !fnSkinCluster.getOutputGeometry( outputGeoObjs ) )
	{
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: skinCluster \"%s\" does not have any output geometry!" ) % fnSkinCluster.name() ).str() );
	}
	MFnDagNode dagFn( outputGeoObjs[0] );
	MDagPath geoPath;
	dagFn.getPath( geoPath );
		
	// loop through all the points of the geometry and set the weights
	MItGeometry geoIt( outputGeoObjs[0] );
	MPlug weightListArrayPlug = fnSkinClusterNode.findPlug( "weightList", true, &s );
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
			int influenceIndex = fnSkinCluster.indexForInfluenceObject( influencePaths[ pointInfluenceIndices[ firstIndex + i ] ] );
			MPlug influenceWeightPlug = pointWeightsPlug.elementByLogicalIndex( influenceIndex, &s );
			influenceWeightPlug.setValue( pointInfluenceWeights[ firstIndex + i ] );
		}				
	}
	
	return true;
}
