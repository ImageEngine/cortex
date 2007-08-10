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

#ifndef IE_CORE_VECTORTYPEDDATABINDING_INL
#define IE_CORE_VECTORTYPEDDATABINDING_INL

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

#include <sstream>

namespace IECore
{

template<typename Container>
class VectorTypedDataFunctions
{
public:
	typedef TypedData< Container > ThisClass;
	typedef typename boost::intrusive_ptr< ThisClass > ThisClassPtr;
	typedef typename Container::value_type data_type;
	typedef typename Container::size_type index_type;
	typedef typename Container::size_type size_type;
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;

	/// default constructor
	static ThisClassPtr 
	dataConstructor() 
	{
		return ThisClassPtr(new ThisClass());
	}

	/// constructor that receives a python list object
	static ThisClassPtr 
	dataListOrSizeConstructor(boost::python::object v) 
	{
		boost::python::extract<size_type> x( v );
		if( x.check() )
		{
			// we've got a length
			ThisClassPtr r = new ThisClass();
			r->writable().resize( x() );
			return r;
		}
		else
		{
			ThisClassPtr r = new ThisClass();
			boost::python::container_utils::extend_container(r->writable(), v);
			return r;
		}
	}

	/// binding for __getitem__ function
	static boost::python::object getItem(ThisClass &x, PyObject *i)
	{	
		if (PySlice_Check(i)) 
		{
			return boost::python::object(getSlice(x, reinterpret_cast<PySliceObject*>(i)));
		}

		const Container &xData = x.readable();
		index_type index = convertIndex(x, i);
		return boost::python::object(xData[index]);
	}
	
	/// return a new object containing the given range of items
	static ThisClassPtr
	getSlice(ThisClass &x, PySliceObject *i)
	{
		long from, to;
		convertSlice(x, i, from, to);
		const Container &xData = x.readable();
		ThisClassPtr newObj = ThisClassPtr(new ThisClass());
		Container &objData = newObj->writable();
		const_iterator iterX = xData.begin() + from;
		for ( ; iterX < xData.begin() + to; ++iterX)
		{
			objData.push_back(*iterX);		
		}
		return newObj;
	}
	
	/// binding for __setitem__ function
	static void setItem(ThisClass &x, PyObject *i, boost::python::object v)
	{
		if (PySlice_Check(i))
		{
			 setSlice(x, reinterpret_cast<PySliceObject*>(i), v);
		}
		else
		{
			Container &xData = x.writable();
			index_type index = convertIndex(x, i);
			xData[index] = convertValue(v.ptr());
		}
	}
	
	/// set a range of items with a specified value or group of values
	static void setSlice(ThisClass &x, PySliceObject *i, boost::python::object v)
	{
		long from, to;
		convertSlice(x, i, from, to);

		Container temp;
		const Container *vData = &temp;
		if (PyList_Check(v.ptr())) {
			// we are dealing with a python list object
			boost::python::container_utils::extend_container(temp, v);
		} else {
			// try to extract the same object
			boost::python::extract<ThisClass&> elem(v.ptr());
			// try if elem is an exact Data
			if (elem.check())
			{
				// ok!
				vData = &(elem().readable());
			}	
			else
			{
				// try to extract a single data_type.
				data_type value = convertValue(v.ptr());
				if (from <= to) 
				{
					Container &xData = x.writable();
					xData.erase(xData.begin()+from, xData.begin()+to);
					xData.insert(xData.begin()+from, value);
				}
				return;
			}
		}
		Container &xData = x.writable();
		// we have vData pointing to a valid vector
		if (from > to) 
		{
			xData.insert(xData.begin()+from, vData->begin(), vData->end());
		}
		else 
		{
			xData.erase(xData.begin()+from, xData.begin()+to);
			xData.insert(xData.begin()+from, vData->begin(), vData->end());
		}
	}
	
	/// binding for append function
	static void append(ThisClass &x, PyObject* v)
	{
		Container &xData = x.writable();
		boost::python::extract<data_type&> elem(v);
		xData.push_back(convertValue(v));
	}
	
