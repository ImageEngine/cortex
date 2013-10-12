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

// System includes

// External includes
#include "boost/python.hpp"
#include "boost/python/make_constructor.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"
#include "boost/numeric/conversion/cast.hpp"
#include "boost/python/implicit.hpp"

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"

#include "IECore/VectorTypedData.h"
#include "IECorePython/ImathMatrixVectorBinding.h"
#include "IECorePython/ImathVecVectorBinding.h"
#include "IECorePython/ImathColorVectorBinding.h"
#include "IECorePython/ImathBoxVectorBinding.h"
#include "IECorePython/ImathQuatVectorBinding.h"
#include "IECorePython/VectorTypedDataBinding.inl"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;
using namespace IECore;

namespace IECorePython
{

IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( half )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( float )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( double )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( int )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( unsigned int )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( char )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( unsigned char )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( short )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( unsigned short )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( ::int64_t )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( ::uint64_t )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( std::string )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( InternedString )

// we have to specialise the repr() and str() separately here, because of
// the whole vector<bool> is not a container thing.
template<>
std::string repr<BoolVectorData>( BoolVectorData &x )
{
	std::stringstream s;
	s << "IECore." << x.typeName() << "( [ ";
	const std::vector<bool> &xd = x.readable();
	for( size_t i=0; i<xd.size(); i++ )
	{
		bool b = xd[i];
		s << repr( b );
		if( i!=xd.size()-1 )
		{
			s << ", ";
		}
	}
	s<< " ] )";
	return s.str();
}


template<>
std::string str<BoolVectorData>( BoolVectorData &x )
{
	std::stringstream s;
	const std::vector<bool> &xd = x.readable();
	for( size_t i=0; i<xd.size(); i++ )
	{
		bool b = xd[i];
		s << str( b );
		if( i!=xd.size()-1 )
		{
			s << " ";
		}
	}
	return s.str();
}

void bindAllVectorTypedData()
{
	// basic types
	BIND_VECTOR_TYPEDDATA(
		bool,
		"bool")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		half,
		"half")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		float,
		"float")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		double,
		"double")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		int,
		"int")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		unsigned int,
		"unsigned int")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		char,
		"char")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		unsigned char,
		"unsigned char")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		short,
		"short")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		unsigned short,
		"unsigned short")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		::int64_t,
		"int64_t")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		::uint64_t,
		"uint64_t")

	BIND_VECTOR_TYPEDDATA (
		std::string,
		"string")
		
	BIND_VECTOR_TYPEDDATA (
		InternedString,
		"InternedString")	

	// Imath types
	bindImathMatrixVectorTypedData();
	bindImathVecVectorTypedData();
	bindImathColorVectorTypedData();
	bindImathBoxVectorTypedData();
	bindImathQuatVectorTypedData();
}


} // namespace IECorePython
