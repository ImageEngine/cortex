//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/LensDistortOp.h"
#include "IECore/LensModel.h"
#include "IECore/FastFloat.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CachedReader.h"
#include "IECore/Interpolator.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"

using namespace boost;
using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( LensDistortOp );

LensDistortOp::LensDistortOp()
	:	WarpOp(
			"Distorts an ImagePrimitive using a parametric lens model which is supplied as a .cob file. "
			"The resulting image will have the same display window as the original with a different data window."
		)
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
	return m_lensParameter;
}

const ObjectParameter * LensDistortOp::lensParameter() const
{
	return m_lensParameter;
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
	
	m_imageSize = inputImage->getDisplayWindow().size();
	m_imageDataWindow = inputImage->getDataWindow();

	// Get the distorted window.
	m_distortedDataWindow = m_lensModel->bounds( m_mode, m_imageDataWindow, m_imageSize.x, m_imageSize.y );

	// Compute a 2D cache of the warped points for use in the warp() method.
	IECore::FloatVectorDataPtr cachePtr = new IECore::FloatVectorData;
	std::vector<float> &cache( cachePtr->writable() );
	cache.resize( ( m_distortedDataWindow.size().x + 1 ) * ( m_distortedDataWindow.size().y + 1 ) * 2 ); // We interleave the X and Y vector components within the cache.

	if ( m_mode == kDistort )
	{
		for( int y = m_distortedDataWindow.min.y, pixelIndex = 0; y <= m_distortedDataWindow.max.y; y++ )
		{
			for( int x = m_distortedDataWindow.min.x; x <= m_distortedDataWindow.max.x; x++ )
			{
				Imath::V2f inPos( distort( Imath::V2f( x, y ) ) );
				cache[pixelIndex++] = inPos[0];
				cache[pixelIndex++] = inPos[1];
			}
		}
	}
	else
	{
		for( int y = m_distortedDataWindow.min.y, pixelIndex = 0; y <= m_distortedDataWindow.max.y; y++ )
		{
			for( int x = m_distortedDataWindow.min.x; x <= m_distortedDataWindow.max.x; x++ )
			{
				Imath::V2f inPos( undistort( Imath::V2f( x, y ) ) );
				cache[pixelIndex++] = inPos[0];
				cache[pixelIndex++] = inPos[1];
			}
		}
	}

	m_cachePtr = cachePtr;
}

Imath::Box2i LensDistortOp::warpedDataWindow( const Imath::Box2i &dataWindow ) const
{
	return m_distortedDataWindow;
}

Imath::V2f LensDistortOp::undistort( const Imath::V2f &p ) const
{
	// Convert to UV space.
	Imath::V2d uv = p / m_imageSize;

	// Undistort and convert to pixel space
	Imath::V2d dp( m_lensModel->undistort( uv ) * m_imageSize );
	
	return dp;
}

Imath::V2f LensDistortOp::distort( const Imath::V2f &p ) const
{
	// Convert to UV space.
	Imath::V2d uv = p / m_imageSize;
	
	// Distort and convert to pixel space
	Imath::V2d dp( m_lensModel->distort( uv ) * m_imageSize );
	
	return dp;
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