	/// binding for __delitem__ function
	static void delItem(ThisClass &x, PyObject *i)
	{	
		if (PySlice_Check(i))
		{
			delSlice(x, reinterpret_cast<PySliceObject*>(i));
			return;
		}
		Container &xData = x.writable();
		index_type index = convertIndex(x, i);
		xData.erase(xData.begin()+index);
	}

	/// remove a range of elements from the vector
	static void delSlice(ThisClass &x, PySliceObject *i)
	{	
		long from, to;
		convertSlice(x, i, from, to);
		Container &xData = x.writable();
		xData.erase(xData.begin()+from, xData.begin()+to);
	}

	/// binding for __contains__ function
	static bool contains(ThisClass &x, const data_type &v)
	{	
		const Container &xData = x.readable();
		return (find(xData.begin(), xData.end(), v) != xData.end());
	}

	/// binding for __len__ function
	static size_type len(ThisClass &x)
	{
		return x.readable().size();
	}
	
	static void resize( ThisClass &x, size_t s )
	{
		x.writable().resize( s );
	}

	/// binding for append function
	static void extend(ThisClass &x, boost::python::object v)
	{
		Container temp;
		const Container *vData = &temp;
		if (PyList_Check(v.ptr())) {
			// we are dealing with a python list object
			boost::python::container_utils::extend_container(temp, v);
		} else {
			// try to extract the same object
			boost::python::extract<ThisClass&> elem(v.ptr());
			// try if elem is an exact Data
			if (elem.check())
			{
				// ok!
				vData = &(elem().readable());
			}	
			else
			{
			   	PyErr_SetString(PyExc_TypeError, "Invalid parameter");
				boost::python::throw_error_already_set();
			}
		}
		// now concatenate the given list to the object
		Container &xData = x.writable();
		const_iterator iterV = vData->begin();
		for (; iterV != vData->end(); iterV++) 
		{
			xData.push_back(*iterV);
		}
	}
	
	/// binding for count function
	static size_t
	count(ThisClass &x, const data_type &v)
	{
		size_t counter = 0;
		const Container &xData = x.readable();
		typename Container::const_iterator iterX = xData.begin();
		while ((iterX = find(iterX, xData.end(), v)) != xData.end())
		{
			counter++;
			iterX++;
		}
		return counter;
	}
	
	/// binding for index(x) function
	static size_t
	index1(ThisClass &x, const data_type &v)
	{
		const Container &xData = x.readable();
		return index(x, v, PyInt_FromLong(0), PyInt_FromLong(xData.size()));
	}
	
	/// binding for index(x, start) function
	static size_t
	index2(ThisClass &x, const data_type &v, PyObject *i)
	{
		const Container &xData = x.readable();
		return index(x, v, i, PyInt_FromLong(xData.size()));
	}
	
	/// binding for index(x, start, end) function
	static size_t
	index(ThisClass &x, const data_type &v, PyObject *i, PyObject *j)
	{
   		index_type beginIndex = convertIndex(x, i, true);
   		index_type endIndex = convertIndex(x, j, true);
		
		const Container &xData = x.readable();
		typename Container::const_iterator beginX = xData.begin() + beginIndex;
		typename Container::const_iterator endX = xData.begin() + endIndex;
		typename Container::const_iterator iterX;
		iterX = find(beginX, endX, v);
		if (iterX == endX) 
		{
		   	PyErr_SetString(PyExc_ValueError, "VectorTypedData.index(x): x not in list");
			boost::python::throw_error_already_set();
		}
		return (iterX - xData.begin());
	}
	
	/// binding for insert function
	static void
	insert(ThisClass &x, PyObject *i, PyObject *v)
	{
		Container &xData = x.writable();
		typename Container::iterator iterX = xData.begin() + convertIndex(x, i, true);
		xData.insert(iterX, convertValue(v));
	}

	/// binding for __cmp__ function
	static int cmp(ThisClass &x, ThisClass &y)
	{
		const Container &xData = x.readable();
		const Container &yData = y.readable();
		const_iterator iterX = xData.begin();
		const_iterator iterY = yData.begin();
		while (iterX < xData.end() && iterY < yData.end())
		{
			if (*iterX < *iterY) 
			{
				return -1;
			}
			else if (*iterX > *iterY)
			{
				return +1;
			}
			iterX++;
			iterY++;
		}
		if (iterX < xData.end() && iterY == yData.end())
		{
			return +1;
		}
		if (iterY < yData.end() && iterX == xData.end())
		{
			return -1;
		}
		return 0;
	}

