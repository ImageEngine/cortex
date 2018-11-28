//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_PRIMITIVEVARIABLE_INL
#define IECORESCENE_PRIMITIVEVARIABLE_INL

#include "IECoreScene/Export.h"

#include "IECore/VectorTypedData.h"

#include "boost/iterator/iterator_facade.hpp"

namespace IECoreScene
{

template<typename T>
PrimitiveVariable::IndexedView<T>::IndexedView( const PrimitiveVariable &variable )
	:	m_data( data( variable ) ), m_indices( variable.indices ? &variable.indices->readable() : nullptr )
{
}

template<typename T>
PrimitiveVariable::IndexedView<T>::IndexedView( const std::vector<T> &data, const std::vector<int> *indices)
	: 	m_data( &data ), m_indices( indices )
{
}

template<typename T>
const std::vector<T> *PrimitiveVariable::IndexedView<T>::data( const PrimitiveVariable &variable )
{
	typedef IECore::TypedData<std::vector<T>> DataType;
	if( const DataType *d = IECore::runTimeCast<const DataType>( variable.data.get() ) )
	{
		return &d->readable();
	}
	throw IECore::Exception(
		std::string( "PrimitiveVariable does not contain " ) + DataType::staticTypeName()
	);
}

template<typename T>
class PrimitiveVariable::IndexedView<T>::Iterator : public boost::iterator_facade<Iterator, const typename std::vector<T>::value_type, boost::random_access_traversal_tag, typename std::vector<T>::const_reference>
{

	private :

		Iterator( const int *index, typename std::vector<T>::const_iterator it )
			:	m_index( index ), m_it( it )
		{

		}

		friend class PrimitiveVariable::IndexedView<T>;
		friend class boost::iterator_core_access;

		void increment()
		{
			if( m_index )
			{
				++m_index;
			}
			else
			{
				++m_it;
			}
		}

		void decrement()
		{
			if( m_index )
			{
				--m_index;
			}
			else
			{
				--m_it;
			}
		}

		void advance( int64_t n )
		{
			if( m_index )
			{
				m_index += n;
			}
			else
			{
				m_it += n;
			}
		}

		ptrdiff_t distance_to( const Iterator &other ) const
		{
			if( m_index )
			{
				return other.m_index - m_index;
			}
			else
			{
				return other.m_it - m_it;
			}
		}

		bool equal( const Iterator &other ) const
		{
			return m_index == other.m_index && m_it == other.m_it;
		}

		typename std::vector<T>::const_reference dereference() const
		{
			if( m_index )
			{
				return *(m_it+*m_index);
			}
			else
			{
				return *m_it;
			}
		}

		const int *m_index;
		typename std::vector<T>::const_iterator m_it;

};

template<typename T>
typename PrimitiveVariable::IndexedView<T>::Iterator PrimitiveVariable::IndexedView<T>::begin()
{
	return Iterator(
		m_indices ? m_indices->data() : nullptr,
		m_data->begin()
	);
}

template<typename T>
typename PrimitiveVariable::IndexedView<T>::Iterator PrimitiveVariable::IndexedView<T>::end()
{
	return Iterator(
		m_indices ? m_indices->data() + m_indices->size() : nullptr,
		m_indices ? m_data->begin() : m_data->end()
	);
}

/// A simple type to hold named PrimitiveVariables.
typedef std::map<std::string, PrimitiveVariable> PrimitiveVariableMap;

} // namespace IECoreScene

#endif // IECORESCENE_PRIMITIVEVARIABLE_INL
