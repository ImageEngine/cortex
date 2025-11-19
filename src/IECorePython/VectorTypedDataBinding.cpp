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

#include "IECorePython/VectorTypedDataBinding.h"

#include "IECorePython/ImathBoxVectorBinding.h"
#include "IECorePython/ImathColorVectorBinding.h"
#include "IECorePython/ImathMatrixVectorBinding.h"
#include "IECorePython/ImathQuatVectorBinding.h"
#include "IECorePython/ImathVecVectorBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/VectorTypedDataBinding.inl"

#include "IECore/DataAlgo.h"
#include "IECore/Export.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathBox.h"
#include "Imath/ImathQuat.h"
#include "Imath/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/numeric/conversion/cast.hpp"
#include "boost/python/implicit.hpp"
#include "boost/python/make_constructor.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;
using namespace IECore;

namespace
{

template<typename T>
struct PythonType
{
	static const char *value();
};

template<> struct PythonType<half>{ static const char *value(){ return "e"; } };
template<> struct PythonType<float>{ static const char *value(){ return "f"; } };
template<> struct PythonType<double>{ static const char *value(){ return "d"; } };
template<> struct PythonType<int>{ static const char *value(){ return "i"; } };
template<> struct PythonType<unsigned int>{ static const char *value(){ return "I"; } };
template<> struct PythonType<char>{ static const char *value(){ return "b"; } };
template<> struct PythonType<unsigned char>{ static const char *value(){ return "B"; } };
template<> struct PythonType<short>{ static const char *value(){ return "h"; } };
template<> struct PythonType<unsigned short>{ static const char *value(){ return "H"; } };
template<> struct PythonType<int64_t>{ static const char *value(){ return "q"; } };
template<> struct PythonType<uint64_t>{ static const char *value(){ return "Q"; } };

}  // namespace

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
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( int64_t )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( uint64_t )
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

Buffer::Buffer( Data *data, const bool writable ) : m_data( data->copy() ), m_writable( writable )
{

}

Buffer::~Buffer()
{

}

DataPtr Buffer::asData() const
{
	return m_data->copy();
}

bool Buffer::isWritable() const
{
	return m_writable;
}

int Buffer::getBuffer( PyObject *object, Py_buffer *view, int flags )
{
	// This method is a variation on Python's `PyBuffer_FillInfo`.
	if( view == NULL ) {
		PyErr_SetString( PyExc_ValueError, "getBuffer(): view==NULL argument is obsolete" );
		return -1;
	}

	if( flags != PyBUF_SIMPLE )
	{
		if( flags == PyBUF_READ || flags == PyBUF_WRITE )
		{
			PyErr_BadInternalCall();
			return -1;
		}
	}

	IECorePython::BufferPtr self = boost::python::extract<IECorePython::BufferPtr>( object );
	if( !self )
	{
		PyErr_SetString( PyExc_ValueError, "getBuffer(): Could not extract `BufferPtr` from Python object." );
		return -1;
	}

	if( ( flags | PyBUF_WRITABLE ) == PyBUF_WRITABLE && !self->isWritable() )
	{
		return -1;
	}

	try
	{
		dispatch(
			self->m_data.get(),
			[&flags, &view, &object, &self]( auto *bufferData ) -> void
			{
				using DataType = typename std::remove_const_t< std::remove_pointer_t< decltype( bufferData ) > >;

				if constexpr( TypeTraits::HasBaseType<DataType>::value && TypeTraits::IsNumericBasedVectorTypedData<DataType>::value )
				{
					using ElementType = typename DataType::ValueType::value_type;
					if constexpr( std::is_arithmetic_v<ElementType> )
					{
						view->obj = Py_XNewRef( object );
						view->readonly = !self->isWritable();
						view->buf = view->readonly ? (void*)bufferData->baseReadable() : (void*)bufferData->baseWritable();
						view->len = bufferData->readable().size() * sizeof( ElementType );
						view->itemsize = sizeof( ElementType );
						view->format = ( ( flags & PyBUF_FORMAT ) == PyBUF_FORMAT ) ? const_cast<char *>( PythonType<ElementType>::value() ) : NULL;
						view->ndim = 1;
						view->internal = NULL;
						view->shape = ( ( flags & PyBUF_ND ) == PyBUF_ND ) ? (Py_ssize_t*)( new Py_ssize_t( bufferData->readable().size() ) ) : NULL;
						view->strides = ( ( flags & PyBUF_STRIDES ) == PyBUF_STRIDES ) ? &( view->itemsize ) : NULL;
						view->suboffsets = NULL;
					}
				}
			}
		);
	}
	catch( Exception &e )
	{
		view->obj = NULL;
		PyErr_SetString( PyExc_BufferError, e.what() );
		return -1;
	}

	return 0;
}

void Buffer::releaseBuffer( PyObject *object, Py_buffer *view )
{
	if( view->shape != NULL )
	{
		delete view->shape;
	}
	// Python takes care of decrementing `object`
}

static PyBufferProcs BufferProtocol = {
  (getbufferproc)Buffer::getBuffer,
  (releasebufferproc)Buffer::releaseBuffer,
};

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
		int64_t,
		"int64_t")

	BIND_FULL_OPERATED_VECTOR_TYPEDDATA(
		uint64_t,
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

	auto c = RefCountedClass<Buffer, IECore::RefCounted>( "Buffer" )
		.def( init<Data *, bool>() )
		.def( "asData", &Buffer::asData )
		.def( "isWritable", &Buffer::isWritable )
	;
	PyTypeObject *o = (PyTypeObject *)c.ptr();
	o->tp_as_buffer = &BufferProtocol;
	PyType_Modified( o );
	
}


} // namespace IECorePython