	/*	
	static object getList(ThisClass &x)
	{
		const Container &xData = x.readable();
		PyObject myList = PyList_New(xData.size());
		const_iterator iterX = xData.begin() + from;
		for (size_t index = 0; iterX <= xData.end(); ++iterX, index++)
		{
			myList.SetItem(index, object(*iterX));
		}
		return object(myList);
	}
	*/
/*	
	static index_type iter()
	{
	}
*/	

	/*
	 * Math Operators
	 */

#define BINARY_OPERATOR_CODE(op)																\
		const Container &xData = x.readable();													\
		boost::python::extract<ThisClass&> elem(y);												\
		/* try if y is another vector of the same type */										\
		if (elem.check())																		\
		{																						\
			const Container &yData = elem().readable();											\
			if (xData.size() != yData.size()) {													\
			   	PyErr_SetString(PyExc_TypeError, "Vector sizes don't match.");					\
			   	boost::python::throw_error_already_set();										\
			}																					\
			/* use operator for each element on y. */											\
			Container &resData = x.writable();													\
			iterator iterRes = resData.begin();													\
			const_iterator iterY = yData.begin();												\
			while (iterY != yData.end()) 														\
			{																					\
				(*iterRes) op (*iterY);															\
				iterRes++;																		\
				iterY++;																		\
			}																					\
		}																						\
		else 																					\
		{																						\
			/* try if y is a single value of data_type */										\
			boost::python::extract<data_type> elem(y);											\
			if (elem.check()) 																	\
			{																					\
				/* use operator for each element on x. */				 						\
				Container &resData = x.writable();												\
				iterator iterRes = resData.begin();												\
				while (iterRes != resData.end()) 												\
				{																				\
					(*iterRes) op elem();														\
					iterRes++;																	\
				}																				\
			}																					\
			else 																				\
			{																					\
			   	PyErr_SetString(PyExc_SyntaxError, "Invalid operator");							\
				boost::python::throw_error_already_set();										\
			}																					\
		}
		
	/* Mathmatical operations on the vector */
	
	/// binding for any unsupported binary operator
	static ThisClassPtr 
	invalidOperator(ThisClass &x, PyObject* y)
	{
	   	PyErr_SetString(PyExc_SyntaxError, "Binary operator not supported for this class.");
	  	boost::python::throw_error_already_set();
		ThisClassPtr res;
		return res;
	}
	
	
	/// binding for __add__ function
	static ThisClassPtr 
	add(ThisClass &x, PyObject* y)
	{
		/* create ptr to new object equal to x */
		ThisClassPtr res = ThisClassPtr(new ThisClass(x.readable()));
		iadd(res, y);
		return res;
	}

	/// binding for __iadd__ function
	static ThisClassPtr 
	iadd(ThisClassPtr x_, PyObject* y)
	{
		ThisClass &x = *x_;
		BINARY_OPERATOR_CODE(+=)
		return x_;
	}

	/// binding for __sub__ function
	static ThisClassPtr 
	sub(ThisClass &x, PyObject* y)
	{
		/* create ptr to new object equal to x */
		ThisClassPtr res = ThisClassPtr(new ThisClass(x.readable()));
		isub(res, y);
		return res;
	}

	/// binding for __isub__ function
	static ThisClassPtr 
	isub(ThisClassPtr x_, PyObject* y)
	{
		ThisClass &x = *x_;
		BINARY_OPERATOR_CODE(-=)
		return x_;
	}

	/// binding for __mul__ function
	static ThisClassPtr 
	mul(ThisClass &x, PyObject* y)
	{
		/* create ptr to new object equal to x */
		ThisClassPtr res = ThisClassPtr(new ThisClass(x.readable()));
		imul(res, y);
		return res;
	}
	
	/// binding for __imul__ function
	static ThisClassPtr 
	imul(ThisClassPtr x_, PyObject* y)
	{
		ThisClass &x = *x_;
		BINARY_OPERATOR_CODE(*=)
		return x_;
	}

