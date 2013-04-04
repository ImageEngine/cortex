//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#ifndef IECOREPYTHON_GEOMETRICTYPEDDATABINDING_INL
#define IECOREPYTHON_GEOMETRICTYPEDDATABINDING_INL

#include "IECore/GeometricTypedData.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include <sstream>

namespace IECorePython
{

#define IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( TYPE ) \
template<> \
string repr<GeometricTypedData<TYPE> >( GeometricTypedData<TYPE> &x ) \
{ \
	stringstream s; \
	s << "IECore." << x.typeName() << "( " << repr( const_cast<TYPE &>( x.readable() ) ) << " )"; \
	return s.str(); \
} \
\
template<> \
string str<GeometricTypedData<TYPE> >( GeometricTypedData<TYPE> &x ) \
{ \
	stringstream s; \
	s << str( const_cast<TYPE &>( x.readable() ) ); \
	return s.str(); \
} \

#define IECOREPYTHON_DEFINEGEOMETRICVECTORDATASTRSPECIALISATION( TYPE ) \
template<> \
std::string repr<GeometricTypedData<std::vector<TYPE> > >( GeometricTypedData<std::vector<TYPE> > &x ) \
{ \
	std::stringstream s; \
	s << "IECore." << x.typeName() << "( [ "; \
	const GeometricTypedData<std::vector<TYPE> >::ValueType &xd = x.readable(); \
	for( size_t i=0; i<xd.size(); i++ ) \
	{ \
		s << repr( const_cast<TYPE&>( xd[i] ) ); \
		if( i!=xd.size()-1 ) \
		{ \
			s << ", "; \
		} \
	} \
	s<< " ] )"; \
	return s.str(); \
} \
\
template<> \
std::string str<GeometricTypedData<std::vector<TYPE> > >( GeometricTypedData<std::vector<TYPE> > &x ) \
{ \
	std::stringstream s; \
	const GeometricTypedData<std::vector<TYPE> >::ValueType &xd = x.readable(); \
	for( size_t i=0; i<xd.size(); i++ ) \
	{ \
		s << str( const_cast<TYPE &>( xd[i] ) ); \
		if( i!=xd.size()-1 ) \
		{ \
			s << " "; \
		} \
	} \
	return s.str(); \
} \

template<typename ThisClass, typename ThisBinder>
class GeometricVectorTypedDataFunctions
{
	public :
		typedef typename ThisBinder::ThisClassPtr ThisClassPtr;

		/// constructor that receives a python list object and an interpretation
		static ThisClassPtr
		dataListOrSizeConstructorAndInterpretation( boost::python::object v, IECore::GeometricData::Interpretation i )
		{
			ThisClassPtr r = ThisBinder::dataListOrSizeConstructor( v );
			r->setInterpretation( i );
			return r;
		}
};

// bind a VectorTypedData class that supports all Math operators (+=, -=, *=, /=)
/// \todo: would be nice if we didn't have to re-define all these operators just to
/// change the macro call and add a couple new methods.
#define BIND_OPERATED_GEOMETRIC_VECTOR_TYPEDDATA(T, Tname) \
		{ \
			typedef GeometricTypedData< std::vector< T > > ThisClass; \
			typedef VectorTypedDataFunctions< ThisClass > ThisBinder; \
			typedef GeometricVectorTypedDataFunctions< ThisClass, ThisBinder > ThisGeometricBinder; \
			\
			RunTimeTypedClass<TypedData< std::vector< T > > >(); \
			\
			BASIC_VECTOR_BINDING(GeometricTypedData< std::vector< T > >, Tname) \
				/* operators duplicated from BIND_OPERATED_VECTOR_TYPEDDATA */ \
				.def("__add__", &ThisBinder::add, "addition (s + v) : accepts another vector of the same type or a single " Tname) \
				.def("__iadd__", &ThisBinder::iadd, "inplace addition (s += v) : accepts another vector of the same type or a single " Tname) \
				.def("__sub__", &ThisBinder::sub, "subtraction (s - v) : accepts another vector of the same type or a single " Tname) \
				.def("__isub__", &ThisBinder::isub, "inplace subtraction (s -= v) : accepts another vector of the same type or a single " Tname) \
				.def("__mul__", &ThisBinder::mul, "multiplication (s * v) : accepts another vector of the same type or a single " Tname) \
				.def("__imul__", &ThisBinder::imul, "inplace multiplication (s *= v) : accepts another vector of the same type or a single " Tname) \
				.def("__div__", &ThisBinder::div, "division (s / v) : accepts another vector of the same type or a single " Tname) \
				.def("__idiv__", &ThisBinder::idiv, "inplace division (s /= v) : accepts another vector of the same type or a single " Tname) \
				.def("__cmp__", &ThisBinder::invalidOperator, "Raises an exception. This vector type does not support comparison operators.") \
				.def("toString", &ThisBinder::toString, "Returns a string with a copy of the bytes in the vector.") \
				/* geometric methods */ \
				.def("__init__", make_constructor(&ThisGeometricBinder::dataListOrSizeConstructorAndInterpretation), \
					 "Accepts another vector of the same class or a python list containing " Tname \
					 "\nor any other python built-in type that is convertible to it. Alternatively accepts the size of the new vector.") \
				.def("getInterpretation", &ThisClass::getInterpretation, "Returns the geometric interpretation of this data.") \
				.def("setInterpretation", &ThisClass::setInterpretation, "Sets the geometric interpretation of this data.") \
			; \
		} \

} // namespace IECorePython;

#endif // IECOREPYTHON_GEOMETRICTYPEDDATABINDING_INL
