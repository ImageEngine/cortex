//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_ASSOCIATEDLEGENDRE_H
#define IECORE_ASSOCIATEDLEGENDRE_H

#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"
#include <vector>

namespace IECore
{

// Implements associated Legendre polynomial computation.
// Based mainly on "Spherical Harmonic Lighting: The Gritty Details" by Robin Green.
template < typename V >
class AssociatedLegendre
{
	public :

		BOOST_STATIC_ASSERT( boost::is_floating_point<V>::value );

		// computes the function for a given band m and parameter m.
		// uses formula: ((-1)^mm) * (2mm-1))!! * (1-x^2)^(mm/2)
		static V evaluate( unsigned int mm, V x );

		// computes the function for a given band l and parameter m based on previous computed values for bands l-1 and l-2 with parameter m.
		static V evaluateFromRecurrence1( unsigned int l, unsigned int m, V x, V p1, V p2 );

		// computes the function for a given band l+1 and parameter l based on previous computed value for band l and parameters l.
		static V evaluateFromRecurrence2( unsigned int l, V x, V p1 );

		// computes the function for a given band l and parameter m.
		static V evaluate( unsigned int l, unsigned int m, V x );

		// computes the normalization factor for the function on a given band l and parameter m.
		static V normalizationFactor( unsigned int l, unsigned int m );

};

} // namespace IECore

#include "AssociatedLegendre.inl"

#endif // IECORE_ASSOCIATEDLEGENDRE_H
