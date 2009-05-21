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

#ifndef IECORE_LOOKUP_H
#define IECORE_LOOKUP_H

#include "OpenEXR/ImathColor.h"

#include <vector>

namespace IECore
{

/// The Lookup class takes a function over a one dimensional domain and
/// accelerates its computation using linear interpolation between precomputed
/// values stored in a table.
template<typename X, typename Y>
class Lookup
{
	
	public :
	
		typedef X XType;
		typedef Y YType;
	
		Lookup();
	
		template<class Function>
		Lookup( const Function &function, XType xMin, XType xMax, unsigned numSamples );

		template<class Function>
		void init( const Function &function, XType xMin, XType xMax, unsigned numSamples );
		
		inline Y operator() ( X x ) const;
		
	private :
	
		std::vector<Y> m_values;
		XType m_xMin;
		XType m_xMax;
		XType m_xMult;
				
};

typedef Lookup<float, float> Lookupff;
typedef Lookup<double, double> Lookupdd;

typedef Lookup<float, Imath::Color3f> LookupfColor3f;
typedef Lookup<float, Imath::Color4f> LookupfColor4f;

} // namespace IECore

#include "IECore/Lookup.inl"

#endif // IECORE_LOOKUP_H
