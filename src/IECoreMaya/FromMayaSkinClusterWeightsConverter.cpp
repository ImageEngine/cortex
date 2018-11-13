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

#include "IECoreMaya/FromMayaSkinClusterWeightsConverter.h"
#include "IECoreMaya/Convert.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/Exception.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/VectorTypedData.h"

#include "maya/MFnSkinCluster.h"
#include "maya/MFnDagNode.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPath.h"
#include "maya/MObjectArray.h"
#include "maya/MItGeometry.h"

using namespace IECoreMaya;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( FromMayaSkinClusterWeightsConverter );

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaSkinClusterWeightsConverter> FromMayaSkinClusterWeightsConverter::m_description( MFn::kSkinClusterFilter, IECore::CompoundObject::staticTypeId(), false );

FromMayaSkinClusterWeightsConverter::FromMayaSkinClusterWeightsConverter( const MObject &object )
	:	FromMayaObjectConverter( "Converts weights from skinCluster nodes to a CompoundObject", object )
{
	m_useCompression = new IECore::BoolParameter(
		"useCompression",
		"Compress weights as Shorts",
		true
	);
	m_compressionThreshold = new IECore::FloatParameter(
		"compressionThreshold",
		"Weights below that threshold are being ignored.",
		0.0
	);

	parameters()->addParameter( m_useCompression );
	parameters()->addParameter( m_compressionThreshold );
}

IECore::ObjectPtr FromMayaSkinClusterWeightsConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus status;

	// our data storage objects
	IECore::FloatVectorDataPtr pointInfluenceWeightsData = new IECore::FloatVectorData();
	IECore::IntVectorDataPtr pointInfluenceIndicesData = new IECore::IntVectorData();
	IECore::IntVectorDataPtr pointIndexOffsetsData  = new IECore::IntVectorData();
	IECore::IntVectorDataPtr pointInfluenceCountsData = new IECore::IntVectorData();

	auto &pointInfluenceWeightsW = pointInfluenceWeightsData->writable();
	auto &pointInfluenceIndicesW = pointInfluenceIndicesData->writable();
	auto &pointIndexOffsetsW = pointIndexOffsetsData->writable();
	auto &pointInfluenceCountsW = pointInfluenceCountsData->writable();

	float compressionThreshold = m_compressionThreshold->getNumericValue();

	// // get a skin cluster fn
	MFnSkinCluster skinClusterFn(object);

	// get the first input geometry to the skin cluster
	// TODO: if needed, extend this to retrieve more than one output geometry
	MObjectArray outputGeoObjs;
	status = skinClusterFn.getOutputGeometry( outputGeoObjs );
	if ( !status )
	{
		throw IECore::Exception( "FromMayaSkinClusterWeightsConverter: skinCluster node does not have any output geometry!" );
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
		MObject pointObj = geoIt.currentItem( &status );
		MDoubleArray weights;
		unsigned int weightsCount;

		skinClusterFn.getWeights( geoPath, pointObj, weights, weightsCount );
		int pointInfluencesCount = 0;

		for ( int influenceId = 0; influenceId < int( weightsCount ); influenceId++ )
		{
			// ignore zero weights, we are generating a compressed (non-sparse) representation of the weights
			if ( weights[influenceId] > compressionThreshold )
			{
				pointInfluencesCount++;
				pointInfluenceWeightsW.push_back( float( weights[influenceId] ) );
				pointInfluenceIndicesW.push_back( influenceId );
			}
		}

		pointIndexOffsetsW.push_back( currentOffset );
		pointInfluenceCountsW.push_back( pointInfluencesCount );
		currentOffset += pointInfluencesCount;
	}


	IECore::CompoundObjectPtr outDataPtr = new IECore::CompoundObject();

	outDataPtr->members()["pointInfluenceIndices"] = pointInfluenceIndicesData;
	outDataPtr->members()["pointIndexOffsets"] = pointIndexOffsetsData;
	outDataPtr->members()["pointInfluenceCounts"] = pointInfluenceCountsData;

	bool useCompression = m_useCompression->getTypedValue();
	if ( useCompression )
	{
		IECore::DataConvert< IECore::FloatVectorData, IECore::UShortVectorData, IECore::ScaledDataConversion< float, unsigned short > >converter;
		IECore::UShortVectorDataPtr pointInfluenceShortWeightsData = converter( pointInfluenceWeightsData );
		outDataPtr->members()["pointInfluenceWeights"] = pointInfluenceShortWeightsData;
	}
	else
	{
		outDataPtr->members()["pointInfluenceWeights"] = pointInfluenceWeightsData;
	}

	return outDataPtr;
}
