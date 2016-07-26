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

#include <cassert>

#include "boost/format.hpp"

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

#include "IECore/CompoundParameter.h"
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
	m_ignoreMissingInfluencesParameter = new IECore::BoolParameter(
		"ignoreMissingInfluences",
		"If True, ignores SmoothSkinningData influences that aren't in the Maya scene and prunes the weights of Maya skinCluster influences that aren't in the SmoothSkinningData",
		false
	);
	
	m_ignoreBindPoseParameter = new IECore::BoolParameter(
		"ignoreBindPose",
		"If True, does not make connections to the bindPose node",
		false
	);
	
	parameters()->addParameter( m_ignoreMissingInfluencesParameter );
	parameters()->addParameter( m_ignoreBindPoseParameter );
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

	const unsigned origNumInfluences = influenceNames.size();
	unsigned numInfluences = origNumInfluences;
	std::vector<bool> ignoreInfluence( origNumInfluences, false );
	std::vector<int> indexMap( origNumInfluences, -1 );
	const bool ignoreMissingInfluences = m_ignoreMissingInfluencesParameter->getTypedValue();
	const bool ignoreBindPose = m_ignoreBindPoseParameter->getTypedValue();
	
	// gather the influence objects
	MObject mObj;
	MDagPath path;
	MSelectionList influenceList;
	MDagPathArray influencePaths;
	for ( unsigned i=0, index=0; i < origNumInfluences; i++ )
	{
		MString influenceName( influenceNames[i].c_str() );
		s = influenceList.add( influenceName );
		if ( !s )
		{
			if ( ignoreMissingInfluences )
			{
				ignoreInfluence[i] = true;
				MGlobal::displayWarning( MString( "ToMayaSkinClusterConverter: \"" + influenceName + "\" is not a valid influence" ) );
				continue;
			}
			
			throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: \"%s\" is not a valid influence" ) % influenceName ).str() );
		}
		
		influenceList.getDependNode( index, mObj );
		MFnIkJoint fnInfluence( mObj, &s );
		if ( !s )
		{
			if ( ignoreMissingInfluences )
			{
				ignoreInfluence[i] = true;
				influenceList.remove( index );
				MGlobal::displayWarning( MString( "ToMayaSkinClusterConverter: \"" + influenceName + "\" is not a valid influence" ) );
				continue;
			}
			
			throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: \"%s\" is not a valid influence" ) % influenceName ).str() );
		}
		
		fnInfluence.getPath( path );
		influencePaths.append( path );
		indexMap[i] = index;
		index++;
	}
	
	MPlugArray connectedPlugs;
	
	bool existingBindPose = true;
	MPlug bindPlug = fnSkinClusterNode.findPlug( "bindPose", true, &s );
	if ( !bindPlug.connectedTo( connectedPlugs, true, false ) )
	{
		existingBindPose = false;
		if ( !ignoreBindPose )
		{
			throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: \"%s\" does not have a valid bindPose" ) % fnSkinClusterNode.name() ).str() );
		}
	}
	
	MPlug bindPoseMatrixArrayPlug;
	MPlug bindPoseMemberArrayPlug;
	if ( existingBindPose )
	{
		MFnDependencyNode fnBindPose( connectedPlugs[0].node() );
		if ( fnBindPose.typeName() != "dagPose" )
		{
			throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: \"%s\" is not a valid bindPose" ) % fnBindPose.name() ).str() );
		}
		
		bindPoseMatrixArrayPlug = fnBindPose.findPlug( "worldMatrix", true, &s );
		bindPoseMemberArrayPlug = fnBindPose.findPlug( "members", true, &s );
	}
	
	/// \todo: optional parameter to reset the skinCluster's geomMatrix plug
	
	// break existing influence connections to the skinCluster
	MDGModifier dgModifier;
	MMatrixArray ignoredPreMatrices;
	MPlug matrixArrayPlug = fnSkinClusterNode.findPlug( "matrix", true, &s );
	MPlug bindPreMatrixArrayPlug = fnSkinClusterNode.findPlug( "bindPreMatrix", true, &s );
	for ( unsigned i=0; i < matrixArrayPlug.numConnectedElements(); i++ )
	{
		MPlug matrixPlug = matrixArrayPlug.connectionByPhysicalIndex( i, &s );
		matrixPlug.connectedTo( connectedPlugs, true, false );
		if ( !connectedPlugs.length() )
		{
			continue;
		}
		
		MFnIkJoint fnInfluence( connectedPlugs[0].node() );
		fnInfluence.getPath( path );
		if ( ignoreMissingInfluences && !influenceList.hasItem( path ) )
		{
			MPlug preMatrixPlug = bindPreMatrixArrayPlug.elementByLogicalIndex( i );
			preMatrixPlug.getValue( mObj );
			MFnMatrixData matFn( mObj );
			ignoredPreMatrices.append( matFn.matrix() );
			ignoreInfluence.push_back( false );
			indexMap.push_back( influenceList.length() );
			influenceList.add( connectedPlugs[0].node() );
			numInfluences++;
		}
		dgModifier.disconnect( connectedPlugs[0], matrixPlug );
	}
	MPlug lockArrayPlug = fnSkinClusterNode.findPlug( "lockWeights", true, &s );
	for ( unsigned i=0; i < lockArrayPlug.numConnectedElements(); i++ )
	{
		MPlug lockPlug = lockArrayPlug.connectionByPhysicalIndex( i, &s );
		lockPlug.connectedTo( connectedPlugs, true, false );
		if ( connectedPlugs.length() )
		{
			dgModifier.disconnect( connectedPlugs[0], lockPlug );
		}
	}
	MPlug paintPlug = fnSkinClusterNode.findPlug( "paintTrans", true, &s );
	paintPlug.connectedTo( connectedPlugs, true, false );
	if ( connectedPlugs.length() )
	{
		dgModifier.disconnect( connectedPlugs[0], paintPlug );
	}
	
	// break existing influence connections to the bind pose
	if ( existingBindPose )
	{
		for ( unsigned i=0; i < bindPoseMatrixArrayPlug.numConnectedElements(); i++ )
		{
			MPlug matrixPlug = bindPoseMatrixArrayPlug.connectionByPhysicalIndex( i, &s );
			matrixPlug.connectedTo( connectedPlugs, true, false );
			if ( connectedPlugs.length() )
			{
				dgModifier.disconnect( connectedPlugs[0], matrixPlug );
			}
		}
		for ( unsigned i=0; i < bindPoseMemberArrayPlug.numConnectedElements(); i++ )
		{
			MPlug memberPlug = bindPoseMemberArrayPlug.connectionByPhysicalIndex( i, &s );
			memberPlug.connectedTo( connectedPlugs, true, false );
			if ( connectedPlugs.length() )
			{
				dgModifier.disconnect( connectedPlugs[0], memberPlug );
			}
		}
	}
	
	if ( !dgModifier.doIt() )
	{
		dgModifier.undoIt();
		throw IECore::Exception( "ToMayaSkinClusterConverter: Unable to break the influence connections" );
	}
	
	// make connections from influences to skinCluster and bindPose
	for ( unsigned i=0; i < numInfluences; i++ )
	{
		if ( ignoreInfluence[i] )
		{
			continue;
		}
		
		int index = indexMap[i];
		s = influenceList.getDependNode( index, mObj );
		MFnIkJoint fnInfluence( mObj, &s );
		MPlug influenceMatrixPlug = fnInfluence.findPlug( "worldMatrix", true, &s ).elementByLogicalIndex( 0, &s );
		MPlug influenceMessagePlug = fnInfluence.findPlug( "message", true, &s );
		MPlug influenceBindPosePlug = fnInfluence.findPlug( "bindPose", true, &s );
		MPlug influenceLockPlug = fnInfluence.findPlug( "lockInfluenceWeights", true, &s );
		if ( !s )
		{
			// add the lockInfluenceWeights attribute if it doesn't exist
			MFnNumericAttribute nAttr;
			MObject attribute = nAttr.create( "lockInfluenceWeights", "liw", MFnNumericData::kBoolean, false );
			fnInfluence.addAttribute( attribute );
			influenceLockPlug = fnInfluence.findPlug( "lockInfluenceWeights", true, &s );
		}
		
		// connect influence to the skinCluster
		MPlug matrixPlug = matrixArrayPlug.elementByLogicalIndex( index );
		MPlug lockPlug = lockArrayPlug.elementByLogicalIndex( index );
		dgModifier.connect( influenceMatrixPlug, matrixPlug );
		dgModifier.connect( influenceLockPlug, lockPlug );
		
		// connect influence to the bindPose
		if ( !ignoreBindPose )
		{
			MPlug bindPoseMatrixPlug = bindPoseMatrixArrayPlug.elementByLogicalIndex( index );
			MPlug memberPlug = bindPoseMemberArrayPlug.elementByLogicalIndex( index );
			dgModifier.connect( influenceMessagePlug, bindPoseMatrixPlug );
			dgModifier.connect( influenceBindPosePlug, memberPlug );
		}
	}
	unsigned firstIndex = find( ignoreInfluence.begin(), ignoreInfluence.end(), false ) - ignoreInfluence.begin();
	influenceList.getDependNode( firstIndex, mObj );
	MFnDependencyNode fnInfluence( mObj );
	MPlug influenceMessagePlug = fnInfluence.findPlug( "message", true, &s );
	dgModifier.connect( influenceMessagePlug, paintPlug );
	if ( !dgModifier.doIt() )
	{
		dgModifier.undoIt();
		throw IECore::Exception( "ToMayaSkinClusterConverter: Unable to create the influence connections" );
	}
	
	// use influencePoseData as bindPreMatrix
	for ( unsigned i=0; i < numInfluences; i++ )
	{
		if ( ignoreInfluence[i] )
		{
			continue;
		}
		
		MMatrix preMatrix = ( i < origNumInfluences ) ? IECore::convert<MMatrix>( influencePoseData[i] ) : ignoredPreMatrices[i-origNumInfluences];
		MPlug preMatrixPlug = bindPreMatrixArrayPlug.elementByLogicalIndex( indexMap[i], &s );
		s = preMatrixPlug.getValue( mObj );
		if ( s )
		{
			MFnMatrixData matFn( mObj );
			matFn.set( preMatrix );
			mObj = matFn.object();
		}
		else
		{
			MFnMatrixData matFn;
			mObj = matFn.create( preMatrix );
		}
		
		preMatrixPlug.setValue( mObj );
	}
	
	// remove unneeded bindPreMatrix children
	unsigned existingElements = bindPreMatrixArrayPlug.numElements();
	for ( unsigned i=influenceList.length(); i < existingElements; i++ )
	{
		MPlug preMatrixPlug = bindPreMatrixArrayPlug.elementByLogicalIndex( i, &s );
		/// \todo: surely there is a way to accomplish this in c++...
		MGlobal::executeCommand( ( boost::format( "removeMultiInstance %s" ) % preMatrixPlug.name() ).str().c_str() );
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

	size_t pointCount = geoIt.exactCount();
	if( pointCount != pointIndexOffsets.size() )
	{
		throw IECore::Exception( ( boost::format( "ToMayaSkinClusterConverter: topology of skinCluster \"%s\"'s output geometry has changed!" ) % fnSkinCluster.name() ).str() );
	}

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
			int influenceIndex = pointInfluenceIndices[ firstIndex + i ];
			if ( ignoreInfluence[ influenceIndex ] )
			{
				continue;
			}
			
			int skinClusterInfluenceIndex = fnSkinCluster.indexForInfluenceObject( influencePaths[ indexMap[ influenceIndex ] ] );
			MPlug influenceWeightPlug = pointWeightsPlug.elementByLogicalIndex( skinClusterInfluenceIndex, &s );
			influenceWeightPlug.setValue( pointInfluenceWeights[ firstIndex + i ] );
		}
	}
	
	return true;
}
