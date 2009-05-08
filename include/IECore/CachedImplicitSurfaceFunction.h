//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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


#ifndef IE_CORE_CACHEDIMPLICITSURFACEFUNCTION_H
#define IE_CORE_CACHEDIMPLICITSURFACEFUNCTION_H

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathLimits.h"

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/HashTable.h"

namespace IECore
{

/// A template to define an implicit surface function, which returns a value of type template parameter V when passed
/// a location of type template parameter P
template<typename P, typename V>
class CachedImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
{
	public:

		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;

		typedef ImplicitSurfaceFunction<P, V> Fn;

		IE_CORE_DECLAREMEMBERPTR2( CachedImplicitSurfaceFunction<P, V> );

	protected:

		typedef long KeyBaseType;
		typedef Imath::Vec3<KeyBaseType> Key;

		struct Hash
		{
			size_t operator()( const Key &t ) const
			{
				return t.x + 100 * ( t.y + 100 * t.z );
			}

		};

		typedef HashTable< Key, Value, Hash > Cache;

	public:


		CachedImplicitSurfaceFunction( typename Fn::Ptr fn, PointBaseType tolerance = Imath::limits<PointBaseType>::epsilon() );

		/// Returns the value of the underlying function
		inline Value operator()( const Point &p );

		/// Clears all the cache function values
		void clear();

		/// Returns the number of entries held in the cache
		typename Cache::size_type size() const;

		virtual Value getValue( const Point &p );

	protected:

		Cache m_cache;

		typename Fn::Ptr m_fn;
		PointBaseType m_tolerance;

};

typedef CachedImplicitSurfaceFunction<Imath::V3f, float> CachedImplicitSurfaceFunctionV3ff;
typedef CachedImplicitSurfaceFunction<Imath::V3f, double> CachedImplicitSurfaceFunctionV3fd;
typedef CachedImplicitSurfaceFunction<Imath::V3d, float> CachedImplicitSurfaceFunctionV3df;
typedef CachedImplicitSurfaceFunction<Imath::V3d, double> CachedImplicitSurfaceFunctionV3dd;

}

#include "IECore/CachedImplicitSurfaceFunction.inl"

#endif // IE_CORE_CACHEDIMPLICITSURFACEFUNCTION_H
