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

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECore/GeometricTypedData.h"

#include <sstream>

namespace Private
{

inline std::string interpretationStr( IECore::GeometricData::Interpretation interpretation )
{
	switch( interpretation )
	{
		case IECore::GeometricData::None:
			return "IECore.GeometricData.Interpretation.None_";
		case IECore::GeometricData::Point:
			return "IECore.GeometricData.Interpretation.Point";
		case IECore::GeometricData::Normal:
			return "IECore.GeometricData.Interpretation.Normal";
		case IECore::GeometricData::Vector:
			return "IECore.GeometricData.Interpretation.Vector";
		case IECore::GeometricData::Color:
			return "IECore.GeometricData.Interpretation.Color";
		case IECore::GeometricData::UV:
			return "IECore.GeometricData.Interpretation.UV";
		case IECore::GeometricData::Rational:
			return "IECore.GeometricData.Interpretation.Rational";
		default:
			return "IECore.GeometricData.Interpretation.None_";
	}
}

} // namespace

namespace IECorePython
{

#define IECOREPYTHON_DEFINEGEOMETRICTYPEDDATASTRSPECIALISATION( TYPE ) \
template<> \
string repr<IECore::GeometricTypedData<TYPE> >( IECore::GeometricTypedData<TYPE> &x ) \
{ \
	stringstream s; \
	s << "IECore." << x.typeName() << "( " << repr( const_cast<TYPE &>( x.readable() ) ) << " )"; \
	return s.str(); \
} \
\
template<> \
string str<IECore::GeometricTypedData<TYPE> >( IECore::GeometricTypedData<TYPE> &x ) \
{ \
	stringstream s; \
	s << str( const_cast<TYPE &>( x.readable() ) ); \
	return s.str(); \
} \

#define IECOREPYTHON_DEFINEGEOMETRICVECTORDATASTRSPECIALISATION( TYPE ) \
template<> \
std::string repr<IECore::GeometricTypedData<std::vector<TYPE> > >( IECore::GeometricTypedData<std::vector<TYPE> > &x ) \
{ \
	std::stringstream s; \
	s << "IECore." << x.typeName() << "( [ "; \
	const IECore::GeometricTypedData<std::vector<TYPE> >::ValueType &xd = x.readable(); \
	for( size_t i=0; i<xd.size(); i++ ) \
	{ \
		s << repr( const_cast<TYPE&>( xd[i] ) ); \
		if( i!=xd.size()-1 ) \
		{ \
			s << ", "; \
		} \
	} \
	s << " ], "; \
	s << Private::interpretationStr(x.getInterpretation()) << ")"; \
	return s.str(); \
} \
\
template<> \
std::string str<IECore::GeometricTypedData<std::vector<TYPE> > >( IECore::GeometricTypedData<std::vector<TYPE> > &x ) \
{ \
	std::stringstream s; \
	const IECore::GeometricTypedData<std::vector<TYPE> >::ValueType &xd = x.readable(); \
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
class GeometricVectorTypedDataFunctions : ThisBinder
{
	public :
		typedef typename ThisBinder::ThisClassPtr ThisClassPtr;
		typedef typename ThisClass::ValueType Container;

		/// constructor that receives a python list object and an interpretation
		static ThisClassPtr
		dataListOrSizeConstructorAndInterpretation( boost::python::object v, IECore::GeometricData::Interpretation i )
		{
			ThisClassPtr r = ThisBinder::dataListOrSizeConstructor( v );
			r->setInterpretation( i );
			return r;
		}

		/// binding for __getitem__ function
		static boost::python::object getItem( ThisClass &x, PyObject *i )
		{
			if ( PySlice_Check( i ) )
			{
				return boost::python::object( getSlice( x, reinterpret_cast<PySliceObject*>( i ) ) );
			}

			const Container &xData = x.readable();
			typename Container::size_type index = convertIndex( x, i );
			return boost::python::object( xData[index] );
		}

		/// return a new object containing the given range of items
		static ThisClassPtr
		getSlice( ThisClass &x, PySliceObject *i )
		{
			ThisClassPtr newObj = ThisBinder::getSlice( x, i );
			newObj->setInterpretation( x.getInterpretation() );
			return newObj;
		}

		/// binding for __add__ function
		static ThisClassPtr add( ThisClass &x, PyObject* y )
		{
			/* create ptr to new object equal to x */
			ThisClassPtr res = ThisClassPtr( new ThisClass( x.readable(), x.getInterpretation() ) );
			iadd( res, y );
			return res;
		}

		/// binding for __sub__ function
		static ThisClassPtr sub( ThisClass &x, PyObject* y )
		{
			/* create ptr to new object equal to x */
			ThisClassPtr res = ThisClassPtr( new ThisClass( x.readable(), x.getInterpretation() ) );
			isub( res, y );
			return res;
		}

		/// binding for __mul__ function
		static ThisClassPtr mul( ThisClass &x, PyObject* y )
		{
			/* create ptr to new object equal to x */
			ThisClassPtr res = ThisClassPtr( new ThisClass( x.readable(), x.getInterpretation() ) );
			imul( res, y );
			return res;
		}

		/// binding for __div__ function
		static ThisClassPtr div( ThisClass &x, PyObject* y )
		{
			/* create ptr to new object equal to x */
			ThisClassPtr res = ThisClassPtr( new ThisClass( x.readable(), x.getInterpretation() ) );
			idiv( res, y );
			return res;
		}

		static ThisClassPtr iadd( ThisClassPtr x_, PyObject* y )
		{
			return ThisBinder::iadd( x_, y );
		}

        static ThisClassPtr isub( ThisClassPtr x_, PyObject* y )
		{
			return ThisBinder::isub( x_, y );
		}

		static ThisClassPtr imul( ThisClassPtr x_, PyObject* y )
		{
			return ThisBinder::imul( x_, y );
		}

		static ThisClassPtr idiv( ThisClassPtr x_, PyObject* y )
		{
			return ThisBinder::idiv( x_, y );
		}

	protected :

	    typedef typename Container::size_type index_type;
	    static index_type convertIndex( ThisClass &container, PyObject *i_,bool acceptExpand = false )
	    {
	        return ThisBinder::convertIndex( container, i_, acceptExpand );
	    }
};

// bind a VectorTypedData class that supports all Math operators (+=, -=, *=, /=)
/// \todo: would be nice if we didn't have to re-define all these operators just to
/// change the macro call and add a couple new methods.
#define BIND_OPERATED_GEOMETRIC_VECTOR_TYPEDDATA(T, Tname) \
		{ \
			typedef IECore::GeometricTypedData< std::vector< T > > ThisClass; \
			typedef VectorTypedDataFunctions< ThisClass > ThisBinder; \
			typedef GeometricVectorTypedDataFunctions< ThisClass, ThisBinder > ThisGeometricBinder; \
			\
			RunTimeTypedClass<IECore::TypedData< std::vector< T > > >(); \
			\
			BASIC_VECTOR_BINDING(IECore::GeometricTypedData< std::vector< T > >, Tname) \
				/* operators modified from BIND_OPERATED_VECTOR_TYPEDDATA */ \
				.def("__getitem__", &ThisGeometricBinder::getItem, "indexing operator.\nAccept an integer index (starting from 0), slices and negative indexes too.") \
				.def("__add__", &ThisGeometricBinder::add, "addition (s + v) : accepts another vector of the same type or a single " Tname) \
				.def("__sub__", &ThisGeometricBinder::sub, "subtraction (s - v) : accepts another vector of the same type or a single " Tname) \
				.def("__mul__", &ThisGeometricBinder::mul, "multiplication (s * v) : accepts another vector of the same type or a single " Tname) \
				.def("__div__", &ThisGeometricBinder::div, "division (s / v) : accepts another vector of the same type or a single " Tname) \
				.def("__truediv__", &ThisGeometricBinder::div, "division (s / v) : accepts another vector of the same type or a single " Tname) \
				/* operators duplicated from BIND_OPERATED_VECTOR_TYPEDDATA */ \
				.def("__iadd__", &ThisGeometricBinder::iadd, "inplace addition (s += v) : accepts another vector of the same type or a single " Tname) \
				.def("__isub__", &ThisGeometricBinder::isub, "inplace subtraction (s -= v) : accepts another vector of the same type or a single " Tname) \
				.def("__imul__", &ThisGeometricBinder::imul, "inplace multiplication (s *= v) : accepts another vector of the same type or a single " Tname) \
				.def("__idiv__", &ThisGeometricBinder::idiv, "inplace division (s /= v) : accepts another vector of the same type or a single " Tname) \
				.def("__itruediv__", &ThisGeometricBinder::idiv, "inplace division (s /= v) : accepts another vector of the same type or a single " Tname) \
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
