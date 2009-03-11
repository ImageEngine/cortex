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

#include <cassert>

namespace IECore
{

template<typename P, typename V>	
CSGImplicitSurfaceFunction<P,V>::CSGImplicitSurfaceFunction( typename Fn::Ptr fn1, typename Fn::Ptr fn2, Mode mode ) : m_fn1(fn1), m_fn2(fn2), m_mode(mode)
{
	assert( m_fn1 );
	assert( m_fn2 );			
}

template<typename P, typename V>
typename CSGImplicitSurfaceFunction<P,V>::Value CSGImplicitSurfaceFunction<P,V>::operator()( const Point &p )
{
	assert( m_fn1 );
	assert( m_fn2 );			

	switch (m_mode)
	{
		case Union:					
			return std::min(
				m_fn1->operator()(p), 
				m_fn2->operator()(p)
			);
		case Intersection:
			return std::max(
				m_fn1->operator()(p), 
				m_fn2->operator()(p)
			);
		case Difference:	
			return std::max(
				m_fn1->operator()(p), 
				-m_fn2->operator()(p)
			);
		default:
			assert(false);
			return Value(0);
	}
}

template<typename P, typename V>
typename CSGImplicitSurfaceFunction<P,V>::Value CSGImplicitSurfaceFunction<P,V>::getValue( const Point &p )
{
	return this->operator()(p);
}

} // namespace IECore
