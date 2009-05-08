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

#include "IECore/MedianCutSampler.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/SummedAreaOp.h"
#include "IECore/CompoundParameter.h"

#include "boost/multi_array.hpp"
#include "boost/format.hpp"

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MedianCutSampler );

MedianCutSampler::MedianCutSampler()
	:
	Op(
		staticTypeName(),
		"Performs importance sampling of an image.",
		new ObjectParameter( "result",
			"A CompoundObject containing a vector of areas which cover the image, and all of which "
			"represent the same amount of energy, and a vector of weighted centroids of these areas.",
			NullObject::defaultNullObject(),
			CompoundObject::staticTypeId()
		)
	)
{
	m_imageParameter = new ImagePrimitiveParameter(
		"image",
		"The image to sample from.",
		new ImagePrimitive
	);

	m_channelNameParameter = new StringParameter(
		"channelName",
		"The name of a channel to use when computing the point distribution.",
		"Y"
	);

	m_subdivisionDepthParameter = new IntParameter(
		"subdivisionDepth",
		"The number of times to subdivide the image. This controls how many "
		"points will be created.",
		4
	);

	IntParameter::PresetsContainer projectionPresets;
	projectionPresets.push_back( IntParameter::Preset( "rectilinear", Rectilinear ) );
	projectionPresets.push_back( IntParameter::Preset( "latLong", LatLong ) );
	m_projectionParameter = new IntParameter(
		"projection",
		"The projection the image represents. When in latLong mode the "
		"image intensities are weighted to account for pinching towards the ."
		"poles, and the splitting criteria is also weighted to account for "
		"changing box widths towards the poles.",
		2,
		1,
		2,
		projectionPresets,
		true
	);

	parameters()->addParameter( m_imageParameter );
	parameters()->addParameter( m_channelNameParameter );
	parameters()->addParameter( m_subdivisionDepthParameter );
	parameters()->addParameter( m_projectionParameter );

}

MedianCutSampler::~MedianCutSampler()
{
}

ImagePrimitiveParameterPtr MedianCutSampler::imageParameter()
{
	return m_imageParameter;
}

ConstImagePrimitiveParameterPtr MedianCutSampler::imageParameter() const
{
	return m_imageParameter;
}

StringParameterPtr MedianCutSampler::channelNameParameter()
{
	return m_channelNameParameter;
}

ConstStringParameterPtr MedianCutSampler::channelNameParameter() const
{
	return m_channelNameParameter;
}

IntParameterPtr MedianCutSampler::subdivisionDepthParameter()
{
	return m_subdivisionDepthParameter;
}

ConstIntParameterPtr MedianCutSampler::subdivisionDepthParameter() const
{
	return m_subdivisionDepthParameter;
}

IntParameterPtr MedianCutSampler::projectionParameter()
{
	return m_projectionParameter;
}

ConstIntParameterPtr MedianCutSampler::projectionParameter() const
{
	return m_projectionParameter;
}

/// \todo This functionality and the code in SummedAreaOp should be
/// refactored into a low level templated SummedAreaTable class that
/// operates on data passed to it.
typedef boost::multi_array_ref<float, 2> Array2D;
static inline float energy( const Array2D &summedLuminance, const Box2i &area )
{
	V2i min = area.min - V2i( 1 ); // box is inclusive so we need to step outside

	float a = summedLuminance[area.max.x][area.max.y];
	float b = min.y >= 0 ? summedLuminance[area.max.x][min.y] : 0.0f;
	float c = min.x >= 0 ? summedLuminance[min.x][area.max.y] : 0.0f;
	float d = min.x >= 0 && min.y >= 0 ? summedLuminance[min.x][min.y] : 0.0f;

	return a - b - c + d;
}

