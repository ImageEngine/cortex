//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "IECoreMaya/FromMayaSkinClusterConverter.h"
#include "IECoreMaya/Convert.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"
#include "IECoreScene/SmoothSkinningData.h"

#include "maya/MFnSkinCluster.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnMatrixData.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MObjectArray.h"
#include "maya/MItGeometry.h"
#include "maya/MPlug.h"

using namespace IECoreMaya;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( FromMayaSkinClusterConverter );

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaSkinClusterConverter> FromMayaSkinClusterConverter::m_description( MFn::kSkinClusterFilter, IECoreScene::SmoothSkinningData::staticTypeId(), true );

FromMayaSkinClusterConverter::FromMayaSkinClusterConverter( const MObject &object )
	:	FromMayaObjectConverter( "Converts data on skinCluster nodes.into SmoothSkinningData", object )
{
	IECore::IntParameter::PresetsContainer influenceNamePresets;
	influenceNamePresets.push_back( IECore::IntParameter::Preset( "Partial", Partial ) );
	influenceNamePresets.push_back( IECore::IntParameter::Preset( "Full", Full ) );

	m_influenceNameParameter = new IECore::IntParameter(
		"influenceName",
		"Will the influence names contain the partial or full dag path.",
		Partial,
		Partial,
		Full,
		influenceNamePresets,
		true
	);

	parameters()->addParameter( m_influenceNameParameter );
}

IECore::IntParameterPtr FromMayaSkinClusterConverter::influenceNameParameter()
{
	return m_influenceNameParameter;
}

IECore::ConstIntParameterPtr FromMayaSkinClusterConverter::influenceNameParameter() const
{
	return m_influenceNameParameter;
}

IECore::ObjectPtr FromMayaSkinClusterConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus stat;

	// our data storage objects
	IECore::StringVectorDataPtr influenceNamesData = new IECore::StringVectorData();
	IECore::M44fVectorDataPtr influencePoseData  = new IECore::M44fVectorData();
	IECore::IntVectorDataPtr pointIndexOffsetsData  = new IECore::IntVectorData();
	IECore::IntVectorDataPtr pointInfluenceCountsData = new IECore::IntVectorData();
	IECore::IntVectorDataPtr pointInfluenceIndicesData = new IECore::IntVectorData();
	IECore::FloatVectorDataPtr pointInfluenceWeightsData = new IECore::FloatVectorData();

	// get a skin cluster fn
	MFnSkinCluster skinClusterFn(object);

	MDagPathArray influencePaths;
	skinClusterFn.influenceObjects(influencePaths);

	// get the influence names
	int influencesCount = influencePaths.length();
	influenceNamesData->writable().reserve( influencesCount );

	InfluenceName in = (InfluenceName)m_influenceNameParameter->getNumericValue();
	switch( in )
	{
		case Partial :
		{
			for (int i=0; i < influencesCount; i++)
			{
				influenceNamesData->writable().push_back( influencePaths[i].partialPathName(&stat).asChar() );
			}
			break;
		}
		case Full :
		{
			for (int i=0; i < influencesCount; i++)
			{
				influenceNamesData->writable().push_back( influencePaths[i].fullPathName(&stat).asChar() );
			}
			break;
		}
	}

	// extract bind pose
	MFnDependencyNode skinClusterNodeFn( object );

	MPlug bindPreMatrixArrayPlug = skinClusterNodeFn.findPlug( "bindPreMatrix", true, &stat );

	for (int i=0; i < influencesCount; i++)
	{
		MPlug bindPreMatrixElementPlug = bindPreMatrixArrayPlug.elementByLogicalIndex(
				skinClusterFn.indexForInfluenceObject( influencePaths[i], NULL ), &stat);
		MObject matObj;
		bindPreMatrixElementPlug.getValue( matObj );
		MFnMatrixData matFn( matObj, &stat );
		MMatrix mat = matFn.matrix();
		Imath::M44f cmat = IECore::convert<Imath::M44f>( mat );

		influencePoseData->writable().push_back( cmat );
	}

	// extract the skinning information

	// get the first input geometry to the skin cluster
	// TODO: if needed, extend this to retrieve more than one output geometry
	MObjectArray outputGeoObjs;
	stat = skinClusterFn.getOutputGeometry( outputGeoObjs );

	if (! stat)
	{
		throw IECore::Exception( "FromMayaSkinClusterConverter: skinCluster node does not have any output geometry!" );
	}

	// get the dag path to the first object
	MFnDagNode dagFn( outputGeoObjs[0] );
	MDagPath geoPath;
	dagFn.getPath( geoPath );

	// generate a geo iterator for the components
	MItGeometry geoIt( outputGeoObjs[0] );
	int currentOffset = 0;

	// loop through all the points of the geometry to extract their bind information
	for ( ; !geoIt.isDone(); geoIt.next() )
	{
		MObject pointObj = geoIt.currentItem( &stat );
		MDoubleArray weights;
		unsigned int weightsCount;

		skinClusterFn.getWeights( geoPath, pointObj, weights, weightsCount );
		int pointInfluencesCount = 0;

		for ( int influenceId = 0; influenceId < int( weightsCount ); influenceId++ )
		{
			// ignore zero weights, we are generating a compressed (non-sparse) representation of the weights
			/// \todo: use a parameter to specify a threshold value rather than 0.0
			if ( weights[influenceId] != 0.0 )
			{
				pointInfluencesCount++;
				pointInfluenceWeightsData->writable().push_back( float( weights[influenceId] ) );
				pointInfluenceIndicesData->writable().push_back( influenceId );

			}
		}

		pointIndexOffsetsData->writable().push_back( currentOffset );
		pointInfluenceCountsData->writable().push_back( pointInfluencesCount );
		currentOffset += pointInfluencesCount;
	}

	// put all our results in a smooth skinning data object
	return new IECoreScene::SmoothSkinningData( influenceNamesData, influencePoseData, pointIndexOffsetsData,
										pointInfluenceCountsData, pointInfluenceIndicesData, pointInfluenceWeightsData  );
}



