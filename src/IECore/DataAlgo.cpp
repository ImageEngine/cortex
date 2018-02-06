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

namespace
{

struct GeometricInterpretationGetter
{
	typedef IECore::GeometricData::Interpretation ReturnType;

	GeometricInterpretationGetter()
	{
	}

	template<typename T>
	ReturnType operator()( T *data )
	{
		return data->getInterpretation();
	}
};

struct GeometricInterpretationSetter
{
	typedef void ReturnType;

	GeometricInterpretationSetter( IECore::GeometricData::Interpretation interpretation ) : m_interpretation( interpretation )
	{
	}

	template<typename T>
	ReturnType operator()( T *data )
	{
		data->setInterpretation( m_interpretation );
	}

	IECore::GeometricData::Interpretation m_interpretation;
};

struct SetGeometricInterpretationError
{
	template<typename T, typename F>
	void operator()( const T *data, const F &functor )
	{
		if( functor.m_interpretation != IECore::GeometricData::None )
		{
			throw IECore::InvalidArgumentException( std::string( "Cannot set geometric interpretation on type " ) + data->typeName() );
		}
	}
};

template<typename T> struct IsSupportedVectorTypedData : public boost::false_type {};
template<> struct IsSupportedVectorTypedData< IECore::TypedData<std::vector<std::string> > > : public boost::true_type {};
template<> struct IsSupportedVectorTypedData< IECore::TypedData<std::vector<bool> > > : public boost::true_type {};
template<> struct IsSupportedVectorTypedData< IECore::TypedData<std::vector<int> > > : public boost::true_type {};
template<> struct IsSupportedVectorTypedData< IECore::TypedData<std::vector<unsigned int > > > : public boost::true_type {};


class UniqueValueCollector
{
	public:
		UniqueValueCollector()
		{
		}

		typedef IECore::DataPtr ReturnType;

		template<typename T>
		ReturnType operator()( T *array )
		{
			auto r = new IECore::TypedData<typename T::ValueType>();

			typedef typename T::ValueType::value_type BaseType;
			std::unordered_set<BaseType> uniqueValues;

			const auto &readable = array->readable();
			for( const auto &t : readable )
			{
				uniqueValues.insert( t );
			}

			auto &writable = r->writable();
			writable.reserve( uniqueValues.size() );
			for( const auto &t : uniqueValues )
			{
				writable.push_back( t );
			}

			return r;
		}
};

} // namespace

IECore::GeometricData::Interpretation IECore::getGeometricInterpretation( const IECore::Data *data )
{
	GeometricInterpretationGetter getter;

	/// \todo - would be nice if there was a const version of despatchTypedData so that I didn't need to const_cast here
	/// Should be entirely safe though, since at this point I know that GeometricInterpretationGetter will not modify its input.
	return IECore::despatchTypedData<GeometricInterpretationGetter, IECore::TypeTraits::IsGeometricTypedData, IECore::DespatchTypedDataIgnoreError>(
		const_cast<IECore::Data *>(data),
		getter
	);
}

void IECore::setGeometricInterpretation( IECore::Data *data, IECore::GeometricData::Interpretation interpretation )
{
	GeometricInterpretationSetter setter( interpretation );
	IECore::despatchTypedData<GeometricInterpretationSetter, IECore::TypeTraits::IsGeometricTypedData, SetGeometricInterpretationError>( data, setter );
}

IECore::DataPtr IECore::uniqueValues(const IECore::Data *data)
{
	UniqueValueCollector uniqueValueCollector;
	return IECore::despatchTypedData<UniqueValueCollector, IsSupportedVectorTypedData>(
		const_cast<IECore::Data *>( data ),
		uniqueValueCollector
	);
}
