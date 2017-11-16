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

#include "tbb/tbb.h"

#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/VectorTypedData.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/VectorOps.h"
#include "IECore/DespatchTypedData.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointSmoothSkinningOp.h"
#include "IECoreScene/SmoothSkinningData.h"
#include "IECoreScene/PrimitiveEvaluator.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointSmoothSkinningOp );

PointSmoothSkinningOp::PointSmoothSkinningOp() :
	        ModifyOp(
	                "Deforms points and normals based on a pose and SmoothSkinningData.",
	                new PrimitiveParameter(
	                        "result",
	                        "The updated Primitive with deformed points and normals.",
	                        new PointsPrimitive()
	                        ),
	                new PrimitiveParameter(
	                        "input",
	                        "The input Primitive with points and normals to deform.",
	                        new PointsPrimitive()
	                        )
	                )
{
    m_positionVarParameter = new StringParameter(
            "positionVar",
            "The variable name to use as per-point position.",
            "P" );
    parameters()->addParameter( m_positionVarParameter );


    m_normalVarParameter = new StringParameter(
            "normalVar",
            "The variable name to use as per-point normal.",
            "N" );
    parameters()->addParameter( m_normalVarParameter );

	m_deformNormalsParameter = new BoolParameter(
	        "deformNormals",
	        "Deform the normals of the mesh or just the points.",
	        false
	);
	parameters()->addParameter( m_deformNormalsParameter );

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
	parameters()->addParameter( m_blendParameter );

	m_smoothSkinningDataParameter = new SmoothSkinningDataParameter(
		"smoothSkinningData",
		"Set the SmoothSkinningData to be used in the deformation",
		new SmoothSkinningData()
	);
	parameters()->addParameter( m_smoothSkinningDataParameter );

	m_deformationPoseParameter = new M44fVectorParameter(
		"deformationPose",
		"Set the deformationPose (a M44fVectorData object) to be used in the deformation",
		new M44fVectorData()
	);
	parameters()->addParameter( m_deformationPoseParameter );

	m_refIndicesParameter = new IntVectorParameter(
		"referenceIndices",
		"Set the reference indices to be used for querying the smooth skinning data in the deformation.",
		new IntVectorData()
	);
	parameters()->addParameter( m_refIndicesParameter );

}

PointSmoothSkinningOp::~PointSmoothSkinningOp()
{
}

StringParameter * PointSmoothSkinningOp::positionVarParameter()
{
        return m_positionVarParameter.get();
}

const StringParameter * PointSmoothSkinningOp::positionVarParameter() const
{
        return m_positionVarParameter.get();
}

StringParameter * PointSmoothSkinningOp::normalVarParameter()
{
        return m_normalVarParameter.get();
}

const StringParameter * PointSmoothSkinningOp::normalVarParameter() const
{
        return m_normalVarParameter.get();
}

M44fVectorParameter * PointSmoothSkinningOp::deformationPoseParameter()
{
	return m_deformationPoseParameter.get();
}

const M44fVectorParameter * PointSmoothSkinningOp::deformationPoseParameter() const
{
	return m_deformationPoseParameter.get();
}

SmoothSkinningDataParameter * PointSmoothSkinningOp::smoothSkinningDataParameter()
{
	return m_smoothSkinningDataParameter.get();
}

const SmoothSkinningDataParameter * PointSmoothSkinningOp::smoothSkinningDataParameter() const
{
	return m_smoothSkinningDataParameter.get();
}

BoolParameter * PointSmoothSkinningOp::deformNormalsParameter()
{
	return m_deformNormalsParameter.get();
}

const BoolParameter * PointSmoothSkinningOp::deformNormalsParameter() const
{
	return m_deformNormalsParameter.get();
}

IntParameter * PointSmoothSkinningOp::blendParameter()
{
	return m_blendParameter.get();
}

const IntParameter * PointSmoothSkinningOp::blendParameter() const
{
	return m_blendParameter.get();
}

IntVectorParameter * PointSmoothSkinningOp::refIndicesParameter()
{
	return m_refIndicesParameter.get();
}

const IntVectorParameter * PointSmoothSkinningOp::refIndicesParameter() const
{
	return m_refIndicesParameter.get();
}

struct PointSmoothSkinningOp::DeformPositions
{
	public :

		DeformPositions( std::vector<V3f> &p_data, const std::vector<int> &pointIndexOffsets, const std::vector<int> &pointInfluenceCounts, const std::vector<int> &pointInfluenceIndices, const std::vector<float> &pointInfluenceWeights, const std::vector<M44f> &skin_data, const std::vector<int> &refId_data )
			:	m_pData( p_data ), m_pointIndexOffsets( pointIndexOffsets ), m_pointInfluenceCounts( pointInfluenceCounts ), m_pointInfluenceIndices( pointInfluenceIndices ), m_pointInfluenceWeights( pointInfluenceWeights ), m_skinData(skin_data), m_refIdData(refId_data)
		{
		}

