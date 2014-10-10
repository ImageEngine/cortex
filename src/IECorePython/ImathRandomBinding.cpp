//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECorePython/ImathRandomBinding.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Random.h"

#include "OpenEXR/ImathRandom.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

#include "boost/type_traits.hpp"

using namespace boost::python;
using namespace boost;
using namespace Imath;
using namespace IECore;

namespace IECorePython
{

template<typename R>
float nextFloat( R &r )
{
	return r.nextf();
}

template<typename T, typename R>
T nextVec( R &r )
{
	T result;
	for( unsigned int i=0; i<T::dimensions(); i++ )
	{
		result[i] = r.nextf();
	}
	return result;
}

template<typename R, typename ResultType, typename ResultType::ValueType::value_type (*F)( R & )>
struct Vectoriser
{
//	typedef ResultType::ValueType T;
	typedef typename ResultType::Ptr ResultTypePtr;
	typedef typename ResultType::ValueType ValueType;
	typedef typename ValueType::iterator Iterator;

	static ResultTypePtr vectorise( R &r, size_t size )
	{
		ResultTypePtr result = new ResultType;
		ValueType &v = result->writable();
		v.resize( size );
		for( Iterator it=v.begin(); it!=v.end(); it++ )
		{
			*it = F( r );
		}
		return result;
	};

	template<typename S>
	static ResultTypePtr vectoriseSeededT( R &r, typename S::ConstPtr seeds )
	{
		const typename S::ValueType &seedsV = seeds->readable();
		ResultTypePtr result = new ResultType;
		ValueType &v = result->writable();
		v.resize( seedsV.size() );
		typename S::ValueType::const_iterator sIt = seedsV.begin();
		for( Iterator it=v.begin(); it!=v.end(); it++ )
		{
			r.init( (long unsigned int)(*sIt++) );
			*it = F( r );
		}
		return result;
	}

