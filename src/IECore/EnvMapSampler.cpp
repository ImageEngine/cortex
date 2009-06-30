//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/EnvMapSampler.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/MedianCutSampler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/LuminanceOp.h"
#include "IECore/AngleConversion.h"
#include "IECore/SphericalToEuclidianTransform.h"

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( EnvMapSampler );

EnvMapSampler::EnvMapSampler()
	:
	Op(
		staticTypeName(),
		"Samples an environment map to generate lights.",
		new ObjectParameter( "result",
			"A CompoundObject containing the light directions and colours.",
			NullObject::defaultNullObject(),
			CompoundObject::staticTypeId()
		)
	)
{
	m_imageParameter = new ImagePrimitiveParameter(
		"image",
		"The image to sample lights from. This should be in lat-long format, and"
		"must have 3 float channels named R, G and B.",
		new ImagePrimitive
	);

	m_subdivisionDepthParameter = new IntParameter(
		"subdivisionDepth",
		"The number of times to subdivide the image. This controls how many "
		"lights will be created.",
		4
	);

	parameters()->addParameter( m_imageParameter );
	parameters()->addParameter( m_subdivisionDepthParameter );

}

EnvMapSampler::~EnvMapSampler()
{
}

ImagePrimitiveParameterPtr EnvMapSampler::imageParameter()
{
	return m_imageParameter;
}

ConstImagePrimitiveParameterPtr EnvMapSampler::imageParameter() const
{
	return m_imageParameter;
}

IntParameterPtr EnvMapSampler::subdivisionDepthParameter()
{
	return m_subdivisionDepthParameter;
}

ConstIntParameterPtr EnvMapSampler::subdivisionDepthParameter() const
{
	return m_subdivisionDepthParameter;
}

ObjectPtr EnvMapSampler::doOperation( ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr image = static_pointer_cast<ImagePrimitive>( imageParameter()->getValue() )->copy();
	Box2i dataWindow = image->getDataWindow();

	// find the rgb channels
	ConstFloatVectorDataPtr redData = image->getChannel<float>( "R" );
	ConstFloatVectorDataPtr greenData = image->getChannel<float>( "G" );
	ConstFloatVectorDataPtr blueData = image->getChannel<float>( "B" );
	if( !(redData && greenData && blueData) )
	{
		throw Exception( "Image does not contain valid RGB float channels." );
	}
	const vector<float> &red = redData->readable();
	const vector<float> &green = greenData->readable();
	const vector<float> &blue = blueData->readable();

	// get a luminance channel
	LuminanceOpPtr luminanceOp = new LuminanceOp();
	luminanceOp->inputParameter()->setValue( image );
	luminanceOp->copyParameter()->getTypedValue() = false;
	luminanceOp->removeColorPrimVarsParameter()->getTypedValue() = false;
	luminanceOp->operate();

	// do the median cut thing to get some samples
	MedianCutSamplerPtr sampler = new MedianCutSampler;
	sampler->imageParameter()->setValue( image );
	sampler->subdivisionDepthParameter()->setNumericValue( subdivisionDepthParameter()->getNumericValue() );
	ConstCompoundObjectPtr samples = static_pointer_cast<CompoundObject>( sampler->operate() );
	const vector<V2f> &centroids = static_pointer_cast<V2fVectorData>( samples->members().find( "centroids" )->second )->readable();
	const vector<Box2i> &areas = static_pointer_cast<Box2iVectorData>( samples->members().find( "areas" )->second )->readable();

	// get light directions and colors from the samples
	V3fVectorDataPtr directionsData = new V3fVectorData;
	Color3fVectorDataPtr colorsData = new Color3fVectorData;
	vector<V3f> &directions = directionsData->writable();
	vector<Color3f> &colors = colorsData->writable();

	float radiansPerPixel = M_PI / (dataWindow.size().y + 1);
	float angleAtTop = ( M_PI - radiansPerPixel ) / 2.0f;

	Imath::M44f rotX90 = Imath::Eulerf( -M_PI * 0.5, 0, 0 ).toMatrix44();
	SphericalToEuclidianTransform< Imath::V2f, Imath::V3f > sph2euc;

	for( unsigned i=0; i<centroids.size(); i++ )
	{
		const Box2i &area = areas[i];
		Color3f color( 0 );
		for( int y=area.min.y; y<=area.max.y; y++ )
		{
			int yRel = y - dataWindow.min.y;

			float angle = angleAtTop - yRel * radiansPerPixel;
			float weight = cosf( angle );
			int index = (area.min.x - dataWindow.min.x) + (dataWindow.size().x + 1 ) * yRel;
			for( int x=area.min.x; x<=area.max.x; x++ )
			{
				color[0] += weight * red[index];
				color[1] += weight * green[index];
				color[2] += weight * blue[index];
				index++;
			}
		}
		color /= red.size();
		colors.push_back( color );

		Imath::V2f phiTheta( lerpfactor( (float)centroids[i].x, (float)dataWindow.min.x, (float)dataWindow.max.x ) * 2 * M_PI,
							 lerpfactor( (float)centroids[i].y, (float)dataWindow.min.y, (float)dataWindow.max.y ) * M_PI
		);
		// rotate coordinates along X axis so that the image maps Y coordinates to the vertical direction instead of Z.
		V3f direction = sph2euc.transform( phiTheta ) * rotX90;
		directions.push_back( -direction ); // negated so we output the direction the light shines in
	}

	// return the result
	CompoundObjectPtr result = new CompoundObject;
	result->members()["directions"] = directionsData;
	result->members()["colors"] = colorsData;
	return result;
}