		void operator()( const tbb::blocked_range<size_t> &r ) const
		{
			for( size_t p_it=r.begin(); p_it!=r.end(); ++p_it )
			{
				V3f p_new(0,0,0);

				V3f &p_value = m_pData[p_it];
				int p_id;
				if( m_refIdData.size() )
				{
					// get the actual index to look up in the smooth skinning data
					p_id = m_refIdData[p_it];
				}
				else
				{
					p_id = p_it;
				}

				int p_influence_count = m_pointInfluenceCounts[p_id];
				int p_index_offset = m_pointIndexOffsets[p_id];

				for (int p_influence_id = p_index_offset; p_influence_id < (p_index_offset+p_influence_count);
						p_influence_id++)
				{
					int influence_id = m_pointInfluenceIndices[p_influence_id];
					float weight = m_pointInfluenceWeights[p_influence_id];
					p_new += p_value * m_skinData[influence_id] * weight;
				}
				p_value = p_new;
			}
		}

	private :

		std::vector<V3f> &m_pData;
		const std::vector<int> &m_pointIndexOffsets;
		const std::vector<int> &m_pointInfluenceCounts;
		const std::vector<int> &m_pointInfluenceIndices;
		const std::vector<float> &m_pointInfluenceWeights;
		const std::vector<M44f> &m_skinData;
		const std::vector<int> &m_refIdData;

};

struct PointSmoothSkinningOp::DeformNormals
{
	public :

		DeformNormals( std::vector<V3f> &n_data, const std::vector<int> &pointIndexOffsets, const std::vector<int> &pointInfluenceCounts, const std::vector<int> &pointInfluenceIndices, const std::vector<float> &pointInfluenceWeights, const std::vector<M44f> &skin_data, const std::vector<int> &refId_data, std::vector<int> &vertexIndicesData )
			:	m_nData( n_data ), m_pointIndexOffsets( pointIndexOffsets ), m_pointInfluenceCounts( pointInfluenceCounts ), m_pointInfluenceIndices( pointInfluenceIndices ), m_pointInfluenceWeights( pointInfluenceWeights ), m_skinData(skin_data), m_refIdData(refId_data), m_vertexIndicesData( vertexIndicesData )
		{
		}

		void operator()( const tbb::blocked_range<size_t> &r ) const
		{
			for( size_t n_it=r.begin(); n_it!=r.end(); ++n_it )
			{
				V3f n_new(0,0,0);

				V3f &n_value = m_nData[n_it];
				V3f n_unw;

				int n_id = n_it;

				if( m_vertexIndicesData.size() )
				{
					n_id = m_vertexIndicesData[n_id];
				}
				if( m_refIdData.size() )
				{
					n_id = m_refIdData[n_id];
				}

				int n_influence_count = m_pointInfluenceCounts[n_id];
				int n_index_offset = m_pointIndexOffsets[n_id];

				for (int n_influence_id = n_index_offset; n_influence_id < (n_index_offset+n_influence_count);
						n_influence_id++)
				{
					int influence_id = m_pointInfluenceIndices[n_influence_id];
					float weight = m_pointInfluenceWeights[n_influence_id];

					m_skinData[influence_id].multDirMatrix(n_value,n_unw);
					n_new += n_unw * weight;
				}
				n_value = n_new;
			}
		}

	private :

		std::vector<V3f> &m_nData;
		const std::vector<int> &m_pointIndexOffsets;
		const std::vector<int> &m_pointInfluenceCounts;
		const std::vector<int> &m_pointInfluenceIndices;
		const std::vector<float> &m_pointInfluenceWeights;
		const std::vector<M44f> &m_skinData;
		const std::vector<int> &m_refIdData;
		const std::vector<int> &m_vertexIndicesData;

};

