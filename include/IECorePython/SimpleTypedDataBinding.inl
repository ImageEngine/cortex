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

#include "IECore/SimpleTypedData.h"

namespace IECorePython
{

template<typename T>
TypedDataFromType<T>::TypedDataFromType()
{
	boost::python::converter::registry::push_back(
		&convertible,
		&construct,
		boost::python::type_id<typename T::Ptr>()
	);

}

template<typename T>
void *TypedDataFromType<T>::convertible( PyObject *obj )
{
	boost::python::extract<typename T::ValueType> e( obj );
	if( e.check() )
	{
		return obj;
	}
	return nullptr;
}

template<typename T>
void TypedDataFromType<T>::construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
{
	void *storage = ((boost::python::converter::rvalue_from_python_storage<typename T::Ptr>*)data)->storage.bytes;
	new (storage) typename T::Ptr( new T( boost::python::extract<typename T::ValueType>( obj ) ) );
	data->convertible = storage;
}

// specialise the bool version so it doesn't go gobbling up ints and things and turning them
// into BoolData
template<>
struct TypedDataFromType<IECore::BoolData>
{

	TypedDataFromType()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<IECore::BoolDataPtr>()
		);

	}

	static void *convertible( PyObject *obj )
	{
		if( PyBool_Check( obj ) )
		{
			return obj;
		}
		return nullptr;
	}

	static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((boost::python::converter::rvalue_from_python_storage<IECore::BoolDataPtr>*)data)->storage.bytes;
		new (storage) IECore::BoolDataPtr( new IECore::BoolData( boost::python::extract<bool>( obj ) ) );
		data->convertible = storage;
	}

};

} // namespace IECorePython
