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

/// \todo Simple types should handle True/False tests based on their content. Ex.: IntData(0) should be False, IntData(3) should be True. Types like IntVectorData already handle such tests that way (IntVectorData([]) is False and other values are True).

#include "boost/python.hpp"

#include <limits.h>
#include <sstream>

#include "boost/python/make_constructor.hpp"

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/halfLimits.h"

#include "IECore/SimpleTypedData.h"
#include "IECorePython/GeometricTypedDataBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/IECoreBinding.h"
#include "IECorePython/SimpleTypedDataBinding.h"

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;
using namespace IECore;

namespace IECorePython
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// functions used by the bindings
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
struct ConstructHelper
{
	static typename T::Ptr construct()
	{
		return new T;
	}
};

/// Half needs explicit initialisation
template<>
struct ConstructHelper<HalfData>
{
	static HalfDataPtr construct()
	{
		return new HalfData( 0 );
	}
};

template<class T>
static typename T::Ptr constructWithValue( const typename T::ValueType &v )
{
	return new T( v );
}

template<class T>
static typename T::Ptr constructWithValueAndInterpretation( const typename T::ValueType &v, IECore::GeometricData::Interpretation i )
{
	return new T( v, i );
}

template<class T>
static void setValue( T &that, const typename T::ValueType &v )
{
	that.writable() = v;
}

template<class T>
static typename T::ValueType getValue( T &that )
{
	return that.readable();
}

template<class T>
static int cmp( T &x, T &y )
{
	const typename T::ValueType &xData = x.readable();
	const typename T::ValueType &yData = y.readable();
	if( xData < yData )
	{
		return -1;
	}
	if( xData > yData )
	{
		return 1;
	}
	return 0;
}

template<>
int cmp( StringData &x, StringData &y )
{
	return x.readable().compare( y.readable() );
}

template<>
string str( char &x )
{
	stringstream s;
	s << (int)x;
	return s.str();
}

template<>
string str( unsigned char &x )
{
	stringstream s;
	s << (int)x;
	return s.str();
}

template<>
string repr( char &x )
{
	stringstream s;
	s << (int)x;
	return s.str();
}

template<>
string repr( unsigned char &x )
{
	stringstream s;
	s << (int)x;
	return s.str();
}

template<>
string str( std::string &x )
{
	return x;
}

template<>
string repr( std::string &x )
{
	return "\"" + x + "\"";
}

template<>
string str( InternedString &x )
{
	return x.value();
}

template<>
string repr( InternedString &x )
{
	return "\"" + x.value() + "\"";
}

#define DEFINENUMERICSTRSPECIALISATION( TYPE )															\
template<>																								\
string repr<TYPE>( TYPE &x )																			\
{																										\
	stringstream s;																						\
	s << x;																								\
	return s.str();																						\
}																										\
																										\
template<>																								\
string str<TYPE>( TYPE &x )																				\
{																										\
	stringstream s;																						\
	s << x;																								\
	return s.str();																						\
}

DEFINENUMERICSTRSPECIALISATION( bool );
DEFINENUMERICSTRSPECIALISATION( int );
DEFINENUMERICSTRSPECIALISATION( unsigned int );
DEFINENUMERICSTRSPECIALISATION( float );
DEFINENUMERICSTRSPECIALISATION( double );
DEFINENUMERICSTRSPECIALISATION( half );
DEFINENUMERICSTRSPECIALISATION( short );
DEFINENUMERICSTRSPECIALISATION( unsigned short );
DEFINENUMERICSTRSPECIALISATION( int64_t );
DEFINENUMERICSTRSPECIALISATION( uint64_t );

#define DEFINETYPEDDATASTRSPECIALISATION( TYPE )														\
template<>																								\
string repr<TypedData<TYPE> >( TypedData<TYPE> &x )														\
{																										\
	stringstream s;																						\
	s << "IECore." << x.typeName() << "( " << repr( const_cast<TYPE &>( x.readable() ) ) << " )";					\
	return s.str();																						\
}																										\
																										\
