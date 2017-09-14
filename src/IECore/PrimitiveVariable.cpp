//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/DespatchTypedData.h"
#include "IECore/PrimitiveVariable.h"

using namespace IECore;

PrimitiveVariable::PrimitiveVariable()
	: interpolation( Invalid ), data( nullptr ), indices( nullptr )
{
}

PrimitiveVariable::PrimitiveVariable( Interpolation i, DataPtr d )
	: interpolation( i ), data( d ), indices( nullptr )
{
}

PrimitiveVariable::PrimitiveVariable( Interpolation i, DataPtr d, IntVectorDataPtr indices )
	: interpolation( i ), data( d ), indices( indices )
{
}

PrimitiveVariable::PrimitiveVariable( const PrimitiveVariable &other )
{
	interpolation = other.interpolation;
	data = other.data;
	indices = other.indices;
}

PrimitiveVariable::PrimitiveVariable( const PrimitiveVariable &other, bool deepCopy )
{
	interpolation = other.interpolation;
	if( deepCopy )
	{
		data = other.data ? other.data->copy() : nullptr;
		indices = other.indices ? other.indices->copy() : nullptr;
	}
	else
	{
		data = other.data;
		indices = other.indices;
	}
}

bool PrimitiveVariable::operator==( const PrimitiveVariable &other ) const
{
	if( interpolation != other.interpolation )
	{
		return false;
	}

	if( data && other.data )
	{
		if( !data->isEqualTo( other.data.get() ) )
		{
			return false;
		}
	}
	else if( data || other.data )
	{
		return false;
	}

	if( indices && other.indices )
	{
		if( !indices->isEqualTo( other.indices.get() ) )
		{
			return false;
		}
	}
	else if( indices || other.indices )
	{
		return false;
	}

	return true;
}

bool PrimitiveVariable::operator!=( const PrimitiveVariable &other ) const
{
	return !(*this == other);
}

namespace
{

struct Expander
{
	typedef DataPtr ReturnType;

	Expander( const std::vector<int> &indices ) : m_indices( indices )
	{
	}

	const std::vector<int> &m_indices;

	template<typename T>
	ReturnType operator() ( T * data )
	{
		const typename T::ValueType &compactValues = data->readable();

		typename T::Ptr result = new T();
		typename T::ValueType &expandedValues = result->writable();
		expandedValues.reserve( m_indices.size() );
		for( const auto &index : m_indices )
		{
			expandedValues.push_back( compactValues[index] );
		}

		return result;
	}
};

} // namespace

DataPtr PrimitiveVariable::expandedData() const
{
	if( !indices )
	{
		return data->copy();
	}

	Expander expander( indices->readable() );
	return despatchTypedData<Expander, TypeTraits::IsVectorTypedData>( data.get(), expander );
}
