//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#ifndef IECOREUSD_DATAALGO_INL
#define IECOREUSD_DATAALGO_INL

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/gf/quatf.h"
#include "pxr/base/gf/quatd.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreUSD
{

namespace DataAlgo
{

namespace Private
{

// Internal implementation of `toUSD()`

template<typename T>
typename CortexTypeTraits<T>::USDType toUSDInternal( const T &value, typename std::enable_if<CortexTypeTraits<T>::BitwiseEquivalent>::type *enabler = nullptr )
{
	using USDType = typename CortexTypeTraits<T>::USDType;
	return reinterpret_cast<const USDType &>( value );
}

template<typename T>
typename CortexTypeTraits<Imath::Quat<T>>::USDType toUSDInternal( const Imath::Quat<T> &src )
{
	using USDQuatType = typename CortexTypeTraits<Imath::Quat<T>>::USDType;
	using USDVecType = typename CortexTypeTraits<Imath::Vec3<T>>::USDType;
	return USDQuatType( src.r, USDVecType( src.v.x, src.v.y, src.v.z ) );
}

inline pxr::TfToken toUSDInternal( const IECore::InternedString &src )
{
	return pxr::TfToken( src.string() );
}

// Internal implementation of `fromUSD()`

template<typename T>
typename USDTypeTraits<T>::CortexType fromUSDInternal( const T &value, typename std::enable_if<USDTypeTraits<T>::BitwiseEquivalent>::type *enabler = nullptr )
{
	using CortexType = typename USDTypeTraits<T>::CortexType;
	return reinterpret_cast<const CortexType &>( value );
}

inline Imath::Quatf fromUSDInternal( const pxr::GfQuath &src )
{
	const auto &v = src.GetImaginary();
	return Imath::Quatf( src.GetReal(), Imath::V3f( v[0], v[1], v[2] ) );
}

inline Imath::Quatf fromUSDInternal( const pxr::GfQuatf &src )
{
	const auto &v = src.GetImaginary();
	return Imath::Quatf( src.GetReal(), Imath::V3f( v[0], v[1], v[2] ) );
}

inline Imath::Quatd fromUSDInternal( const pxr::GfQuatd &src )
{
	const auto &v = src.GetImaginary();
	return Imath::Quatd( src.GetReal(), Imath::V3d( v[0], v[1], v[2] ) );
}

inline IECore::InternedString fromUSDInternal( const pxr::TfToken &src )
{
	return IECore::InternedString( src.GetString() );
}

// Internal implementation of array conversions

template<typename T>
typename std::enable_if<USDTypeTraits<T>::BitwiseEquivalent, typename USDTypeTraits<T>::CortexVectorDataType::Ptr>::type fromUSDArrayInternal( const pxr::VtArray<T> &array )
{
	using CortexType = typename USDTypeTraits<T>::CortexType;
	using VectorDataType = typename USDTypeTraits<T>::CortexVectorDataType;
	using VectorType = typename VectorDataType::ValueType;
	return new VectorDataType(
		VectorType(
			reinterpret_cast<const CortexType *>( array.cdata() ),
			reinterpret_cast<const CortexType *>( array.cdata() ) + array.size()
		)
	);
}

template<typename T>
typename std::enable_if<!USDTypeTraits<T>::BitwiseEquivalent, typename USDTypeTraits<T>::CortexVectorDataType::Ptr>::type fromUSDArrayInternal( const pxr::VtArray<T> &array )
{
	using VectorDataType = typename USDTypeTraits<T>::CortexVectorDataType;
	typename VectorDataType::Ptr d = new VectorDataType;
	auto &v = d->writable();
	v.reserve( array.size() );
	for( const auto &e : array )
	{
		v.push_back( fromUSD( e ) );
	}
	return d;
}

} // namespace Private

template<typename T>
typename USDTypeTraits<T>::CortexType fromUSD( const T &value )
{
	return Private::fromUSDInternal( value );
}

template<typename T>
boost::intrusive_ptr< typename USDTypeTraits<T>::CortexVectorDataType > fromUSD( const pxr::VtArray<T> &array )
{
	return Private::fromUSDArrayInternal( array );
}

template<typename T>
typename CortexTypeTraits<T>::USDType toUSD( const T &value, typename std::enable_if<!std::is_void<typename CortexTypeTraits<T>::USDType>::value>::type *enabler )
{
	return Private::toUSDInternal( value );
}

} // namespace DataAlgo

} // namespace IECoreUSD

#endif // IECOREUSD_DATAALGO_INL