static void medianCut( const Array2D &luminance, const Array2D &summedLuminance, MedianCutSampler::Projection projection, const Box2i &area, vector<Box2i> &areas, vector<V2f> &centroids, int depth, int maxDepth )
{
	float radiansPerPixel = M_PI / (luminance.shape()[1]);

	if( depth==maxDepth )
	{
		float totalEnergy = 0.0f;
		V2f position( 0.0f );
		for( int y=area.min.y; y<=area.max.y; y++ )
		{
			for( int x=area.min.x; x<=area.max.x; x++ )
			{
				float e = luminance[x][y];
				position += V2f( x, y ) * e;
				totalEnergy += e;
			}
		}

		position /= totalEnergy;
		centroids.push_back( position );
		areas.push_back( area );
	}
	else
	{
		// find cut dimension
		V2f size = area.size();
		if( projection==MedianCutSampler::LatLong )
		{
			float centreY = (area.max.y + area.min.y) / 2.0f;
			float centreAngle = (M_PI - radiansPerPixel) / 2.0f - centreY * radiansPerPixel;
			size.x *= cosf( centreAngle );
		}
		int cutAxis = size.x > size.y ? 0 : 1;
		float e = energy( summedLuminance, area );
		float halfE = e / 2.0f;
		Box2i lowArea = area;
		while( e > halfE )
		{
			lowArea.max[cutAxis] -= 1;
			e = energy( summedLuminance, lowArea );
		}
		Box2i highArea = area;
		highArea.min[cutAxis] = lowArea.max[cutAxis] + 1;
		medianCut( luminance, summedLuminance, projection, lowArea, areas, centroids, depth + 1, maxDepth );
		medianCut( luminance, summedLuminance, projection, highArea, areas, centroids, depth + 1, maxDepth );
	}
}


ObjectPtr MedianCutSampler::doOperation( ConstCompoundObjectPtr operands )
{
	ImagePrimitivePtr image = static_pointer_cast<ImagePrimitive>( imageParameter()->getValue() )->copy();
	Box2i dataWindow = image->getDataWindow();

	// find the right channel
	const std::string &channelName = m_channelNameParameter->getTypedValue();
	FloatVectorDataPtr luminance = image->getChannel<float>( channelName );
	if( !luminance )
	{
		throw Exception( str( format( "No FloatVectorData channel named \"%s\"." ) % channelName ) );
	}

	// if the projection requires it, weight the luminances so they're less
	// important towards the poles of the sphere
	Projection projection = (Projection)m_projectionParameter->getNumericValue();
	if( projection==LatLong )
	{
		float radiansPerPixel = M_PI / (dataWindow.size().y + 1);
		float angle = ( M_PI - radiansPerPixel ) / 2.0f;

		float *p = &(luminance->writable()[0]);

		for( int y=dataWindow.min.y; y<=dataWindow.max.y; y++ )
		{
			float *pEnd = p + dataWindow.size().x + 1;
			float w = cosf( angle );
			while( p < pEnd )
			{
				*p *= w;
				p++;
			}
			angle -= radiansPerPixel;
		}

	}

	// make a summed area table for speed
	FloatVectorDataPtr summedLuminance = luminance;
	luminance = luminance->copy(); // we need this for the centroid computation

	SummedAreaOpPtr summedAreaOp = new SummedAreaOp();
	summedAreaOp->inputParameter()->setValue( image );
	summedAreaOp->copyParameter()->setTypedValue( false );
	summedAreaOp->channelNamesParameter()->getTypedValue().clear();
	summedAreaOp->channelNamesParameter()->getTypedValue().push_back( "Y" );
	summedAreaOp->operate();

	// do the median cut thing
	CompoundObjectPtr result = new CompoundObject;
	V2fVectorDataPtr centroids = new V2fVectorData;
	Box2iVectorDataPtr areas = new Box2iVectorData;
	result->members()["centroids"] = centroids;
	result->members()["areas"] = areas;

	dataWindow.max -= dataWindow.min;
	dataWindow.min -= dataWindow.min; // let's start indexing from 0 shall we?
	Array2D array( &(luminance->writable()[0]), extents[dataWindow.size().x+1][dataWindow.size().y+1], fortran_storage_order() );
	Array2D summedArray( &(summedLuminance->writable()[0]), extents[dataWindow.size().x+1][dataWindow.size().y+1], fortran_storage_order() );
	medianCut( array, summedArray, projection, dataWindow, areas->writable(), centroids->writable(), 0, subdivisionDepthParameter()->getNumericValue() );

	return result;
}