	static ResultTypePtr vectoriseSeeded( R &r, ConstDataPtr seeds )
	{
		switch( seeds->typeId() )
		{
			case FloatVectorDataTypeId :
				return vectoriseSeededT<FloatVectorData>( r, boost::static_pointer_cast<const FloatVectorData>( seeds ) );
			case DoubleVectorDataTypeId :
				return vectoriseSeededT<DoubleVectorData>( r, boost::static_pointer_cast<const DoubleVectorData>( seeds ) );
			case IntVectorDataTypeId :
				return vectoriseSeededT<IntVectorData>( r, boost::static_pointer_cast<const IntVectorData>( seeds ) );
			case UIntVectorDataTypeId :
				return vectoriseSeededT<UIntVectorData>( r, boost::static_pointer_cast<const UIntVectorData>( seeds ) );
			default :
				throw Exception( "Unsupported type for seeds parameter." );
		}
		return 0;
	}

};

template<typename T, typename F>
void bindRand( const char *className )
{

	class_<T>( className )
		.def( init<unsigned long int>() )
		.def( "init", &T::init )
		.def( "nextb", &T::nextb )
		.def( "nexti", &T::nexti )
		.def( "nextf", (F (T::*)())&T::nextf )
		.def( "nextf", (F (T::*)( F, F ))&T::nextf )
		.def( "fVector",  &Vectoriser<T, FloatVectorData, &nextFloat<T> >::vectoriseSeeded )
		.def( "fVector",  &Vectoriser<T, FloatVectorData, &nextFloat<T> >::vectorise )
		.def( "nextV2f", &nextVec<V2f, T> )
		.def( "nextV3f", &nextVec<V3f, T> )
		.def( "nextV2d", &nextVec<V2d, T> )
		.def( "nextV3d", &nextVec<V3d, T> )
		.def( "v2fVector", &Vectoriser<T, V2fVectorData, &nextVec<V2f, T> >::vectoriseSeeded )
		.def( "v2fVector", &Vectoriser<T, V2fVectorData, &nextVec<V2f, T> >::vectorise )
		.def( "v2dVector", &Vectoriser<T, V2dVectorData, &nextVec<V2d, T> >::vectoriseSeeded )
		.def( "v2dVector", &Vectoriser<T, V2dVectorData, &nextVec<V2d, T> >::vectorise )
		.def( "v3fVector", &Vectoriser<T, V3fVectorData, &nextVec<V3f, T> >::vectoriseSeeded )
		.def( "v3fVector", &Vectoriser<T, V3fVectorData, &nextVec<V3f, T> >::vectorise )
		.def( "v3dVector", &Vectoriser<T, V3dVectorData, &nextVec<V3d, T> >::vectoriseSeeded )
		.def( "v3dVector", &Vectoriser<T, V3dVectorData, &nextVec<V3d, T> >::vectorise )
		.def( "nextColor3f", &nextVec<Color3f, T> )
		.def( "color3fVector", &Vectoriser<T, Color3fVectorData, &nextVec<Color3f, T> >::vectoriseSeeded )
		.def( "color3fVector", &Vectoriser<T, Color3fVectorData, &nextVec<Color3f, T> >::vectorise )
		.def( "gauss", &gaussRand<T> )
		.def( "gaussVector", &Vectoriser<T, FloatVectorData, &gaussRand<T> >::vectoriseSeeded )
		.def( "gaussVector", &Vectoriser<T, FloatVectorData, &gaussRand<T> >::vectorise )
		.def( "solidCirclef", &solidSphereRand<V2f,T> )
		.def( "solidCircled", &solidSphereRand<V2d,T> )
		.def( "solidCirclefVector", &Vectoriser<T, V2fVectorData, &solidSphereRand<V2f, T> >::vectoriseSeeded )
		.def( "solidCirclefVector", &Vectoriser<T, V2fVectorData, &solidSphereRand<V2f, T> >::vectorise )
		.def( "solidCircledVector", &Vectoriser<T, V2dVectorData, &solidSphereRand<V2d, T> >::vectoriseSeeded )
		.def( "solidCircledVector", &Vectoriser<T, V2dVectorData, &solidSphereRand<V2d, T> >::vectorise )
		.def( "solidSpheref", &solidSphereRand<V3f,T> )
		.def( "solidSphered", &solidSphereRand<V3d,T> )
		.def( "solidSpherefVector", &Vectoriser<T, V3fVectorData, &solidSphereRand<V3f, T> >::vectoriseSeeded )
		.def( "solidSpherefVector", &Vectoriser<T, V3fVectorData, &solidSphereRand<V3f, T> >::vectorise )
		.def( "solidSpheredVector", &Vectoriser<T, V3dVectorData, &solidSphereRand<V3d, T> >::vectoriseSeeded )
		.def( "solidSpheredVector", &Vectoriser<T, V3dVectorData, &solidSphereRand<V3d, T> >::vectorise )
		.def( "hollowCirclef", &hollowSphereRand<V2f,T> )
		.def( "hollowCircled", &hollowSphereRand<V2d,T> )
		.def( "hollowCirclefVector", &Vectoriser<T, V2fVectorData, &hollowSphereRand<V2f, T> >::vectoriseSeeded )
		.def( "hollowCirclefVector", &Vectoriser<T, V2fVectorData, &hollowSphereRand<V2f, T> >::vectorise )
		.def( "hollowCircledVector", &Vectoriser<T, V2dVectorData, &hollowSphereRand<V2d, T> >::vectoriseSeeded )
		.def( "hollowCircledVector", &Vectoriser<T, V2dVectorData, &hollowSphereRand<V2d, T> >::vectorise )
		.def( "hollowSpheref", &hollowSphereRand<V3f,T> )
		.def( "hollowSphered", &hollowSphereRand<V3d,T> )
		.def( "hollowSpherefVector", &Vectoriser<T, V3fVectorData, &hollowSphereRand<V3f, T> >::vectoriseSeeded )
		.def( "hollowSpherefVector", &Vectoriser<T, V3fVectorData, &hollowSphereRand<V3f, T> >::vectorise )
		.def( "hollowSpheredVector", &Vectoriser<T, V3dVectorData, &hollowSphereRand<V3d, T> >::vectoriseSeeded )
		.def( "hollowSpheredVector", &Vectoriser<T, V3dVectorData, &hollowSphereRand<V3d, T> >::vectorise )
		.def( "gaussCirclef", &gaussSphereRand<V2f,T> )
		.def( "gaussCircled", &gaussSphereRand<V2d,T> )
		.def( "gaussCirclefVector", &Vectoriser<T, V2fVectorData, &gaussSphereRand<V2f, T> >::vectoriseSeeded )
		.def( "gaussCirclefVector", &Vectoriser<T, V2fVectorData, &gaussSphereRand<V2f, T> >::vectorise )
		.def( "gaussCircledVector", &Vectoriser<T, V2dVectorData, &gaussSphereRand<V2d, T> >::vectoriseSeeded )
		.def( "gaussCircledVector", &Vectoriser<T, V2dVectorData, &gaussSphereRand<V2d, T> >::vectorise )
		.def( "gaussSpheref", &gaussSphereRand<V3f,T> )
		.def( "gaussSphered", &gaussSphereRand<V3d,T> )
		.def( "gaussSpherefVector", &Vectoriser<T, V3fVectorData, &gaussSphereRand<V3f, T> >::vectoriseSeeded )
		.def( "gaussSpherefVector", &Vectoriser<T, V3fVectorData, &gaussSphereRand<V3f, T> >::vectorise )
		.def( "gaussSpheredVector", &Vectoriser<T, V3dVectorData, &gaussSphereRand<V3d, T> >::vectoriseSeeded )
		.def( "gaussSpheredVector", &Vectoriser<T, V3dVectorData, &gaussSphereRand<V3d, T> >::vectorise )
		.def( "cosineHemispheref", &cosineHemisphereRand<V3f,T> )
		.def( "cosineHemisphered", &cosineHemisphereRand<V3d,T> )
		.def( "cosineHemispherefVector", &Vectoriser<T, V3fVectorData, &cosineHemisphereRand<V3f, T> >::vectoriseSeeded )
		.def( "cosineHemispherefVector", &Vectoriser<T, V3fVectorData, &cosineHemisphereRand<V3f, T> >::vectorise )
		.def( "cosineHemispheredVector", &Vectoriser<T, V3dVectorData, &cosineHemisphereRand<V3d, T> >::vectoriseSeeded )
		.def( "cosineHemispheredVector", &Vectoriser<T, V3dVectorData, &cosineHemisphereRand<V3d, T> >::vectorise )
		.def( "barycentricf", &barycentricRand<V3f,T> )
		.def( "barycentricd", &barycentricRand<V3d,T> )
		.def( "barycentricfVector", &Vectoriser<T, V3fVectorData, &barycentricRand<V3f, T> >::vectoriseSeeded )
		.def( "barycentricfVector", &Vectoriser<T, V3fVectorData, &barycentricRand<V3f, T> >::vectorise )
		.def( "barycentricdVector", &Vectoriser<T, V3dVectorData, &barycentricRand<V3d, T> >::vectoriseSeeded )
		.def( "barycentricdVector", &Vectoriser<T, V3dVectorData, &barycentricRand<V3d, T> >::vectorise )
	;

}

void bindImathRandom()
{
	bindRand<Rand32, float>( "Rand32" );
	bindRand<Rand48, double>( "Rand48" );
}

} // namespace IECorePython
