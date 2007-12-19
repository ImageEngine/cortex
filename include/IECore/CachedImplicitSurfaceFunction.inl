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

namespace IECore
{

template<typename P, typename V>
CachedImplicitSurfaceFunction<P,V>::CachedImplicitSurfaceFunction( typename CachedImplicitSurfaceFunction<P,V>::Fn::Ptr fn, PointBaseType tolerance )
{
	assert( fn );
	assert( tolerance >= 0.0 );

	m_fn = fn;
	m_tolerance = tolerance;			
}		

template<typename P, typename V>
typename CachedImplicitSurfaceFunction<P,V>::Value CachedImplicitSurfaceFunction<P,V>::operator()( const CachedImplicitSurfaceFunction<P,V>::Point &p )
{				
	Key cacheKey(
		(KeyBaseType)((PointTraits::get(p, 0) - m_tolerance / 0.5) / m_tolerance),
		(KeyBaseType)((PointTraits::get(p, 1) - m_tolerance / 0.5) / m_tolerance),
		(KeyBaseType)((PointTraits::get(p, 2) - m_tolerance / 0.5) / m_tolerance)
	);		

	typename Cache::const_iterator it = m_cache.find( cacheKey );
	if ( it != m_cache.end() )
	{
		return it->m_value;
	}

	Value v = m_fn->operator()(p);	

	Element e;
	e.m_key = cacheKey;
	e.m_value = v;

	m_cache.insert( e );

	return v;						
}

template<typename P, typename V>
void CachedImplicitSurfaceFunction<P,V>::clear()
{
	m_cache.clear();
}

template<typename P, typename V>
typename CachedImplicitSurfaceFunction<P,V>::Cache::size_type CachedImplicitSurfaceFunction<P,V>::size() const
{
	return m_cache.size();
}	
	
template<typename P, typename V>
typename CachedImplicitSurfaceFunction<P,V>::Value CachedImplicitSurfaceFunction<P,V>::getValue( const CachedImplicitSurfaceFunction<P,V>::Point &p )
{
	return this->operator()(p);
}

}
