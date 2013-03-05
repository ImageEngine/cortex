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
			"Distorts an ImagePrimitive using a parameteric lens model which is supplied as a .cob file. "
			"The resulting image will have the same display window as the origonal with a different data window."
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
}

Imath::Box2i LensDistortOp::warpedDataWindow( const Imath::Box2i &dataWindow ) const
{
	return m_lensModel->bounds( m_mode, m_imageDataWindow, m_imageSize.x, m_imageSize.y );
}

Imath::V2f LensDistortOp::warp( const Imath::V2f &p ) const
{
	// Convert to UV space.
	Imath::V2d uv = p / m_imageSize;
	
	// Distort/Undistort
	Imath::V2d dp;
	if ( m_mode == kDistort )
	{
		dp = m_lensModel->distort( uv );
	}
	else
	{
		dp = m_lensModel->undistort( uv );
	}
	
	// Convert to pixel space.
	dp *= m_imageSize;
	
	// Clamp to the data window.
	if( m_imageDataWindow.min.x > dp.x ) dp.x = m_imageDataWindow.min.x;
	if( m_imageDataWindow.min.y > dp.y ) dp.y = m_imageDataWindow.min.y;
	if( m_imageDataWindow.max.x-1.0 < dp.x ) dp.x = m_imageDataWindow.max.x-1;
	if( m_imageDataWindow.max.y-1.0 < dp.y ) dp.y = m_imageDataWindow.max.y-1; 
	
	return dp;
}

void LensDistortOp::end()
{
}