	/// binding for __div__ function
	static ThisClassPtr 
	div(ThisClass &x, PyObject* y)
	{
		/* create ptr to new object equal to x */
		ThisClassPtr res = ThisClassPtr(new ThisClass(x.readable()));
		idiv(res, y);
		return res;
	}
	 
	/// binding for __idiv__ function
	static ThisClassPtr 
	idiv(ThisClassPtr x_, PyObject* y)
	{
		ThisClass &x = *x_;
		BINARY_OPERATOR_CODE(/=)
		return x_;
	}

protected:
	/* 
	 * Utility functions
	 */

	/// converts from python indexes to non-negative C++ indexes.
	static index_type
	convertIndex(ThisClass & container, PyObject *i_, bool acceptExpand = false)
	{ 
		boost::python::extract<long> i(i_);
		if (i.check())
		{
			long index = i();
			size_type curSize = len(container);
			if (index < 0)
				index += curSize;
				
			if (acceptExpand) 
			{
				if (index < 0)
					index = 0;
				if (index > (int)curSize)
					index = curSize;
			}
			else
			{
				if (index >= (int)curSize || index < 0)
				{
					PyErr_SetString(PyExc_IndexError, "Index out of range");
					boost::python::throw_error_already_set();
				}
			}
			return (index_type)index;
		}
		PyErr_SetString(PyExc_TypeError, "Invalid index type");
		boost::python::throw_error_already_set();
		return index_type();
	}
	
	/// converts python slices to non-negative C++ indexes.
	static void
	convertSlice(ThisClass & container, PySliceObject* slice, long& from_, long& to_)
	{
		if (Py_None != slice->step) {
			PyErr_SetString( PyExc_IndexError, "slice step size not supported.");
			boost::python::throw_error_already_set();
		}

		long min_index = 0;
		long max_index = static_cast<long>(container.readable().size());

		if (Py_None == slice->start) {
			from_ = min_index;
		}
		else {
			long from = boost::python::extract<long>( slice->start);
			if (from < 0) // Negative slice index
				from += max_index;
			if (from < 0) // Clip lower bounds to zero
				from = 0;
			from_ = boost::numeric_cast< long >(from);
			if (from_ > max_index) // Clip upper bounds to max_index.
				from_ = max_index;
		}
		if (Py_None == slice->stop) {
			to_ = max_index;
		}
		else {
			long to = boost::python::extract<long>( slice->stop);
			if (to < 0)
				to += max_index;
			if (to < 0)
				to = 0;
			to_ = boost::numeric_cast< long >(to);
			if (to_ > max_index)
				to_ = max_index;
		}
	}		
	