void PointSmoothSkinningOp::modify( Object *input, const CompoundObject *operands )
{
	// get the input parameters
    Primitive *pt = static_cast<Primitive *>( input );

    bool deform_n = operands->member<BoolData>( "deformNormals" )->readable();
	Blend blend = static_cast<Blend>( m_blendParameter->getNumericValue() );
    string position_var = operands->member<StringData>( "positionVar" )->readable();
    string normal_var = operands->member<StringData>( "normalVar" )->readable();
    SmoothSkinningDataPtr ssd = smoothSkinningDataParameter()->getTypedValue< SmoothSkinningData >( );
	M44fVectorDataPtr def = runTimeCast<M44fVectorData>(deformationPoseParameter()->getValue( ));
	const std::vector<int> &refId_data = operands->member<IntVectorData>( "referenceIndices" )->readable();

	// verify position and normal data
    if ( pt->variables.count(position_var)==0 )
    {
    	throw Exception( "Could not find position variable on primitive!" );
    }

    V3fVectorData *p = pt->variableData<V3fVectorData>(position_var);
    if ( !p )
    {
    	throw Exception("Could not get position data from primitive!");
    }

    std::vector<V3f> &p_data =  p->writable();
    int p_size = p_data.size();

	// Check reference id data. If provided, it must be the same size as P.
	int refId_size = refId_data.size();
	if( refId_size && refId_size != p_size )
	{
		throw InvalidArgumentException( "Number of reference indices does not match point count on Primitive given to PointSmoothSkinningOp" );
	}

    // verify the SmoothSkinningData
	if ( !ssd )
	{
		return;
	}

	int ssd_p_size = ssd->pointInfluenceCounts()->readable().size();

	if ( !refId_size && ssd_p_size != p_size )
	{
		throw InvalidArgumentException( "Number of points in SmoothSkinningData does not match point count on Primitive given to PointSmoothSkinningOp" );
	}

	// get the deformation pose and check its compatibility with the SmoothSkinningData
	const std::vector<M44f> &def_data = def->readable();
	int def_size = def_data.size();

	int inf_size = ssd->influencePose()->readable().size();

	if (def_size != inf_size)
	{
		throw InvalidArgumentException( "Number of elements in SmoothSkinningData.influencePose does not match number of elements in deformationPose given to PointSmoothSkinningOp" );
	}

	// check if the smooth skinning data has changed since the last time the op was used;
	// validating the ssd can be expensive and unnecessary for the case that the ssd is not changing
	// so we are storing an internal copy of the ssd as a comparison is much faster than a complete validation
	if ( ssd != m_prevSmoothSkinningData )
	{
		ssd->validate();
		m_prevSmoothSkinningData = ssd;
	}

	// test n data
	if ( deform_n )
	{
		PrimitiveVariableMap::const_iterator it = pt->variables.find(normal_var);
		if ( it != pt->variables.end() )
		{
			if( !pt->isPrimitiveVariableValid( it->second )  )
			{
				throw Exception("Normal variable on primitive is invalid!");
			}
			if (it->second.interpolation == PrimitiveVariable::FaceVarying )
			{
				MeshPrimitive *mesh = dynamic_cast<MeshPrimitive *>( pt );
				if( !mesh )
				{
					V3fVectorData *n = pt->variableData<V3fVectorData>(normal_var);
					int n_size = n->readable().size();
					if ( p_size != n_size )
					{
						throw Exception("Position and normal variables must be the same length!");
					}
				}
			}
		}
		else
		{
			throw Exception( "Could not find normal variable on primitive!" );
		}
	}

	// generate skinning matrices
	// we are pre-creating these as in the typical use-case the number of influence objects is much lower
	// than the number of vertices that are going to be deformed
    std::vector<M44f> skin_data;
	skin_data.reserve( inf_size );

	std::vector<M44f>::const_iterator ip_it = ssd->influencePose()->readable().begin();

	for( std::vector<M44f>::const_iterator dp_it = def_data.begin();
		 dp_it!=def_data.end(); ++dp_it )
	{

		M44f mat = (*ip_it) *  (*dp_it);
		skin_data.push_back( mat );
		++ip_it;
	}


	// iterate through all the points in the source primitive and deform using the weighted skinning matrices
	if ( blend == Linear )
	{
		const std::vector<int> &pointIndexOffsets = ssd->pointIndexOffsets()->readable();
		const std::vector<int> &pointInfluenceCounts = ssd->pointInfluenceCounts()->readable();
		const std::vector<int> &pointInfluenceIndices = ssd->pointInfluenceIndices()->readable();
		const std::vector<float> &pointInfluenceWeights = ssd->pointInfluenceWeights()->readable();

		// deform our P
		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, p_size ),
			DeformPositions( p_data, pointIndexOffsets, pointInfluenceCounts, pointInfluenceIndices, pointInfluenceWeights, skin_data, refId_data )
		);

		// deform our N
		if ( deform_n )
		{
			PrimitiveVariableMap::const_iterator it = pt->variables.find(normal_var);
			if ( it != pt->variables.end() )
			{
				V3fVectorData *n = pt->variableData<V3fVectorData>(normal_var);
				std::vector<V3f> &n_data =  n->writable();

				std::vector<int> vertexIndicesData;
				if (it->second.interpolation == PrimitiveVariable::FaceVarying )
				{
					MeshPrimitive *mesh = dynamic_cast<MeshPrimitive *>( pt );
					if( mesh )
					{
						vertexIndicesData = mesh->vertexIds()->readable();
					}
				}

				tbb::parallel_for(
					tbb::blocked_range<size_t>( 0, n_data.size() ),
					DeformNormals( n_data, pointIndexOffsets, pointInfluenceCounts, pointInfluenceIndices, pointInfluenceWeights, skin_data, refId_data, vertexIndicesData )
				);
			}
        }
	}
	else
	{
		// this should never happen
		assert(0);
	}

}
