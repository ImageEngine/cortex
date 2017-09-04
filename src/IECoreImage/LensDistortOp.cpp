//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "IECore/LensModel.h"
#include "IECore/FastFloat.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CachedReader.h"
#include "IECore/Interpolator.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/LensDistortOp.h"

using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( LensDistortOp );

LensDistortOp::LensDistortOp()
	:	WarpOp(
			"Distorts an ImagePrimitive using a parametric lens model which is supplied as a .cob file. "
			"The resulting image will have the same display window as the original with a different data window."
		),
		m_mode( kUndistort )
{

	IntParameter::PresetsContainer modePresets;
	modePresets.push_back( IntParameter::Preset( "Distort", kDistort ) );
	modePresets.push_back( IntParameter::Preset( "Undistort", kUndistort ) );

	m_modeParameter = new IntParameter(
		"mode",
		"Whether the distort the image or undistort it. An image with a lens distortion will need to be \"Undistorted\" to make it flat.",
		kUndistort,
		modePresets
	);

	m_lensParameter = new ObjectParameter(
		"lensModel",
		"An object parameter that describes the Lens Model to use. The compound parameter must contain a String object name \"lensModel\" that holds the name of the registered model to use.",
		new CompoundObject(),
		CompoundObjectTypeId
	);

	parameters()->addParameter( m_modeParameter );
	parameters()->addParameter( m_lensParameter );

}

LensDistortOp::~LensDistortOp()
{
}

ObjectParameter * LensDistortOp::lensParameter()
{
	return m_lensParameter.get();
}

const ObjectParameter * LensDistortOp::lensParameter() const
{
	return m_lensParameter.get();
}

void LensDistortOp::begin( const CompoundObject * operands )
{
	// Get the lens model parameters.
	IECore::CompoundObjectPtr lensModelParams( runTimeCast<CompoundObject>( lensParameter()->getValue() ) );
	
	// Load the lens object.
	m_lensModel = LensModel::create( lensModelParams );
	m_lensModel->validate();
	
	// Get the distortion mode.
	m_mode = m_modeParameter->getNumericValue();
	
	// Get our image information.
	assert( runTimeCast< ImagePrimitive >(inputParameter()->getValue()) );
	ImagePrimitive *inputImage = static_cast<ImagePrimitive *>( inputParameter()->getValue() );
	
	Imath::Box2i dataWindow( inputImage->getDataWindow() );
	Imath::Box2i displayWindow( inputImage->getDisplayWindow() );
	double displayWH[2] = { static_cast<double>( displayWindow.size().x + 1 ), static_cast<double>( displayWindow.size().y + 1 ) };
	double displayOrigin[2] = { static_cast<double>( displayWindow.min[0] ), static_cast<double>( displayWindow.min[1] ) };
	
	// Get the distorted window.
	// As the LensModel::bounds() method requires that the display window has it's origin at (0,0) in the bottom left of the image and the ImagePrimitive has it's origin in the top left,
	// convert to the correct image space and offset if by the display window's origin if it is non-zero.
	Imath::Box2i distortionSpaceBox(
		Imath::V2i( dataWindow.min[0] - displayWindow.min[0], displayWindow.size().y - ( dataWindow.max[1] - displayWindow.min[1] ) ),
		Imath::V2i( dataWindow.max[0] - displayWindow.min[0], displayWindow.size().y - ( dataWindow.min[1] - displayWindow.min[1] ) )
	);

	// Calculate the distorted data window.
	Imath::Box2i distortedWindow = m_lensModel->bounds( m_mode, distortionSpaceBox, ( displayWindow.size().x + 1 ), ( displayWindow.size().y + 1 ) );

	// Convert the distorted data window back to the same image space as ImagePrimitive.
	m_distortedDataWindow =  Imath::Box2i( 
		Imath::V2i( distortedWindow.min[0] + displayWindow.min[0], ( displayWindow.size().y - distortedWindow.max[1] ) + displayWindow.min[1] ),
		Imath::V2i( distortedWindow.max[0] + displayWindow.min[0], ( displayWindow.size().y - distortedWindow.min[1] ) + displayWindow.min[1] )
	);
	
	// Compute a 2D cache of the warped points for use in the warp() method.
	IECore::FloatVectorDataPtr cachePtr = new IECore::FloatVectorData;
	std::vector<float> &cache( cachePtr->writable() );
	cache.resize( ( m_distortedDataWindow.size().x + 1 ) * ( m_distortedDataWindow.size().y + 1 ) * 2 ); // We interleave the X and Y vector components within the cache.

	for( int y = distortedWindow.max.y, pixelIndex = 0; y >= distortedWindow.min.y; --y )
	{
		for( int x = distortedWindow.min.x; x <= distortedWindow.max.x; ++x )
		{
			// Convert to UV space with the origin in the bottom left.	
			Imath::V2f p( Imath::V2f( x, y ) );
			Imath::V2d uv( p[0] / displayWH[0], p[1] / displayWH[1] );

			// Get the distorted uv coordinate.
			Imath::V2d duv( m_mode == kDistort ? m_lensModel->distort( uv ) : m_lensModel->undistort( uv ) );

			// Transform it to image space.
			p = Imath::V2f(
				duv[0] * displayWH[0] + displayOrigin[0], ( ( displayWH[1] - 1. ) - ( duv[1] * displayWH[1] ) ) + displayOrigin[1] 
			);

			cache[pixelIndex++] = p[0];
			cache[pixelIndex++] = p[1];
		}
	}

	m_cachePtr = cachePtr;
}

Imath::Box2i LensDistortOp::warpedDataWindow( const Imath::Box2i &dataWindow ) const
{
	return m_distortedDataWindow;
}

Imath::V2f LensDistortOp::warp( const Imath::V2f &p ) const
{
	// Just pull the distorted point from the cache.
	const int w( m_distortedDataWindow.size().x + 1 );
	const int xIdx( int( p[0] ) - m_distortedDataWindow.min.x );
	const int yIdx( int( p[1] ) - m_distortedDataWindow.min.y );
	const float *vector( &(( m_cachePtr->readable() )[ ( w * yIdx + xIdx ) * 2 ] ) );
	return Imath::V2f( vector[0], vector[1] );
}

void LensDistortOp::end()
{
}

