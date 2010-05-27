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

#include "boost/format.hpp"

#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/VectorTypedData.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/MeshPrimitiveSmoothSkinningOp.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/PrimitiveEvaluator.h"
#include "IECore/VectorOps.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitiveSmoothSkinningOp );

MeshPrimitiveSmoothSkinningOp::MeshPrimitiveSmoothSkinningOp() : MeshPrimitiveOp( "A MeshPrimitiveOp to deform a mesh based on a pose and SmoothSkinningData" )
{

	IntParameter::PresetsContainer deformNormalsPresets;
	deformNormalsPresets.push_back( IntParameter::Preset( "Off", Off ) );
	deformNormalsPresets.push_back( IntParameter::Preset( "On", On ) );

	m_deformNormalsParameter = new IntParameter(
	        "deformNormals",
	        "Deform the normals of the mesh or just the points.",
	        Off,
	        Off,
	        On,
	        deformNormalsPresets,
	        true
	);

	IntParameter::PresetsContainer blendPresets;
	blendPresets.push_back( IntParameter::Preset( "Linear", Linear ) );

	m_blendParameter = new IntParameter(
	        "blend",
	        "Blending algorithm used to deform the mesh.",
	        Linear,
	        Linear,
	        Linear,
	        blendPresets,
	        true
	);


	m_smoothSkinningDataParameter = new SmoothSkinningDataParameter(
		"smoothSkinningData",
		"Set the SmoothSkinningData to be used in the deformation",
		new SmoothSkinningData()
	);

	m_deformationPoseParameter = new M44fVectorParameter(
		"deformationPose",
		"Set the deformationPose (a M44fVectorData object) to be used in the deformation",
		new M44fVectorData()
	);

	parameters()->addParameter( m_deformationPoseParameter );
	parameters()->addParameter( m_smoothSkinningDataParameter );
	parameters()->addParameter( m_deformNormalsParameter );
	parameters()->addParameter( m_blendParameter );
}

MeshPrimitiveSmoothSkinningOp::~MeshPrimitiveSmoothSkinningOp()
{
}

M44fVectorParameter * MeshPrimitiveSmoothSkinningOp::deformationPoseParameter()
{
	return m_deformationPoseParameter;
}

const M44fVectorParameter * MeshPrimitiveSmoothSkinningOp::deformationPoseParameter() const
{
	return m_deformationPoseParameter;
}

SmoothSkinningDataParameter * MeshPrimitiveSmoothSkinningOp::smoothSkinningDataParameter()
{
	return m_smoothSkinningDataParameter;
}

const SmoothSkinningDataParameter * MeshPrimitiveSmoothSkinningOp::smoothSkinningDataParameter() const
{
	return m_smoothSkinningDataParameter;
}

IntParameter * MeshPrimitiveSmoothSkinningOp::deformNormalsParameter()
{
	return m_deformNormalsParameter;
}

const IntParameter * MeshPrimitiveSmoothSkinningOp::deformNormalsParameter() const
{
	return m_deformNormalsParameter;
}

IntParameter * MeshPrimitiveSmoothSkinningOp::blendParameter()
{
	return m_blendParameter;
}

const IntParameter * MeshPrimitiveSmoothSkinningOp::blendParameter() const
{
	return m_blendParameter;
}


void MeshPrimitiveSmoothSkinningOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	assert( mesh );
	assert( operands );

	// get the switches
	DeformNormals deformNormals = static_cast<DeformNormals>( m_deformNormalsParameter->getNumericValue() );
	Blend blend = static_cast<Blend>( m_blendParameter->getNumericValue() );

	// check if the mesh has valid data for P
	int m_numVerts = mesh->variableSize( PrimitiveVariable::Vertex );

	PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
	if (it == mesh->variables.end())
	{
		throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" in MeshPrimitiveSmoothSkinningOp" );
	}
	const DataPtr &verticesData = it->second.data;

	// check if there are normals in the source mesh
	if (deformNormals == On)
	{
		PrimitiveVariableMap::const_iterator it = mesh->variables.find( "N" );
		if (it == mesh->variables.end())
		{
			throw InvalidArgumentException("MeshPrimitiveSmoothSkinningOp: MeshPrimitive has no primitive variable \"N\"" );
		}
		// get the primvar
		const PrimitiveVariable &nPrimVar = it->second;
		// todo: actually deform the normals if this is requested!

	}

	if (! mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to MeshPrimitiveSmoothSkinningOp" );
	}

	// get the smooth skinning data and check its validity and suitability for the mesh
	SmoothSkinningDataPtr smoothSkinningData = smoothSkinningDataParameter()->getTypedValue< SmoothSkinningData >( );

	if ( !smoothSkinningData )
	{
		return;
	}

	smoothSkinningData->validate();

	int ssdPointCount = smoothSkinningData->pointInfluenceCounts()->readable().size();
	if ( m_numVerts != ssdPointCount )
	{
		throw InvalidArgumentException( "Number of points in SmoothSkinningData does not match vertex count on mesh given to MeshPrimitiveSmoothSkinningOp" );
	}

	// get the deformation pose and check its compatibility with the SmoothSkinningData
	M44fVectorDataPtr deformationPose;

	deformationPose = runTimeCast<M44fVectorData>(deformationPoseParameter()->getValue( ));
	unsigned int influencesCount = smoothSkinningData->influencePose()->readable().size();

	if (deformationPose->readable().size() != influencesCount)
	{
		throw InvalidArgumentException( "Number of elements in SmoothSkinningData.influencePose does not match number of elements in deformationPose given to MeshPrimitiveSmoothSkinningOp" );
	}

	// generate skinning matrices
	// we are pre-creating these as in the typical use-case the number of influence objects is much lower
	// than the number of vertices that are going to be deformed
	M44fVectorDataPtr skinMat = new M44fVectorData();

	skinMat->writable().reserve( influencesCount );

	std::vector<M44f>::const_iterator ip_it = smoothSkinningData->influencePose()->readable().begin();

	for( std::vector<M44f>::const_iterator dp_it = deformationPose->readable().begin();
		 dp_it!=deformationPose->readable().end(); ++dp_it )
	{

		M44f mat = (*ip_it) *  (*dp_it);
		skinMat->writable().push_back( mat );
		++ip_it;
	}


	// iterate through all the vertices in the source mesh and deform using the weighted skinning matrices

	if ( blend == Linear )
	{
		// iterators

		/// \todo Use depatchTypedData
		if (runTimeCast<V3fVectorData>(verticesData))
		{
			V3fVectorDataPtr p = runTimeCast<V3fVectorData>(verticesData);

			for ( V3fVectorData::ValueType::iterator vert = p->writable().begin();
				vert != p->writable().end(); ++vert)
			{
				V3f &currP = *vert;
				V3f newP(0,0,0);

				int vertIndex = vert - p->readable().begin();
				int pointInfluenceCount = smoothSkinningData->pointInfluenceCounts()->readable()[vertIndex];
				int pointIndexOffset = smoothSkinningData->pointIndexOffsets()->readable()[vertIndex];

				for (int pointInfluenceId = pointIndexOffset; pointInfluenceId < (pointIndexOffset+pointInfluenceCount);
						pointInfluenceId++)
				{
					int influenceId = smoothSkinningData->pointInfluenceIndices()->readable()[pointInfluenceId];
					float weight = smoothSkinningData->pointInfluenceWeights()->readable()[pointInfluenceId];
					newP += currP * skinMat->readable()[influenceId] * weight;
				}

				currP = newP;
			}
		}
		else if (runTimeCast<V3dVectorData>(verticesData))
		{
			V3dVectorDataPtr p = runTimeCast<V3dVectorData>(verticesData);

			for ( V3dVectorData::ValueType::iterator vert = p->writable().begin();
				vert != p->writable().end(); ++vert)
			{
				V3d &currP=  *vert;
				V3d newP(0,0,0);

				int vertIndex = vert - p->readable().begin();
				int pointInfluenceCount = smoothSkinningData->pointInfluenceCounts()->readable()[vertIndex];
				int pointIndexOffset = smoothSkinningData->pointIndexOffsets()->readable()[vertIndex];

				for (int pointInfluenceId = pointIndexOffset; pointInfluenceId < (pointIndexOffset+pointInfluenceCount);
						pointInfluenceId++)
				{
					int influenceId = smoothSkinningData->pointInfluenceIndices()->readable()[pointInfluenceId];
					float weight = smoothSkinningData->pointInfluenceWeights()->readable()[pointInfluenceId];
					newP += currP * skinMat->readable()[influenceId] * weight;
				}

				currP = newP;
			}

		}
		else
		{
			throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" of type V3fVectorData/V3dVectorData in MeshPrimitiveSmoothSkinningOp");
		}

	}
	else
	{
		// this should never happen
		assert(0);
	}

}
