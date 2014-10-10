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
//	     other contributors to this software may be used to endorse or
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

#include "IECore/NumericParameter.h"
#include "IECore/StandardRadialLensModel.h"

namespace IECore
{

IE_CORE_DEFINERUNTIMETYPED( StandardRadialLensModel );

// Register the new lens model.
LensModel::LensModelRegistration<StandardRadialLensModel> StandardRadialLensModel::m_registration;

StandardRadialLensModel::StandardRadialLensModel()
	// Diagonal width of the film back divided by 2.
	: m_filmbackDiagonal( 0.707106781 )
{
	// Focal Length
	std::string name("focalLengthCm");
	std::string description( "The focal length of the lens in cm.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 10. ) );
	
	// Film Back Width
	name = std::string("filmbackWidthCm");
	description = std::string( "The film back width of the camera in cm.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 1. ) );
	
	// Film Back Height
	name = std::string("filmbackHeightCm");
	description = std::string( "The film back height of the camera in cm.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 1. ) );
	
	// Lens Offset X
	name = std::string("lensCenterOffsetXCm");
	description = std::string( "The horizontal offset of the center of the lens from the filmback center in cm.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
	
	// Lens Offset Y
	name = std::string("lensCenterOffsetYCm");
	description = std::string( "The vertical offset of the center of the lens from the filmback center in cm.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
	
	// Pixel Aspect
	name = std::string("pixelAspect");
	description = std::string( "The pixel aspect ratio.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 1. ) );
	
	// Distortion
	name = std::string("distortion");
	description = std::string( "The distortion.");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
	
	// Anamorphic Squeeze
	name = std::string("anamorphicSqueeze");
	description = std::string( "Anamorphic Squeeze");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 1. ) );
	
	// Curvature X
	name = std::string("curvatureX");
	description = std::string( "Curvature X");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
	
	// Curvature Y
	name = std::string("curvatureY");
	description = std::string( "Curvature Y");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
	
	// Quartic Distortion
	name = std::string("quarticDistortion");
	description = std::string( "Quartic Distortion");
	parameters()->addParameter( new IECore::DoubleParameter( name, description, 0. ) );
}

StandardRadialLensModel::~StandardRadialLensModel(){}

void StandardRadialLensModel::validate()
{
	double filmBackWidth( parameters()->parameter<IECore::DoubleParameter>("filmbackWidthCm")->getNumericValue() );
	double filmBackHeight( parameters()->parameter<IECore::DoubleParameter>("filmbackHeightCm")->getNumericValue() );
	double filmBackOffsetX( parameters()->parameter<IECore::DoubleParameter>("lensCenterOffsetXCm")->getNumericValue() );
	double filmBackOffsetY( parameters()->parameter<IECore::DoubleParameter>("lensCenterOffsetYCm")->getNumericValue() );

	// Calculate the diagonal length of the filmback.
	m_filmbackDiagonal = sqrt( filmBackWidth * filmBackWidth * .25 + filmBackHeight * filmBackHeight * .25 );

	// Normalize the filmback and the offset by the diagonal radius of the filmback.
	m_dnFilmback = Imath::V2d( filmBackWidth / m_filmbackDiagonal, filmBackHeight / m_filmbackDiagonal );
	m_dnOffset = Imath::V2d( filmBackOffsetX / m_filmbackDiagonal, filmBackOffsetY / m_filmbackDiagonal );

	const double distortion( parameters()->parameter<IECore::DoubleParameter>("distortion")->getNumericValue() );
	const double anamorphicSqueeze( parameters()->parameter<IECore::DoubleParameter>("anamorphicSqueeze")->getNumericValue() );
	const double quarticDistortion( parameters()->parameter<IECore::DoubleParameter>("quarticDistortion")->getNumericValue() );
	const double curvatureX( parameters()->parameter<IECore::DoubleParameter>("curvatureX")->getNumericValue() );
	const double curvatureY( parameters()->parameter<IECore::DoubleParameter>("curvatureY")->getNumericValue() );
	
	// Use the parameters to calculate the coefficients that are needed by the distortion algorithm.
	m_cxx = distortion / anamorphicSqueeze;
	m_cxy = (distortion + curvatureX) / anamorphicSqueeze;
	m_cyx = distortion + curvatureY;
	m_cyy = distortion;
	m_cxxx = quarticDistortion / anamorphicSqueeze;
	m_cxxy = 2.0 * quarticDistortion / anamorphicSqueeze;
	m_cxyy = quarticDistortion / anamorphicSqueeze;
	m_cyxx = quarticDistortion;
	m_cyyx = 2.0 * quarticDistortion;
	m_cyyy = quarticDistortion;
}

Imath::V2d StandardRadialLensModel::undistort( Imath::V2d p )
{
	Imath::V2d dn( UVtoDN( p ) );
	
	const double dnx2( dn.x*dn.x );
	const double dny2( dn.y*dn.y );
	const double dnx4( dnx2*dnx2 );
	const double dny4( dny2*dny2 );
	
	dn.x = dn.x * (1. + m_cxx*dnx2 + m_cxy*dny2 + m_cxxx*dnx4 + m_cxxy*dnx2*dny2 + m_cxyy*dny4);
	dn.y = dn.y * (1. + m_cyx*dnx2 + m_cyy*dny2 + m_cyxx*dnx4 + m_cyyx*dnx2*dny2 + m_cyyy*dny4);
	
	return DNtoUV( dn );
}

Imath::V2d StandardRadialLensModel::distort( Imath::V2d p )
{
	Imath::V2d dn( UVtoDN( p ) );
	const Imath::V2d dnl( dn );
	
	// Use Newtons method to derive the value of the undistorted point...
	for (unsigned int i = 0; i < 15; i++)
	{
		// Calculate the first derivative matrix.
		Imath::M33d fd;
		const double dnx2( dn.x*dn.x ), dny2( dn.y*dn.y );
		const double dnx4( dnx2*dnx2 ), dny4( dny2*dny2 );
		
		fd[0][0] = 1.0 + 3.0*m_cxx*dnx2 + m_cxy*dn.y*dn.y + 5.*m_cxxx*dnx4 + 3.*m_cxxy*dnx2*dny2 + m_cxyy*dny4;
		fd[1][0] = 2.0*m_cxy*dn.x*dn.y + 2.*m_cxxy*dnx2*dn.x*dn.y + 4.*m_cxyy*dny2*dn.y*dn.x;
		fd[0][1] = 2.0*m_cyx*dn.x*dn.y + 2.*m_cyyx*dny2*dn.y*dn.x + 4.*m_cyxx*dnx2*dn.x*dn.y;
		fd[1][1] = 1.0 + 3.0*m_cyy*dny2 + m_cyx*dnx2 + 5.*m_cyyy*dny4 + 3.*m_cyyx*dnx2*dny2 + m_cyxx*dnx4;
		
		Imath::V2d fDist;
		fDist.x = dn.x * (1. + m_cxx*dnx2 + m_cxy*dny2 + m_cxxx*dnx4 + m_cxxy*dnx2*dny2 + m_cxyy*dny4);
		fDist.y = dn.y * (1. + m_cyx*dnx2 + m_cyy*dny2 + m_cyxx*dnx4 + m_cyyx*dnx2*dny2 + m_cyyy*dny4);
		
		dn -= (fDist-dnl)*fd.gjInverse();
	}
	
	return DNtoUV( dn );
}

// Transforms UV coordinates in the range 0-1 to the dimesionless
// coordinates which are used by the distortion algorithm.
Imath::V2d StandardRadialLensModel::UVtoDN( const Imath::V2d& uv )
{
	// Convert the UV coordinates to FOV coordinates.
	// FOV coordinates range from -1 to 1 in both axis.
	Imath::V2d fov( uv * Imath::V2d( 2. ) - Imath::V2d( 1. ) );
	// Normalize the FOV coordinates so that the length from the
	// center of the filmback to the corner is 1. This is referred
	// to as the dimesionless coordiante system.
	Imath::V2d dn( m_dnFilmback * fov * Imath::V2d( .5 ) - m_dnOffset );
	return dn;
}

// Transforms the dimesionless coordinates that are used by the
// distortion algorithm to UV coordinates in the range of 0-1.
Imath::V2d StandardRadialLensModel::DNtoUV( const Imath::V2d& dn )
{
	// Convert the dimesionless coordinates to FOV coordinates.
	// FOV coordinates range from -1 to 1 in both axis.
	Imath::V2d fov( ( ( dn + m_dnOffset ) * Imath::V2d( 2. ) ) / m_dnFilmback );
	// Convert from FOV coordinates to UV coordinates.
	Imath::V2d uv( ( fov + Imath::V2d( 1. ) ) * Imath::V2d( .5 ) );
	return uv;
}

} // namespace IECore