	/// convert a python object explicitly or implicitly to data_type
	static data_type 
	convertValue(PyObject *v)
	{
		boost::python::extract<data_type&> elem(v);
		// try if elem is an exact Data
		if (elem.check())
		{
			return elem();
		}
		else
		{
			//  try to convert elem to Data
			boost::python::extract<data_type> elem(v);
			if (elem.check())
			{
				return elem();
			}
			else
			{
			   	PyErr_SetString(PyExc_TypeError, "Invalid parameter type");
				boost::python::throw_error_already_set();
			}
			return data_type();
		}
	}
};

#define IE_COREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( TYPE )											\
template<>																								\
std::string repr<TypedData<std::vector<TYPE> > >( TypedData<std::vector<TYPE> > &x )					\
{																										\
	std::stringstream s;																				\
	s << x.typeName() << "( [ ";																		\
	const TypedData<std::vector<TYPE> >::ValueType &xd = x.readable();									\
	for( size_t i=0; i<xd.size(); i++ )																	\
	{																									\
		s << repr( const_cast<TYPE&>( xd[i] ) );														\
		if( i!=xd.size()-1 )																			\
		{																								\
			s << ", ";																					\
		}																								\
	}																									\
	s<< " ] )";																							\
	return s.str();																						\
}																										\
																										\
																										\
template<>																								\
std::string str<TypedData<std::vector<TYPE> > >( TypedData<std::vector<TYPE> > &x )						\
{																										\
	std::stringstream s;																				\
	const TypedData<std::vector<TYPE> >::ValueType &xd = x.readable();									\
	for( size_t i=0; i<xd.size(); i++ )																	\
	{																									\
		s << str( const_cast<TYPE &>( xd[i] ) );														\
		if( i!=xd.size()-1 )																			\
		{																								\
			s << " ";																					\
		}																								\
	}																									\
	return s.str();																						\
}																										\


#define BASIC_VECTOR_BINDING(T, bindName, Tname)																	\
		typedef TypedData< std::vector< T > > ThisClass;															\
		typedef boost::intrusive_ptr< ThisClass > ThisClassPtr;														\
		typedef VectorTypedDataFunctions< std::vector<T> > ThisBinder;												\
																													\
		typedef class_< ThisClass , boost::intrusive_ptr< ThisClass >, boost::noncopyable, bases<Data> > PyClass;	\
		PyClass(bindName, 																							\
			Tname "-type vector class derived from Data class.\n"													\
			"This class behaves like the native python lists, except that it only accepts " Tname " values.\n"		\
			"The copy constructor accepts another instance of this class or a python list containing " Tname		\
			"\nor any other python built-in type that is convertible into it.\n"									\
 			"It accepts slicing, negative indexing and special functions like extend, insert, etc.\n"				\
			, no_init)																								\
		.def("__init__", make_constructor(&ThisBinder::dataConstructor), "Default constructor: creates an empty vector.")	\
		.def("__init__", make_constructor(&ThisBinder::dataListOrSizeConstructor),										\
					 "Accepts another vector of the same class or a python list containing " Tname \
					 "\nor any other python built-in type that is convertible to it. Alternatively accepts the size of the new vector.")	 						\
		.def("__getitem__", &ThisBinder::getItem, "indexing operator.\nAccept an integer index (starting from 0), slices and negative indexes too.")		\
		.def("__setitem__", &ThisBinder::setItem, "index assignment operator.\nWorks exactly like on python lists but it only accepts " Tname " as the new value.")	\
		.def("__delitem__", &ThisBinder::delItem, "index deletion operator.\nWorks exactly like on python lists.")		\
		.def("__contains__", &ThisBinder::contains, "In operator.\nWorks exactly like on python lists.")				\
		.def("__len__", &ThisBinder::len, "Length operator.")															\
		.def("append", &ThisBinder::append, "s.append(x)\nAppends a new element x to the end of s")					\
		.def("extend", &ThisBinder::extend, "s.extend(t)\nAppends a new vector or list t to the end of s")			\
		.def("count", &ThisBinder::count, "s.count(x)\nCount ocurrences of an element x in s")						\
		.def("index", &ThisBinder::index, "s.index(x [,start [,stop]])\nReturns the smallest i where s[i] == x.\n"	\
											"start and stop optionally specify the starting and ending index for the search.")	\
		.def("index", &ThisBinder::index1)																			\
		.def("index", &ThisBinder::index2)																			\
		.def("insert", &ThisBinder::insert, "s.insert(i, x)\nInserts x at index i.")								\
		/*  pop, remove, reverse, sort.*/																			\
		.def("size", &ThisBinder::len, "s.size()\nReturns the number of elements on s. Same result as the len operator.")	\
		.def("resize", &ThisBinder::resize, "s.resize( size )\nAdjusts the size of s.")	\
		.def("__str__", &str<ThisClass> )	\
		.def("__repr__", &repr<ThisClass> )	\
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( ThisClass )

// bind a VectorTypedData class that does not support Math operators
#define BIND_VECTOR_TYPEDDATA(T, bindName, Tname)													\
		{																							\
			BASIC_VECTOR_BINDING(T, bindName, Tname)																	\
			.def("__cmp__", &ThisBinder::invalidOperator, "Raises an exception. This vector type does not support comparison operators.")		\
			;																						\
			INTRUSIVE_PTR_PATCH( ThisClass, PyClass );												\
			implicitly_convertible<ThisClassPtr, DataPtr>();										\
		}

// bind a VectorTypedData class that supports simple Math operators (+=, -= and *=)
#define BIND_SIMPLE_OPERATED_VECTOR_TYPEDDATA(T, bindName, Tname)									\
		{																							\
			BASIC_VECTOR_BINDING(T, bindName, Tname)																	\
			/* operators */																			\
			.def("__add__", &ThisBinder::add, "addition (s + v) : accepts another vector of the same type or a single " Tname)						\
			.def("__iadd__", &ThisBinder::iadd, "inplace addition (s += v) : accepts another vector of the same type or a single " Tname)			\
			.def("__sub__", &ThisBinder::sub, "subtraction (s - v) : accepts another vector of the same type or a single " Tname)					\
			.def("__isub__", &ThisBinder::isub, "inplace subtraction (s -= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__mul__", &ThisBinder::mul, "multiplication (s * v) : accepts another vector of the same type or a single " Tname)				\
			.def("__imul__", &ThisBinder::imul, "inplace multiplication (s *= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__cmp__", &ThisBinder::invalidOperator, "Raises an exception. This vector type does not support comparison operators.")		\
			;																						\
			INTRUSIVE_PTR_PATCH( ThisClass, PyClass );												\
			implicitly_convertible<ThisClassPtr, DataPtr>();										\
		}

// bind a VectorTypedData class that supports all Math operators (+=, -=, *=, /=)
#define BIND_OPERATED_VECTOR_TYPEDDATA(T, bindName, Tname)											\
		{																							\
			BASIC_VECTOR_BINDING(T, bindName, Tname)																	\
			/* operators */																			\
			.def("__add__", &ThisBinder::add, "addition (s + v) : accepts another vector of the same type or a single " Tname)						\
			.def("__iadd__", &ThisBinder::iadd, "inplace addition (s += v) : accepts another vector of the same type or a single " Tname)			\
			.def("__sub__", &ThisBinder::sub, "subtraction (s - v) : accepts another vector of the same type or a single " Tname)					\
			.def("__isub__", &ThisBinder::isub, "inplace subtraction (s -= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__mul__", &ThisBinder::mul, "multiplication (s * v) : accepts another vector of the same type or a single " Tname)				\
			.def("__imul__", &ThisBinder::imul, "inplace multiplication (s *= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__div__", &ThisBinder::div, "division (s / v) : accepts another vector of the same type or a single " Tname)						\
			.def("__idiv__", &ThisBinder::idiv, "inplace division (s /= v) : accepts another vector of the same type or a single " Tname)			\
			.def("__cmp__", &ThisBinder::invalidOperator, "Raises an exception. This vector type does not support comparison operators.")		\
			;																						\
			INTRUSIVE_PTR_PATCH( ThisClass, PyClass );												\
			implicitly_convertible<ThisClassPtr, DataPtr>();										\
		}

// bind a VectorTypedData class that supports all Math operators (+=, -=, *=, /=, <, >)
#define BIND_FULL_OPERATED_VECTOR_TYPEDDATA(T, bindName, Tname)											\
		{																							\
			BASIC_VECTOR_BINDING(T, bindName, Tname)																	\
			/* operators */																			\
			.def("__add__", &ThisBinder::add, "addition (s + v) : accepts another vector of the same type or a single " Tname)						\
			.def("__iadd__", &ThisBinder::iadd, "inplace addition (s += v) : accepts another vector of the same type or a single " Tname)			\
			.def("__sub__", &ThisBinder::sub, "subtraction (s - v) : accepts another vector of the same type or a single " Tname)					\
			.def("__isub__", &ThisBinder::isub, "inplace subtraction (s -= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__mul__", &ThisBinder::mul, "multiplication (s * v) : accepts another vector of the same type or a single " Tname)				\
			.def("__imul__", &ThisBinder::imul, "inplace multiplication (s *= v) : accepts another vector of the same type or a single " Tname)		\
			.def("__div__", &ThisBinder::div, "division (s / v) : accepts another vector of the same type or a single " Tname)						\
			.def("__idiv__", &ThisBinder::idiv, "inplace division (s /= v) : accepts another vector of the same type or a single " Tname)			\
			.def("__cmp__", &ThisBinder::cmp, "comparison operators (<, >, >=, <=) : The comparison is element-wise, like a string comparison. \n")	\
			;																						\
			INTRUSIVE_PTR_PATCH( ThisClass, PyClass );												\
			implicitly_convertible<ThisClassPtr, DataPtr>();										\
		}

} // namespace IECore

#endif // IE_CORE_VECTORTYPEDDATABINDING_INL
