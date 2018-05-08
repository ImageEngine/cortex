//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DataAlgo.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/TypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"

#include <unordered_set>

using namespace IECore;

namespace
{

struct GeometricInterpretationGetter
{
	template<typename T>
	IECore::GeometricData::Interpretation operator()( const IECore::GeometricTypedData<T> *data )
	{
		return data->getInterpretation();
	}

	IECore::GeometricData::Interpretation operator()( const Data *data )
	{
		return IECore::GeometricData::Interpretation::None;
	}
};

struct GeometricInterpretationSetter
{
	GeometricInterpretationSetter( IECore::GeometricData::Interpretation interpretation ) : m_interpretation( interpretation )
	{
	}

	template<typename T>
	void operator()( IECore::GeometricTypedData<T> *data )
	{
		data->setInterpretation( m_interpretation );
	}

	void operator()( Data *data )
	{
		if( m_interpretation != IECore::GeometricData::None )
		{
			throw IECore::InvalidArgumentException( std::string( "Cannot set geometric interpretation on type " ) + data->typeName() );
		}
	}

	private :

		IECore::GeometricData::Interpretation m_interpretation;

};

struct UniqueValueCollector
{

	DataPtr operator()( const StringVectorData *data )
	{
		return uniqueValues( data );
	}

	DataPtr operator()( const BoolVectorData *data )
	{
		return uniqueValues( data );
	}

	DataPtr operator()( const IntVectorData *data )
	{
		return uniqueValues( data );
	}

	DataPtr operator()( const UIntVectorData *data )
	{
		return uniqueValues( data );
	}

	DataPtr operator()( const Data *data )
	{
		return nullptr;
	}

	private :

		template<typename T>
		DataPtr uniqueValues( const T *data )
		{
			typedef typename T::ValueType::value_type BaseType;
			std::unordered_set<BaseType> uniqueValues;

			for( const auto &t : data->readable() )
			{
				uniqueValues.insert( t );
			}

			typename T::Ptr result = new T;
			auto &writable = result->writable();
			writable.reserve( uniqueValues.size() );
			for( const auto &t : uniqueValues )
			{
				writable.push_back( t );
			}

			return result;
		}
};

struct Size
{

	template<typename T>
	size_t operator()( const TypedData<std::vector<T>> *data )
	{
		return data->readable().size();
	}

	template<typename T>
	size_t operator()( const TypedData<T> *data )
	{
		return 1;
	}

	size_t operator()( const Data *data )
	{
		return 0;
	}

};

struct Address
{

	void *operator()( BoolVectorData *data )
	{
		return nullptr;
	}

	const void *operator()( const BoolVectorData *data )
	{
		return nullptr;
	}

	template<typename T>
	void *operator()( TypedData<std::vector<T>> *data )
	{
		return data->writable().data();
	}

	template<typename T>
	const void *operator()( const TypedData<std::vector<T>> *data )
	{
		return data->readable().data();
	}

	template<typename T>
	void *operator()( TypedData<T> *data )
	{
		return &data->writable();
	}

	template<typename T>
	const void *operator()( const TypedData<T> *data )
	{
		return &data->readable();
	}

	void *operator()( Data *data )
	{
		return nullptr;
	}

	const void *operator()( const Data *data )
	{
		return nullptr;
	}

};

} // namespace

IECore::GeometricData::Interpretation IECore::getGeometricInterpretation( const IECore::Data *data )
{
	return dispatch( data, GeometricInterpretationGetter() );
}

void IECore::setGeometricInterpretation( IECore::Data *data, IECore::GeometricData::Interpretation interpretation )
{
	dispatch( data, GeometricInterpretationSetter( interpretation ) );
}

IECore::DataPtr IECore::uniqueValues(const IECore::Data *data)
{
	return dispatch( data, UniqueValueCollector() );
}

IECORE_API size_t IECore::size( const IECore::Data *data )
{
	return dispatch( data, Size() );
}

IECORE_API void *IECore::address( IECore::Data *data )
{
	return dispatch( data, Address() );
}

IECORE_API const void *IECore::address( const IECore::Data *data )
{
	return dispatch( data, Address() );
}

