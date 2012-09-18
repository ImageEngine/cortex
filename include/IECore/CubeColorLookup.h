//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CUBECOLORLOOKaaUP_H
#define IE_CORE_CUBECOLORLOOKaaUP_H

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"

#include "IECore/TypedData.h"
#include "IECore/Interpolator.h"

namespace IECore
{

/// The CubeColorLookup, templated on either float or double, performs interpolated lookups into color tables.
template<typename T>
class CubeColorLookup
{
	public:

		friend class TypedData< CubeColorLookup< T > >;

		typedef enum
		{
			NoInterpolation,
			Linear
		} Interpolation ;

		typedef T BaseType;
		typedef std::vector< Imath::Color3<T> > DataType;

		typedef Imath::Vec3<T> VecType;
		typedef Imath::Box< VecType > BoxType;
		typedef Imath::Color3<T> ColorType;

		/// Constructs an identity lookup in the domain [0,1]x[0,1]x[0,1]
		CubeColorLookup();

		/// Constructs a lookup with the given dimensions and data, over the specified domain and interpolation. The data layout should be in C-array format,
		/// i.e. lookups of data[x][y][z] work as expected.
		CubeColorLookup( const Imath::V3i &dimension, const DataType &data, const BoxType &domain = BoxType( VecType( 0, 0, 0 ), VecType( 1, 1, 1 ) ), Interpolation interpolation = Linear );
		virtual ~CubeColorLookup();

		// Sets the interpolation type performed by this lookup
		void setInterpolation( Interpolation i );

		/// Performs a color lookup
		inline ColorType operator() ( const ColorType &color ) const;

		/// Sets the values held by this lookup
		void setCube( const Imath::V3i &dimension, const DataType &data, const BoxType &domain = BoxType( VecType( 0, 0, 0 ), VecType( 1, 1, 1 ) ) );

		/// Retrieve fundamental properties of the lookup
		const Imath::V3i &dimension() const;
		const BoxType &domain() const;
		const DataType &data() const;
		Interpolation getInterpolation() const;

		inline bool operator==( const CubeColorLookup &rhs ) const;
		inline bool operator!=( const CubeColorLookup &rhs ) const;

	protected:

		inline VecType normalizedCoordinates( const ColorType &color ) const;
		inline int clamp( int v, int min, int max ) const;

		Imath::V3i m_dimension;
		BoxType m_domain;
		DataType m_data;
		Interpolation m_interpolation;
};

typedef CubeColorLookup<float> CubeColorLookupf;
typedef CubeColorLookup<double> CubeColorLookupd;

}

#include "IECore/CubeColorLookup.inl"

#endif // IE_CORE_CUBECOLORLOOKUP_H
