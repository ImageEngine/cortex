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

#include "IECore/MarschnerLookupTableOp.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"

using namespace Imath;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( MarschnerLookupTableOp );

static TypeId objectTypes[] = { ImagePrimitiveTypeId, InvalidTypeId };


MarschnerLookupTableOp::MarschnerLookupTableOp()
	:	Op(
		"Calculates a 2D lookup table as a multi-channel image, for the supplied Marscher shading model parameters.",
		new ObjectParameter(
			"result",
			"A mult-channel ImagePrimitive",
			new ImagePrimitive(),
			objectTypes
		)
	)
{
	createParameters( true );
}

MarschnerLookupTableOp::MarschnerLookupTableOp( const bool absorptionAsColor )
	:	Op(
		"Calculates a 2D lookup table as a multi-channel image, for the supplied Marscher shading model parameters.",
		new ObjectParameter(
			"result",
			"A mult-channel ImagePrimitive",
			new ImagePrimitive(),
			objectTypes
		)
	)
{
	createParameters( absorptionAsColor );
}

void MarschnerLookupTableOp::createParameters( const bool asColor )
{
	m_modelParameter = new MarschnerParameter(
		"model",
		"The parameters for the shading model",
		asColor
	);
	
	m_resolutionParameter = new IntParameter(
		"resolution",
		"The resolution of the resulting lookup image",
		128,
		0
	);
	
	parameters()->addParameter( m_modelParameter );
	parameters()->addParameter( m_resolutionParameter );
}


MarschnerLookupTableOp::~MarschnerLookupTableOp()
{
}

MarschnerParameter *MarschnerLookupTableOp::modelParameter()
{
	return m_modelParameter.get();
}

const MarschnerParameter *MarschnerLookupTableOp::modelParameter() const
{
	return m_modelParameter.get();
}

IntParameter *MarschnerLookupTableOp::resolutionParameter()
{
	return m_resolutionParameter.get();
}

const IntParameter *MarschnerLookupTableOp::resolutionParameter() const
{
	return m_resolutionParameter.get();
}

ObjectPtr MarschnerLookupTableOp::doOperation( const CompoundObject *operands )
{
	
	// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter23.html
	
	unsigned resolution = m_resolutionParameter->getNumericValue();

	ImagePrimitivePtr lookup = new ImagePrimitive( Box2i( V2i( 0 ), V2i( resolution-1 ) ), Box2i( V2i( 0 ), V2i( resolution-1 ) ) );

	FloatVectorDataPtr cosDiffThetaData = lookup->createChannel<float>( "cosDiffTheta" );
	
	FloatVectorDataPtr mrData = lookup->createChannel<float>( "MR" );
	FloatVectorDataPtr mttData = lookup->createChannel<float>( "MTT" );
	FloatVectorDataPtr mtrtData = lookup->createChannel<float>( "MTRT" );
	
	FloatVectorDataPtr nrData = lookup->createChannel<float>( "NR" );
	
	FloatVectorDataPtr nttRData = lookup->createChannel<float>( "NTT.r" );
	FloatVectorDataPtr nttGData = lookup->createChannel<float>( "NTT.g" );
	FloatVectorDataPtr nttBData = lookup->createChannel<float>( "NTT.b" );
	
	FloatVectorDataPtr ntrtRData = lookup->createChannel<float>( "NTRT.r" );
	FloatVectorDataPtr ntrtGData = lookup->createChannel<float>( "NTRT.g" );
	FloatVectorDataPtr ntrtBData = lookup->createChannel<float>( "NTRT.b" );

	std::vector<float> &cosDiffTheta = cosDiffThetaData->writable();
	
	std::vector<float> &mr = mrData->writable();
	std::vector<float> &mtt = mttData->writable();
	std::vector<float> &mtrt = mtrtData->writable();
	
	std::vector<float> &nr = nrData->writable();
	
	std::vector<float> &nttR = nttRData->writable();
	std::vector<float> &nttG = nttGData->writable();
	std::vector<float> &nttB = nttBData->writable();
	
	std::vector<float> &ntrtR = ntrtRData->writable();
	std::vector<float> &ntrtG = ntrtGData->writable();
	std::vector<float> &ntrtB = ntrtBData->writable();

	V2f eye, light;
	float step = 2.0f / resolution;
	
	// theta = q = elevation
	// phi = f = azimuth
	//
	// light.x = fi
	// light.y = qi
	//
	// eye.x = fo
	// eye.y = qo
	//
	// fd = fi - fo
	// qd = ( qi - qo ) / 2
	
	float sinqo = -1.0f;
	float sinqi = -1.0f;

	float qd;
	
	// Because we have the eye/light V2 interface to the Marschner
	// function, we need to be a little sneaky. We can work out the 
	// .y angles based on the lookup inputs. Fortunately however
	// the .x inputs are only needed to computer relative angles
	// so we can leave them at 0.0, and achieve the same result.
	light.x = 0.0f;
	eye.x = 0.0f;
	
	V3f vNtt, vNtrt, all;
		
	MarschnerBCSDFC3f m = m_modelParameter->createBCSDF();

	// Calculate M terms, which depend on theta values only.
	// x = sinThetaI
	// y = sinThetaO
		
	unsigned i = 0;
	for( unsigned y = 0; y < resolution; y++ )
	{
		eye.y = asin( sinqo );		
		sinqi = -1.0f;
			
		for( unsigned x = 0; x < resolution; x++ )
		{
			light.y = asin( sinqi );
			
			qd = ( light.y - eye.y ) / 2.0;
			cosDiffTheta[i] = ( cos( qd ) + 1.0f ) / 2.0f;
		
			mr[i] = m.MR( eye, light ) / 30.0f;
			mtt[i] = m.MTT( eye, light ) / 30.0f;
			mtrt[i] = m.MTRT( eye, light ) / 30.0f;
			
			sinqi += step;							
			i++;
		}
		sinqo += step;
	}
	
	// Calculate N terms, which largely depends on relative angles,
	// Light elevation is used in marschnerA, so we have
	// to add our angle there, instead of on the eye
	// x = cosPhiD (cosfd)
	// y = cosTheaD (cosqd)
	
	/// \todo The range/method of these iterations needs optimising,
	/// the full angular range isn't exercised in practice.			
	
	float cosfd = -1.0f;
	float cosqd = -1.0f;
		
	light.x = 0.0f;
	
	float relativeTheta;
	
	i = 0;
	for( unsigned y = 0; y < resolution; y++ )
	{
		relativeTheta = acos( cosqd );
		
		light.y = relativeTheta/2.0;
		eye.y = -relativeTheta/2.0;
		
		cosfd = -1.0f;
			
		for( unsigned x = 0; x < resolution; x++ )
		{
			eye.x = acos( cosfd );
			
			nr[i] = m.NR( eye, light ).x;
			vNtt = m.NTT( eye, light );
			vNtrt = m.NTRT( eye, light );
			
			nttR[i] = vNtt.x;
			nttG[i] = vNtt.y;
			nttB[i] = vNtt.z;
			
			ntrtR[i] = vNtrt.x;
			ntrtG[i] = vNtrt.y;
			ntrtB[i] = vNtrt.z;
			
			cosfd += step;
			i++;
		}
		
		cosqd += step;		
	}
	
	return lookup;
}
