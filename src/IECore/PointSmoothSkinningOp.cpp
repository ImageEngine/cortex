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

#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/VectorTypedData.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/PointSmoothSkinningOp.h"
#include "IECore/SmoothSkinningData.h"
#include "IECore/PrimitiveEvaluator.h"
#include "IECore/VectorOps.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
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

}

PointSmoothSkinningOp::~PointSmoothSkinningOp()
{
}

StringParameter * PointSmoothSkinningOp::positionVarParameter()
{
        return m_positionVarParameter;
}

const StringParameter * PointSmoothSkinningOp::positionVarParameter() const
{
        return m_positionVarParameter;
}

StringParameter * PointSmoothSkinningOp::normalVarParameter()
{
        return m_normalVarParameter;
}

const StringParameter * PointSmoothSkinningOp::normalVarParameter() const
{
        return m_normalVarParameter;
}

M44fVectorParameter * PointSmoothSkinningOp::deformationPoseParameter()
{
	return m_deformationPoseParameter;
}

const M44fVectorParameter * PointSmoothSkinningOp::deformationPoseParameter() const
{
	return m_deformationPoseParameter;
}

SmoothSkinningDataParameter * PointSmoothSkinningOp::smoothSkinningDataParameter()
{
	return m_smoothSkinningDataParameter;
}

const SmoothSkinningDataParameter * PointSmoothSkinningOp::smoothSkinningDataParameter() const
{
	return m_smoothSkinningDataParameter;
}

BoolParameter * PointSmoothSkinningOp::deformNormalsParameter()
{
	return m_deformNormalsParameter;
}

const BoolParameter * PointSmoothSkinningOp::deformNormalsParameter() const
{
	return m_deformNormalsParameter;
}

IntParameter * PointSmoothSkinningOp::blendParameter()
{
	return m_blendParameter;
}

const IntParameter * PointSmoothSkinningOp::blendParameter() const
{
	return m_blendParameter;
}


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


    // verify the SmoothSkinningData
	if ( !ssd )
	{
		return;
	}

	int ssd_p_size = ssd->pointInfluenceCounts()->readable().size();

	if ( ssd_p_size != p_size )
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
		m_prevSmoothSkinningData = ssd->copy();
	}

	// test n data
    if ( deform_n )
    {
     	// verify the normal data is good
        if ( pt->variables.count(normal_var)==0 )
		{
        	throw Exception( "Could not find normal variable on primitive!" );
		}

        V3fVectorData *n = pt->variableData<V3fVectorData>(normal_var);
        if ( !n )
        {
        	throw Exception("Could not get normal data from primitive!");
        }

     	std::vector<V3f> &n_data =  n->writable();
        int n_size = n_data.size();

    	// todo, deal with the case that the primitive is a mesh and might have facevarying normal data
        if ( p_size != n_size )
        {
        	throw Exception("Position and normal variables must be the same length!");
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
        std::vector<V3f>::iterator p_it = p_data.begin();

		// todo, thread this
        // deform our P
        for ( p_it=p_data.begin(); p_it!=p_data.end(); ++p_it )
        {
			V3f p_new(0,0,0);

			int p_id = p_it - p_data.begin();
			int p_influence_count = ssd->pointInfluenceCounts()->readable()[p_id];
			int p_index_offset = ssd->pointIndexOffsets()->readable()[p_id];

			for (int p_influence_id = p_index_offset; p_influence_id < (p_index_offset+p_influence_count);
					p_influence_id++)
			{
				int influence_id = ssd->pointInfluenceIndices()->readable()[p_influence_id];
				float weight = ssd->pointInfluenceWeights()->readable()[p_influence_id];
				p_new += (*p_it) * skin_data[influence_id] * weight;
			}

			(*p_it) = p_new;

        }

        // deform our N
        if ( deform_n )
        {
        	V3fVectorData *n = pt->variableData<V3fVectorData>(normal_var);
         	std::vector<V3f> &n_data =  n->writable();

        	std::vector<V3f>::iterator n_it = n_data.begin();
        	V3f n_unw;
            for ( n_it=n_data.begin(); n_it!=n_data.end(); ++n_it )
            {
    			V3f n_new(0,0,0);

    			int n_id = n_it - n_data.begin();
    			int n_influence_count = ssd->pointInfluenceCounts()->readable()[n_id];
    			int n_index_offset = ssd->pointIndexOffsets()->readable()[n_id];

    			for (int n_influence_id = n_index_offset; n_influence_id < (n_index_offset+n_influence_count);
    					n_influence_id++)
    			{
    				int influence_id = ssd->pointInfluenceIndices()->readable()[n_influence_id];
    				float weight = ssd->pointInfluenceWeights()->readable()[n_influence_id];

    				skin_data[influence_id].multDirMatrix((*n_it),n_unw);
    				n_new += n_unw * weight;
    			}

    			(*n_it) = n_new;
            }
        }
	}
	else
	{
		// this should never happen
		assert(0);
	}

}
