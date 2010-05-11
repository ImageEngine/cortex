//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECore/SmoothSkinningData.h"
#include "IECore/CompoundParameter.h"

#include "maya/MFnSkinCluster.h"
#include "maya/MFnDagNode.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MObjectArray.h"
#include "maya/MItGeometry.h"

using namespace IECoreMaya;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( FromMayaSkinClusterConverter );

static const MFn::Type fromTypes[] = { MFn::kSkinClusterFilter, MFn::kInvalid };
static const IECore::TypeId toTypes[] = { IECore::SmoothSkinningData::staticTypeId(), IECore::InvalidTypeId };

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaSkinClusterConverter> FromMayaSkinClusterConverter::m_description( fromTypes, toTypes );

FromMayaSkinClusterConverter::FromMayaSkinClusterConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaSkinClusterConverter", "Converts skinCluster nodes.into SmoothSkinningData", object )
{
	IECore::IntParameter::PresetsContainer spacePresets;
	spacePresets.push_back( IECore::IntParameter::Preset( "Local", Local ) );
	spacePresets.push_back( IECore::IntParameter::Preset( "World", World ) );
	m_spaceParameter = new IECore::IntParameter(
		"space",
		"The space in which the influencePose matrix is converted.",
		World,
		Local,
		World,
		spacePresets,
		true
	);

	parameters()->addParameter( m_spaceParameter );

	IECore::IntParameter::PresetsContainer influenceNamePresets;
	spacePresets.push_back( IECore::IntParameter::Preset( "Partial", Partial ) );
	spacePresets.push_back( IECore::IntParameter::Preset( "Full", Full ) );
	m_spaceParameter = new IECore::IntParameter(
		"influenceName",
		"Will the influence names contain the partial or full dag path.",
		Partial,
		Full,
		Partial,
		influenceNamePresets,
		true
	);

	parameters()->addParameter( m_influenceNameParameter );
}

IECore::IntParameterPtr FromMayaSkinClusterConverter::spaceParameter()
{
	return m_spaceParameter;
}

IECore::ConstIntParameterPtr FromMayaSkinClusterConverter::spaceParameter() const
{
	return m_spaceParameter;
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
	IECore::M44fVectorDataPtr influencePosesData  = new IECore::M44fVectorData();
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

	if( m_influenceNameParameter->getNumericValue()==Full )
	{
		for (int i=0; i < influencesCount; i++)
		{
			influenceNamesData->writable().push_back( influencePaths[i].fullPathName(&stat).asChar() );
		}
	}
	else
	{
		for (int i=0; i < influencesCount; i++)
		{
			influenceNamesData->writable().push_back( influencePaths[i].partialPathName(&stat).asChar() );
		}
	}

	// get the influence (bind) pose from the influence objects
	influencePosesData->writable().reserve( influencesCount );

	for (int i=0; i < influencesCount; i++)
	{
		// TODO
	}

	// extract the skinning information
	//

	// get the first input geometry to the skin cluster
	// TODO: if needed, extend this to retrieve more than one input geometry
	MObjectArray inputGeoObjs;
	stat = skinClusterFn.getInputGeometry( inputGeoObjs );

	if (! stat)
	{
		throw IECore::Exception( "FromMayaSkinClusterConverter: skinCluster node does not have any incoming geometry!" );
	}

    // get the dag path to the first object
	MFnDagNode dagFn( inputGeoObjs[0] );
	MDagPath geoPath;
	dagFn.getPath( geoPath );

	// generate a geo iterator for the components
	MItGeometry geoIt( inputGeoObjs[0] );
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
            if ( weights[influenceId] != 0.0 )
            {
            	pointInfluencesCount++;
            	pointInfluenceWeightsData->writable().push_back( float( weights[influenceId] ) );
            	pointInfluenceIndicesData->writable().push_back( influenceId );
            }
        }

    	pointIndexOffsetsData->writable().push_back( currentOffset );
    	pointInfluenceCountsData->writable().push_back( int( pointInfluencesCount ) );
    	currentOffset += pointInfluencesCount;
    }

	// put all our results in a smooth skinning data object
	return new IECore::SmoothSkinningData( influenceNamesData, influencePosesData, pointIndexOffsetsData,
										pointInfluenceCountsData, pointInfluenceIndicesData, pointInfluenceWeightsData  );
}



