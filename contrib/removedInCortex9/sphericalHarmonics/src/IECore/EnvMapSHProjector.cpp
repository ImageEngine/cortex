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

#include "IECore/EnvMapSHProjector.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/SphericalHarmonicsAlgo.h"
#include "IECore/Math.h"

using namespace tbb;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( EnvMapSHProjector );

EnvMapSHProjector::EnvMapSHProjector(): Op( "Projects a Lat-Long environment map on a SphericalHarmonics object and returns the resulting SH coefficients.",
											new ObjectParameter( "result", "The result", new Color3fVectorData(), Color3fVectorData::staticTypeId() ) )
{
	m_bandsParameter = new IntParameter(
		"bands",
		"Number of Spherical Harmonics bands used.",
		3,
		1,
		20
	);

	m_samplesParameter = new IntParameter( 
		"samples",
		"Number of samples taken randomly from the image to build the convolved image.",
		40000,
		0
	);

	m_orientationParameter = new M44fParameter(
		"orientation",
		"Defines how the env map is oriented relative to the 3D space. The default value aligns the Y coordinate on the image to the Y axis in 3D space. If you set to identity then the Y coordinate on the image aligns to Z axis in 3D space.",
		new M44fData()
	);

	m_rightHandSystemParameter = new BoolParameter(
		"rightHandSystem",
		"Defines the handedness of the system when converting spherical to euclidean coordinates.",
		true
	);

	m_applyFilterParameter = new BoolParameter(
		"applyFilter",
		"Applies a filter on the resulting spherical harmonics. That is necessary if the map has big contrasting regions",
		true
	);

	m_envMapParameter = new ObjectParameter( 
		"input", 
		"The Lat-Long environment map", 
		new ImagePrimitive(), 
		ImagePrimitive::staticTypeId() 
	);

	parameters()->addParameter( m_bandsParameter );
	parameters()->addParameter( m_samplesParameter );
	parameters()->addParameter( m_orientationParameter );
	parameters()->addParameter( m_rightHandSystemParameter );
	parameters()->addParameter( m_applyFilterParameter );
	parameters()->addParameter( m_envMapParameter );
}

EnvMapSHProjector::~EnvMapSHProjector()
{
}

ObjectPtr EnvMapSHProjector::doOperation( const CompoundObject *operands )
{
	ImagePrimitive * image = runTimeCast< ImagePrimitive, Object >( m_envMapParameter->getValue() );

	if ( image->getDisplayWindow() != image->getDataWindow() )
	{
		throw Exception( "EnvMapSHProjector only works with images that display and data windows match." );
	}

	unsigned bands = m_bandsParameter->getNumericValue();
	unsigned samples = m_samplesParameter->getNumericValue();
	bool rightHandSystem = m_rightHandSystemParameter->getTypedValue();
	bool applyFilter = m_applyFilterParameter->getTypedValue();
	Imath::M44f orientation = m_orientationParameter->getTypedValue();

	int imgWidth = image->getDataWindow().size().x + 1;
	int imgHeight = image->getDataWindow().size().y + 1;

	// create SH projector
	IECore::SHProjectorf projector( samples );
	projector.computeSamples( bands );

	ConstFloatVectorDataPtr redData = image->getChannel< float >( "R" );
	ConstFloatVectorDataPtr greenData = image->getChannel< float >( "G" );
	ConstFloatVectorDataPtr blueData = image->getChannel< float >( "B" );

	if ( !redData || !greenData || !blueData )
	{
		throw Exception( "EnvMap does not have the three colour channels (R,G,B)!" );
	}

	const std::vector<float> &chR = redData->readable();
	const std::vector<float> &chG = greenData->readable();
	const std::vector<float> &chB = blueData->readable();

	// rotate coordinates along X axis so that the image maps Y coordinates to the vertical direction instead of Z.
	Imath::M44f rotX90 = Imath::Eulerf( M_PI * 0.5, 0, 0 ).toMatrix44();

	// \todo: check if the order of multiplication is what we expect...
	orientation = orientation * rotX90;

	EuclideanToSphericalTransform< Imath::V3f, Imath::V2f > euc2sph;
	std::vector< Imath::V3f >::const_iterator cit = projector.euclideanCoordinates().begin();

	SHColor3f sh( bands );
	unsigned int i;
	unsigned actualSamples = projector.euclideanCoordinates().size();
	Imath::V3f systemConversion(1);
	if ( !rightHandSystem )
	{
		systemConversion[2] = -systemConversion[2];
	}

	// image to SH
	for ( i = 0; i < actualSamples; i++, cit++ )
	{
		Imath::V2f phiTheta = euc2sph.transform( ((*cit) * systemConversion) * orientation );
		int ix = (int)(phiTheta.x * (float)imgWidth / ( M_PI * 2 ));
		int iy = (int)(phiTheta.y * (float)imgHeight /  M_PI );
		if ( ix > imgWidth )
			ix = imgWidth;
		if ( iy > imgHeight )
			iy = imgHeight;
		int offset = iy * imgWidth + ix;
		projector( i, Imath::Color3f( chR[ offset ], chG[ offset ], chB[ offset ] ), sh );
	}

	// filter SH
	if ( applyFilter )
	{
		// use author's suggestion for window size.
		IECore::windowingFilter( sh, 2*sh.bands() );
	}

	Color3fVectorDataPtr result = new Color3fVectorData( sh.coefficients() );
	return result;
}
