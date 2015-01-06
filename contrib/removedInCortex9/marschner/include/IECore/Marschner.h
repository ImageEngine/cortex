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

#ifndef IECORE_MARSCHNERBCSDF_H
#define IECORE_MARSCHNERBCSDF_H

#include "IECore/Exception.h"
#include "IECore/EuclideanToSphericalTransform.h"
#include "IECore/CompoundData.h"
#include "IECore/PerlinNoise.h"

namespace IECore
{

/// This class implements the Marschner et al. BCSDF according to:
/// [1] "Light Scattering from Human Hair Fibers" by Marschner et al. 2003 and
/// [2] "Light Scattering from Filaments" by Arno Zinke and Andreas Weber
/// T is the type of the absorption coefficient. If it's frequency-based then it can be Imath::Color3<T>. Otherwise if can be float or double.
/// This class assumes the given euclidean positions are already converted to the hair frame of reference as described below:
/// The Z component is aligned to the hair and it goes from the root to the tip.
/// The X component is the major axis for the cross section of the hair - it is important in case the hair is elliptical ( eccentricity != 1 ).
/// The Y component completes the right-handed orthonormal basis.
template < typename T >
class MarschnerBCSDF
{
	public:
	
		typedef typename VectorTraits<T>::BaseType V;
		typedef Imath::Vec3<V> V3;
		typedef Imath::Vec2<V> V2;

		/// Constructs the BCSDF.
		/// Check [1] for the description of parameters.
		MarschnerBCSDF( V refraction,
						const T &absorption,
						V eccentricity,
						V shiftR,
						V shiftTT,
						V shiftTRT,
						V widthR,
						V widthTT,
						V widthTRT,
						V glint,
						V causticWidth,
						V causticFade,
						V causticLimit );
		
		~MarschnerBCSDF();
		
		/// Computes the light absorption for a given light and eye position.
		/// The positions are in Spherical Coordinates around the hair as described in [1].
		/// the [0] or 'x' component is the azimuth, 0 to 2PI, where X is 0 and Y is PI/2, (X, and Y as defined in the class notes)
		/// The [1] or 'y' component is the elevation, -PI/2 to PI/2 where 0 is perpendicular to the hair.
		T operator() ( const V2 &eye, const V2 &light ) const;
		/// The R, TT, and TRT values passed by reference will be filled with the individual contributions from
		/// the corresponding reflectance model component.
		T operator() ( const V2 &eye, const V2 &light, T &R, T &TT, T &TRT ) const;
		 
		//! @name M term computation
		/// Computes the M term for the R, TT, amd TRT components, as per
		/// [1] Section 4.4 Angles should be as per the () operator.
		/// The M calculations only make use of the elevation components of the input angles,
		/// Taking the relative elevation, but also considering the light elevation directly.
		//@{
		V MR( const V2 &eye, const V2 &light ) const;
		V MTT( const V2 &eye, const V2 &light ) const;
		V MTRT( const V2 &eye, const V2 &light ) const;
		//@}
		
		//! @name N term computation
		/// These function compute the N term for the R, TT, and TRT components, as per
		/// [1] Section 4.4. Angles should be as per the () operator.
		/// The N terms take into consideration the relative elevation, relative azimuth
		/// AND the light elevation.
		//@{
		T NR( const V2 &eye, const V2 &light ) const;
		T NTT( const V2 &eye, const V2 &light ) const;
		T NTRT( const V2 &eye, const V2 &light ) const;
		//@}
		
		/// A convenience function to allow conversion of world space position into hair spherical coordinates.
		/// \param hairSystem a matrix defining the local coordinate system of the hair segment with the basis
		/// described in the class documentation.
		V2 computeLocalVector( const Imath::M44f hairSystem, const V3 &pos ) const;
		/// A convenience function to allow conversion of hair local 3d position into hair spherical coordinates.
		V2 computeLocalVector( const V3 &pos ) const;
			
	private:
	
		V m_refraction;
		T m_absorption;
		V m_eccentricity;
		V m_shiftR, m_shiftTT, m_shiftTRT;
		V m_widthR, m_widthTT, m_widthTRT;
		V m_glint;
		V m_causticWidth, m_causticFade, m_causticLimit;
		EuclideanToSphericalTransform< V3, V2 > m_sphericalConverter;

		static V bravaisIndex( V theta, V eta );
		static V fresnel( V incidenceAngle, V etaPerp, V etaParal, bool invert );
		static V targetAngle( int p, V relativeAzimuth );

		V eccentricityRefraction( V averageAzimuth ) const;

 		static V exitAngle( int p, V eta, V h );
		static V dExitAngle( int p, V eta, V h );
		static V ddExitAngle( int p, V eta, V h );

		V gaussian( V a, V b, V c, V x ) const;
		V3 gaussianPDF( V mu, V sigma ) const;

		V marschnerM( V shift, V width, V normWidth, V x ) const;
		T marschnerA( int p, V gammaI, V refraction, V etaPerp, V etaParal, const V2 &light ) const;
		T marschnerNP( int p, V refraction, V etaPerp, V etaParal, const V2 &light, V targetAngle ) const;
		T marschnerNTRT( V refraction, V etaPerp, V etaParal, const V2 &light, V targetAngle ) const;
				
		static int linearRoots( V a, V b, V &root );
		static int quadraticRoots( V a, V b, V c, V roots[2] );
		static int cubicRoots( V A, V B, V C, V D, V roots[3] );
		static int normalizedCubicRoots( V A, V B, V C, V roots[3] );
		static V cubicRoot( V v );
};

/// Typedefs for common uses
typedef MarschnerBCSDF< Imath::Color3f > MarschnerBCSDFC3f;

} // namespace

#include "IECore/Marschner.inl"

#endif // IECORE_MARSCHNERBCSDF_H
