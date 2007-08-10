//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TURBULENCE_H
#define IE_CORE_TURBULENCE_H

#include "IECore/PerlinNoise.h"

namespace IECore
{

/// The Turbulence class template provides a pretty standard turbulence/fbm
/// implementation on top of the PerlinNoise class. Like the PerlinNoise class
/// it can operate across a broad range of input and output types. The template
/// parameter N is simply the type of a PerlinNoise instantiation to be used
/// in generating the turbulence.
template<typename N>
class Turbulence
{

	public :
	
		typedef N Noise;
		typedef typename N::Point Point;
		typedef typename VectorTraits<Point>::BaseType PointBaseType;
		typedef typename N::Value Value;
		typedef typename VectorTraits<Value>::BaseType ValueBaseType;
		
		/// Constructor. All the arguments are as you would expect. Vector values are
		/// used for lacunarity and gain so that they can be different in each dimension.
		/// If turbulent is true you get turbulence if it's false you get fbm.
		Turbulence( const unsigned int octaves = 4, const Value &gain = Value( 0.5 ),
			const Point &lacunarity = Point( 2.0 ), bool turbulent = true, const N &noise = N() );
		/// Copy constructor
		Turbulence( const Turbulence &other );

		//! @name Property access
		/// These functions let you modify the properties
		/// of the turbulence.
		////////////////////////////////////////////
		//@{
		void setOctaves( unsigned int octaves );
		unsigned int getOctaves() const;
		
		void setGain( const Value &gain );
		const Value &getGain() const;
		
		void setLacunarity( const Point &lacunarity );
		const Point &getLacunarity() const;
		
		void setTurbulent( bool turbulent );
		bool getTurbulent() const;
		
		void setNoise( const Noise &n );
		const Noise &getNoise() const;
		//@}

		/// Returns the turbulence value at the specified point. The range
		/// of the components of the returned value is from -0.5 to 0.5.
		Value turbulence( const Point &p ) const;

	private :
	
		// This calculates m_offset and m_scale so as to bring the
		// result into the appropriate -0.5 to 0.5 range.
		void calculateScaleAndOffset();
		Value m_offset;
		Value m_scale;
		
		unsigned int m_octaves;
		Value m_gain;
		Point m_lacunarity;
		bool m_turbulent;
		
		Noise m_noise;

};

/// Typedefs for common uses
typedef Turbulence<PerlinNoiseV3ff> TurbulenceV3ff;
typedef Turbulence<PerlinNoiseV2ff> TurbulenceV2ff;
typedef Turbulence<PerlinNoiseff> Turbulenceff;

typedef Turbulence<PerlinNoiseV3fV2f> TurbulenceV3fV2f;
typedef Turbulence<PerlinNoiseV2fV2f> TurbulenceV2fV2f;
typedef Turbulence<PerlinNoisefV2f> TurbulencefV2f;

typedef Turbulence<PerlinNoiseV3fV3f> TurbulenceV3fV3f;
typedef Turbulence<PerlinNoiseV2fV3f> TurbulenceV2fV3f;
typedef Turbulence<PerlinNoisefV3f> TurbulencefV3f;

typedef Turbulence<PerlinNoiseV3fColor3f> TurbulenceV3fColor3f;
typedef Turbulence<PerlinNoiseV2fColor3f> TurbulenceV2fColor3f;
typedef Turbulence<PerlinNoisefColor3f> TurbulencefColor3f;

} // namespace IECore

#include "IECore/Turbulence.inl"

#endif // IE_CORE_TURBULENCE_H