template<>																								\
string str<TypedData<TYPE> >( TypedData<TYPE> &x )														\
{																										\
	stringstream s;																						\
	s << str( const_cast<TYPE &>( x.readable() ) );														\
	return s.str();																						\
}

DEFINETYPEDDATASTRSPECIALISATION( bool );
DEFINETYPEDDATASTRSPECIALISATION( char );
DEFINETYPEDDATASTRSPECIALISATION( unsigned char );
DEFINETYPEDDATASTRSPECIALISATION( int );
DEFINETYPEDDATASTRSPECIALISATION( unsigned int );
DEFINETYPEDDATASTRSPECIALISATION( float );
DEFINETYPEDDATASTRSPECIALISATION( double );
DEFINETYPEDDATASTRSPECIALISATION( short );
DEFINETYPEDDATASTRSPECIALISATION( unsigned short );
DEFINETYPEDDATASTRSPECIALISATION( int64_t );
DEFINETYPEDDATASTRSPECIALISATION( uint64_t );
DEFINETYPEDDATASTRSPECIALISATION( string );
DEFINETYPEDDATASTRSPECIALISATION( InternedString );
DEFINETYPEDDATASTRSPECIALISATION( half );

IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V2i );
IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V2f );
IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V2d );
IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V3i );
IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V3f );
IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( V3d );

DEFINETYPEDDATASTRSPECIALISATION( Box2i );
DEFINETYPEDDATASTRSPECIALISATION( Box2f );
DEFINETYPEDDATASTRSPECIALISATION( Box2d );
DEFINETYPEDDATASTRSPECIALISATION( Box3i );
DEFINETYPEDDATASTRSPECIALISATION( Box3f );
DEFINETYPEDDATASTRSPECIALISATION( Box3d );
DEFINETYPEDDATASTRSPECIALISATION( Color3f );
DEFINETYPEDDATASTRSPECIALISATION( Color4f );
DEFINETYPEDDATASTRSPECIALISATION( Color3<double> );
DEFINETYPEDDATASTRSPECIALISATION( Color4<double> );
DEFINETYPEDDATASTRSPECIALISATION( M33f );
DEFINETYPEDDATASTRSPECIALISATION( M33d );
DEFINETYPEDDATASTRSPECIALISATION( M44f );
DEFINETYPEDDATASTRSPECIALISATION( M44d );
DEFINETYPEDDATASTRSPECIALISATION( Quatf );
DEFINETYPEDDATASTRSPECIALISATION( Quatd );
DEFINETYPEDDATASTRSPECIALISATION( LineSegment3f );
DEFINETYPEDDATASTRSPECIALISATION( LineSegment3d );

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// functions to do the binding
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
static RunTimeTypedClass<T> bindSimpleData()
{
	TypedDataFromType<T>();

	RunTimeTypedClass<T> result;
	result.def( "__init__", make_constructor( &ConstructHelper<T>::construct ), "Construct with no specified value." );
	result.def( "__init__", make_constructor( &constructWithValue<T> ), "Construct with the specified value." );
	result.def( "__str__", &str<T> );
	result.def( "__repr__", &repr<T> );
	result.def( "hasBase", &T::hasBase ).staticmethod( "hasBase" );
	result.add_property( "value",	&getValue<T>,
									&setValue<T>, "The value contained by the object.");


	return result;
}

template<class T>
static RunTimeTypedClass<GeometricTypedData<T > > bindSimpleGeometricData()
{
	typedef TypedData<T> ParentClass;
	typedef GeometricTypedData<T> ThisClass;
	
	RunTimeTypedClass<ParentClass>();
	
	RunTimeTypedClass<ThisClass> result = bindSimpleData<ThisClass>();
	result.def( "__init__", make_constructor( &constructWithValueAndInterpretation<ThisClass> ), "Construct with the specified value and interpretation." );
	result.def("getInterpretation", &ThisClass::getInterpretation, "Returns the geometric interpretation of this data.");
	result.def("setInterpretation", &ThisClass::setInterpretation, "Sets the geometric interpretation of this data.");
	
	return result;
}

