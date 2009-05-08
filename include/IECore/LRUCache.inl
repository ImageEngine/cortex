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

#include <cassert>

namespace IECore
{

template<typename Key, typename Data, typename GetterFn>
LRUCache<Key, Data, GetterFn>::LRUCache()
{
	m_maxCost = 500;
	m_currentCost = Cost(0);
}

template<typename Key, typename Data, typename GetterFn>
LRUCache<Key, Data, GetterFn>::~LRUCache()
{
}

template<typename Key, typename Data, typename GetterFn>
void LRUCache<Key, Data, GetterFn>::clear()
{
	m_currentCost = Cost(0);
	m_list.clear();
	m_cache.clear();
}


template<typename Key, typename Data, typename GetterFn>
void LRUCache<Key, Data, GetterFn>::setMaxCost( Cost maxCost )
{
	assert( maxCost >= Cost(0) );
	m_maxCost = maxCost;

	limitCost( m_maxCost );
}

template<typename Key, typename Data, typename GetterFn>
typename LRUCache<Key, Data, GetterFn>::Cost LRUCache<Key, Data, GetterFn>::getMaxCost( ) const
{
	return m_maxCost;
}

template<typename Key, typename Data, typename GetterFn>
typename LRUCache<Key, Data, GetterFn>::Cost LRUCache<Key, Data, GetterFn>::currentCost() const
{
	return m_currentCost;
}

template<typename Key, typename Data, typename GetterFn>
bool LRUCache<Key, Data, GetterFn>::get( const Key& key, GetterFn fn, Data &data ) const
{
	typename Cache::iterator it = m_cache.find( key );

	if ( it == m_cache.end() )
	{
		/// Not found in the cache, so compute the data and its associated cost
		Cost cost;
		bool found = fn.get( key, data, cost );

		assert( cost >= Cost(0) );

		if (found)
		{
			if (cost > m_maxCost)
			{
				/// Don't store as we'll exceed the maximum cost immediately.

				return true;
			}

			/// If necessary, clear out any data with a least-recently-used strategy until we
			/// have enough remaining "cost" to store.
			limitCost( m_maxCost - cost );

			/// If there's still room to store.....
			if (m_currentCost + cost <= m_maxCost )
			{
				/// Update the cost to reflect the storage of the new item
				m_currentCost += cost;

				/// Insert the item at the front of the list
				std::pair<Key, DataCost> listEntry( key, DataCost( data, cost ) );
				m_list.push_front( listEntry );

				/// Add the item to the map
				std::pair<typename Cache::iterator, bool> it  = m_cache.insert( typename Cache::value_type( key, m_list.begin() ) );
				assert( it.second );
				assert( m_list.size() == m_cache.size() );

				assert( data == (it.first->second)->second.first );
			}

			return true;
		}
		else
		{
			// Not found
			return false;
		}
	}
	else
	{
		/// Move the entry to the front of the list
		std::pair<Key, DataCost> listEntry = *(it->second);
		m_list.erase( it->second );
		m_list.push_front( listEntry );

		assert( m_list.size() == m_cache.size() );

		/// Update the map to reflect the change in list position
		it->second = m_list.begin();

		/// Return the actual data
		data = (it->second)->second.first;

		return true;
	}
}

template<typename Key, typename Data, typename GetterFn>
void LRUCache<Key, Data, GetterFn>::limitCost( Cost cost ) const
{
	assert( cost > Cost(0) );
	assert( m_list.size() == m_cache.size() );

	while (m_currentCost > cost && m_list.size() )
	{
		m_currentCost -= m_list.back().second.second;

		/// The back of the list contains the LRU entry.
		m_cache.erase( m_list.back().first );
		m_list.pop_back();
	}

	assert( m_list.size() == m_cache.size() );
}


template<typename Key, typename Data, typename GetterFn>
bool LRUCache<Key, Data, GetterFn>::erase( const Key &key )
{
	typename Cache::iterator it = m_cache.find( key );

	if ( it == m_cache.end() )
	{
		return false;
	}

	m_list.erase( it->second );
	m_cache.erase( it );

	assert( m_list.size() == m_cache.size() );

	return true;
}


} // namespace IECore
