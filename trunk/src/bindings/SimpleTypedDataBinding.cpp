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

#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>

#include "OpenEXR/ImathLimits.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/BoxOperators.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
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
static intrusive_ptr<T> construct()
{
	return new T;
}

template<class T>
static intrusive_ptr<T> constructWithValue( const typename T::ValueType &v )
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
static int cmp( StringData &x, StringData &y )
{
	return x.readable().compare( y.readable() );
}

template<class T>
static typename T::ValueType minValue( T &x )
{
	return Imath::limits<typename T::ValueType>::min();
}

template<class T>
static typename T::ValueType maxValue( T & x)
{
	return Imath::limits<typename T::ValueType>::max();
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
DEFINENUMERICSTRSPECIALISATION( long );
DEFINENUMERICSTRSPECIALISATION( int );
DEFINENUMERICSTRSPECIALISATION( unsigned int );
DEFINENUMERICSTRSPECIALISATION( float );
DEFINENUMERICSTRSPECIALISATION( double );
DEFINENUMERICSTRSPECIALISATION( half );

#define DEFINETYPEDDATASTRSPECIALISATION( TYPE )														\
template<>																								\
string repr<TypedData<TYPE> >( TypedData<TYPE> &x )														\
{																										\
	stringstream s;																						\
	s << x.typeName() << "( " << repr( const_cast<TYPE &>( x.readable() ) ) << " )";					\
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
DEFINETYPEDDATASTRSPECIALISATION( long );
DEFINETYPEDDATASTRSPECIALISATION( int );
DEFINETYPEDDATASTRSPECIALISATION( unsigned int );
DEFINETYPEDDATASTRSPECIALISATION( float );
DEFINETYPEDDATASTRSPECIALISATION( double );
DEFINETYPEDDATASTRSPECIALISATION( string );
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
// functions to do the binding
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
static class_<T, intrusive_ptr<T>, boost::noncopyable, bases<Data> > bindSimpleData()
{
	string typeName = T::staticTypeName();
	typedef class_<T, intrusive_ptr<T>, boost::noncopyable, bases<Data> > ThisPyClass;
	ThisPyClass result( typeName.c_str(), no_init );
	result.def( "__init__", make_constructor( &construct<T> ), "Construct with no specified value." );
	result.def( "__init__", make_constructor( &constructWithValue<T> ), "Construct with the specified value." );
	result.def( "__str__", &str<T> );
	result.def( "__repr__", &repr<T> );
	result.add_property( "value",	&getValue<T>,
									&setValue<T>, "The value contained by the object.");
	result.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(T);									
	INTRUSIVE_PTR_PATCH( T, typename ThisPyClass );
	return result;
}

template<class T>
static void bindNumericMethods( class_<T, intrusive_ptr<T>, boost::noncopyable, bases<Data> > &c )
{
	c.add_property( "minValue", &minValue<T>, "Minimum representable value." );
	c.add_property( "maxValue", &maxValue<T>, "Maximum representable value." );
	c.def( "__cmp__", &cmp<T>, "Comparison operators ( <, >, >=, <= )" );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the one function exposed to the outside world
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bindAllSimpleTypedData()
{
	class_< StringData, StringDataPtr, boost::noncopyable, bases<Data> > sdc = bindSimpleData<StringData>();
	sdc.def("__cmp__", &cmp<StringData> );
	implicitly_convertible<StringDataPtr, DataPtr>();

	bindSimpleData<BoolData>();
	implicitly_convertible<BoolDataPtr, DataPtr>();
	
	class_< IntData, IntDataPtr, boost::noncopyable, bases<Data> > idc = bindSimpleData<IntData>();
	bindNumericMethods( idc );
	idc.def( "__int__", &getValue<IntData> );
	implicitly_convertible<IntDataPtr, DataPtr>();

	class_< LongData, LongDataPtr, boost::noncopyable, bases<Data> > ldc = bindSimpleData<LongData>();
	bindNumericMethods( ldc );
	ldc.def( "__long__", &getValue<LongData> );
	implicitly_convertible<LongDataPtr, DataPtr>();
	
	class_< UIntData, UIntDataPtr, boost::noncopyable, bases<Data> > uidc = bindSimpleData<UIntData>();
	bindNumericMethods( uidc );
	uidc.def( "__long__", &getValue<UIntData> );
	implicitly_convertible<UIntDataPtr, DataPtr>();
	
	class_< FloatData, FloatDataPtr, boost::noncopyable, bases<Data> > fdc = bindSimpleData<FloatData>();
	bindNumericMethods( fdc );
	fdc.def( "__float__", &getValue<FloatData> );
	implicitly_convertible<FloatDataPtr, DataPtr>();
	
	class_< DoubleData, DoubleDataPtr, boost::noncopyable, bases<Data> > ddc = bindSimpleData<DoubleData>();
	bindNumericMethods( ddc );
	ddc.def( "__float__", &getValue<DoubleData> );
	implicitly_convertible<DoubleDataPtr, DataPtr>();
	
	class_< CharData, CharDataPtr, boost::noncopyable, bases<Data> > cdc = bindSimpleData<CharData>();
	bindNumericMethods( cdc );
	implicitly_convertible<CharDataPtr, DataPtr>();
	
	class_< UCharData, UCharDataPtr, boost::noncopyable, bases<Data> > ucdc = bindSimpleData<UCharData>();
	bindNumericMethods( ucdc );
	ucdc.def( "__int__", &getValue<UCharData> );
	ucdc.def( "__chr__", &getValue<UCharData> );
	implicitly_convertible<UCharDataPtr, DataPtr>();

	bindSimpleData<V2iData>();
	implicitly_convertible<V2iDataPtr, DataPtr>();

	bindSimpleData<V3iData>();
	implicitly_convertible<V3iDataPtr, DataPtr>();

	bindSimpleData<V2fData>();
	implicitly_convertible<V2fDataPtr, DataPtr>();
	
	bindSimpleData<V3fData>();
	implicitly_convertible<V3fDataPtr, DataPtr>();
	
	bindSimpleData<V2dData>();
	implicitly_convertible<V2dDataPtr, DataPtr>();
	
	bindSimpleData<V3dData>();
	implicitly_convertible<V3dDataPtr, DataPtr>();

	bindSimpleData<Box2iData>();
	implicitly_convertible<Box2iDataPtr, DataPtr>();
	
	bindSimpleData<Box3iData>();
	implicitly_convertible<Box3iDataPtr, DataPtr>();
	
	bindSimpleData<Box2fData>();
	implicitly_convertible<Box2fDataPtr, DataPtr>();
	
	bindSimpleData<Box3fData>();
	implicitly_convertible<Box3fDataPtr, DataPtr>();
	
	bindSimpleData<Box2dData>();
	implicitly_convertible<Box2dDataPtr, DataPtr>();
	
	bindSimpleData<Box3dData>();
	implicitly_convertible<Box3dDataPtr, DataPtr>();

	bindSimpleData<M33fData>();
	implicitly_convertible<M33fDataPtr, DataPtr>();
	
	bindSimpleData<M33dData>();
	implicitly_convertible<M33dDataPtr, DataPtr>();
	
	bindSimpleData<M44fData>();
	implicitly_convertible<M44fDataPtr, DataPtr>();
	
	bindSimpleData<M44dData>();
	implicitly_convertible<M44dDataPtr, DataPtr>();
	
	bindSimpleData<QuatfData>();
	implicitly_convertible<QuatfDataPtr, DataPtr>();
	
	bindSimpleData<QuatdData>();
	implicitly_convertible<QuatdDataPtr, DataPtr>();
	
	bindSimpleData<Color3fData>();
	implicitly_convertible<Color3fDataPtr, DataPtr>();
	
	bindSimpleData<Color3dData>();
	implicitly_convertible<Color3dDataPtr, DataPtr>();
	
	bindSimpleData<Color4fData>();
	implicitly_convertible<Color4fDataPtr, DataPtr>();
	
	bindSimpleData<Color4dData>();
	implicitly_convertible<Color4dDataPtr, DataPtr>();

}

} // namespace IECore
