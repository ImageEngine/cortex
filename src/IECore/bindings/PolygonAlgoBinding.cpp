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

#include <boost/python.hpp>

#include "IECore/bindings/PolygonAlgoBinding.h"
#include "IECore/PolygonAlgo.h"
#include "IECore/VectorTypedData.h"

using namespace boost::python;
using namespace Imath;

namespace IECore
{

template<typename VecData>
static typename VecData::ValueType::value_type polygonNormalBinding( typename VecData::Ptr p )
{
	return polygonNormal( p->readable().begin(), p->readable().end() );
}

template<typename VecData>
static Winding polygonWindingBinding2D( typename VecData::Ptr p )
{
	return polygonWinding( p->readable().begin(), p->readable().end() );
}

template<typename VecData>
static Winding polygonWindingBinding3D( typename VecData::Ptr p, const typename VecData::ValueType::value_type &v )
{
	return polygonWinding( p->readable().begin(), p->readable().end(), v );
}

template<typename VecData>
static Box<typename VecData::ValueType::value_type> polygonBoundBinding( typename VecData::Ptr p )
{
	return polygonBound( p->readable().begin(), p->readable().end() );
}

void bindPolygonAlgo()
{
	enum_<Winding>( "Winding" )
		.value( "Clockwise", ClockwiseWinding )
		.value( "CounterClockwise", CounterClockwiseWinding )
	;

	def( "polygonNormal", &polygonNormalBinding<V3fVectorData> );
	def( "polygonNormal", &polygonNormalBinding<V3dVectorData> );

	def( "polygonWinding", &polygonWindingBinding2D<V2fVectorData> );
	def( "polygonWinding", &polygonWindingBinding2D<V2dVectorData> );

	def( "polygonWinding", &polygonWindingBinding3D<V3fVectorData> );
	def( "polygonWinding", &polygonWindingBinding3D<V3dVectorData> );
	
	def( "polygonBound", &polygonBoundBinding<V2fVectorData> );
	def( "polygonBound", &polygonBoundBinding<V3fVectorData> );
	def( "polygonBound", &polygonBoundBinding<V2dVectorData> );
	def( "polygonBound", &polygonBoundBinding<V3dVectorData> );
}

} // namespace IECore
