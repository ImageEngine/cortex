//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include <limits.h>

#include "boost/python/make_constructor.hpp"

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/halfLimits.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;

namespace IECore
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
DEFINETYPEDDATASTRSPECIALISATION( half );
DEFINETYPEDDATASTRSPECIALISATION( V2i );
DEFINETYPEDDATASTRSPECIALISATION( V2f );
DEFINETYPEDDATASTRSPECIALISATION( V2d );
DEFINETYPEDDATASTRSPECIALISATION( V3i );
DEFINETYPEDDATASTRSPECIALISATION( V3f );
DEFINETYPEDDATASTRSPECIALISATION( V3d );
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// an rvalue converter to get TypedData<T> from a python object convertible to T
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct TypedDataFromType
{
	TypedDataFromType()
	{
		converter::registry::push_back(
			&convertible,
			&construct,
			type_id<typename T::Ptr>()
		);

	}

	static void *convertible( PyObject *obj )
	{
		extract<typename T::ValueType> e( obj );
		if( e.check() )
		{
			return obj;
		}
		return 0;
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((converter::rvalue_from_python_storage<typename T::Ptr>*)data)->storage.bytes;
		new (storage) typename T::Ptr( new T( extract<typename T::ValueType>( obj ) ) );
		data->convertible = storage;
	}
};

// specialise the bool version so it doesn't go gobbling up ints and things and turning them
// into BoolData
template<>
struct TypedDataFromType<BoolData>
{

	TypedDataFromType()
	{
		converter::registry::push_back(
			&convertible,
			&construct,
			type_id<BoolDataPtr>()
		);

	}

	static void *convertible( PyObject *obj )
	{
		if( PyBool_Check( obj ) )
		{
			return obj;
		}
		return 0;
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((converter::rvalue_from_python_storage<BoolDataPtr>*)data)->storage.bytes;
		new (storage) BoolDataPtr( new BoolData( extract<bool>( obj ) ) );
		data->convertible = storage;
	}

};

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
	result.add_property( "value",	&getValue<T>,
									&setValue<T>, "The value contained by the object.");


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

	bindSimpleData<V2iData>();

	bindSimpleData<V3iData>();

	bindSimpleData<V2fData>();

	bindSimpleData<V3fData>();

	bindSimpleData<V2dData>();

	bindSimpleData<V3dData>();

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

}

} // namespace IECore