template<class T>
static void bindNumericMethods( class_<T, typename T::Ptr, boost::noncopyable, bases<Data> > &c )
{
	c.add_static_property( "minValue", &std::numeric_limits<typename T::ValueType>::min, "Minimum representable value." );
	c.add_static_property( "maxValue", &std::numeric_limits<typename T::ValueType>::max, "Maximum representable value." );
	c.def( "__cmp__", &cmp<T>, "Comparison operators ( <, >, >=, <= )" );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the one function exposed to the outside world
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bindAllSimpleTypedData()
{
	RunTimeTypedClass<StringData> sdc = bindSimpleData<StringData>();
	sdc.def("__cmp__", &cmp<StringData> );
	
	bindSimpleData<InternedStringData>();

	bindSimpleData<BoolData>();

	RunTimeTypedClass<IntData> idc = bindSimpleData<IntData>();
	bindNumericMethods( idc );
	idc.def( "__int__", &getValue<IntData> );

	RunTimeTypedClass<UIntData> uidc = bindSimpleData<UIntData>();
	bindNumericMethods( uidc );
	uidc.def( "__long__", &getValue<UIntData> );

	RunTimeTypedClass<FloatData > fdc = bindSimpleData<FloatData>();
	bindNumericMethods( fdc );
	fdc.def( "__float__", &getValue<FloatData> );

	RunTimeTypedClass<DoubleData> ddc = bindSimpleData<DoubleData>();
	bindNumericMethods( ddc );
	ddc.def( "__float__", &getValue<DoubleData> );

	RunTimeTypedClass<CharData> cdc = bindSimpleData<CharData>();
	bindNumericMethods( cdc );

	RunTimeTypedClass<UCharData> ucdc = bindSimpleData<UCharData>();
	bindNumericMethods( ucdc );
	ucdc.def( "__int__", &getValue<UCharData> );
	ucdc.def( "__chr__", &getValue<UCharData> );

	RunTimeTypedClass<HalfData> hdc = bindSimpleData<HalfData>();
	bindNumericMethods( hdc );
	hdc.def( "__float__", &getValue<HalfData> );

	RunTimeTypedClass<ShortData> shdc = bindSimpleData<ShortData>();
	bindNumericMethods( shdc );
	shdc.def( "__int__", &getValue<ShortData> );

	RunTimeTypedClass<UShortData> ushdc = bindSimpleData<UShortData>();
	bindNumericMethods( ushdc );
	ushdc.def( "__int__", &getValue<UShortData> );

	RunTimeTypedClass<Int64Data> i64dc = bindSimpleData<Int64Data>();
	bindNumericMethods( i64dc );
	i64dc.def( "__long__", &getValue<Int64Data> );

	RunTimeTypedClass<UInt64Data> ui64dc = bindSimpleData<UInt64Data>();
	bindNumericMethods( ui64dc );
	ui64dc.def( "__long__", &getValue<UInt64Data> );
	
	bindSimpleGeometricData<V2i>();
	bindSimpleGeometricData<V3i>();
	bindSimpleGeometricData<V2f>();
	bindSimpleGeometricData<V3f>();
	bindSimpleGeometricData<V2d>();
	bindSimpleGeometricData<V3d>();

	bindSimpleData<Box2iData>();

	bindSimpleData<Box3iData>();

	bindSimpleData<Box2fData>();

	bindSimpleData<Box3fData>();

	bindSimpleData<Box2dData>();

	bindSimpleData<Box3dData>();

	bindSimpleData<M33fData>();

	bindSimpleData<M33dData>();

	bindSimpleData<M44fData>();

	bindSimpleData<M44dData>();

	bindSimpleData<QuatfData>();

	bindSimpleData<QuatdData>();

	bindSimpleData<Color3fData>();

	bindSimpleData<Color3dData>();

	bindSimpleData<Color4fData>();

	bindSimpleData<Color4dData>();

	bindSimpleData<LineSegment3fData>();
	bindSimpleData<LineSegment3dData>();

}

} // namespace IECorePython
