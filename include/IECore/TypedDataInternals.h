//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TYPEDDATAINTERNALS_H
#define IECORE_TYPEDDATAINTERNALS_H

#include "IECore/MurmurHash.h"

namespace IECore
{

template<class T>
class SimpleDataHolder
{

	public :

		SimpleDataHolder()
			: m_data()
		{
		}

		SimpleDataHolder( const T &data )
			: m_data( data )
		{
		}

		const T &readable() const
		{
			return m_data;
		}

		T &writable()
		{
			return m_data;
		}

		bool operator == ( const SimpleDataHolder<T> &other ) const
		{
			return m_data == other.m_data;
		}

		void hash( MurmurHash &h ) const
		{
			h.append( readable() );
		}

	private :

		T m_data;

};

template<class T>
class SharedDataHolder
{

	public :

		SharedDataHolder()
			: m_data( new Shareable )
		{
		}

		SharedDataHolder( const T &data )
			: m_data( new Shareable( data ) )
		{
		}

		const T &readable() const
		{
			assert( m_data );
			return m_data->data;
		}

		T &writable()
		{
			assert( m_data );
			if( m_data->refCount() > 1 )
			{
				// duplicate the data
				m_data = new Shareable( m_data->data );
			}
			m_data->hashValid = false;
			return m_data->data;
		}

		bool operator == ( const SharedDataHolder<T> &other ) const
		{
			if( m_data==other.m_data )
			{
				// comparing the pointers is quick and that's good
				return true;
			}
			// pointers ain't the same - do a potentially slow comparison
			return readable()==other.readable();
		}

		// The method called by the TypedData class when it wants to
		// append the hash for the internal data into h. This is recomputed
		// lazily only after writable() has been called. Rather than modify this
		// function, instead specialise the protected hash() method if the underlying
		// datatype has special needs.
		void hash( MurmurHash &h ) const
		{
			if( !m_data->hashValid )
			{
				m_data->hash = hash();
				m_data->hashValid = true;
			}
			h.append( m_data->hash );
		}

	protected :

		MurmurHash hash() const
		{
			MurmurHash result;
			result.append( &(readable()[0]), readable().size() );
			return result;
		}

	private :

		class Shareable : public RefCounted
		{
			public :

				Shareable() : data(), hashValid( false ) {}
				Shareable( const T &initData ) : data( initData ), hashValid( false ) {}

				T data;
				MurmurHash hash;
				volatile bool hashValid;

		};

		IE_CORE_DECLAREPTR( Shareable )
		ShareablePtr m_data;

};

template <class T>
class TypedDataTraits
{
	public:
		typedef void BaseType;
		// DataHolder /must/ be specialised to something other than void
		// using IECORE_DECLARE_TYPEDDATA. It is left void here so that
		// you must include the appropriate *TypedData.h header - otherwise
		// you might just include TypedData.h, missing a specialisation which
		// then causes sizeof( TypedData<T> ) to be incorrect.
		typedef void DataHolder;
};

} // namespace IECore

#endif // IECORE_TYPEDDATAINTERNALS_H
