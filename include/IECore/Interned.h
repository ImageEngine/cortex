//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_INTERNED_H
#define IECORE_INTERNED_H

#include "IECore/HashTable.h"

#include "boost/multi_index_container.hpp"

namespace IECore
{

/// The Interned class provides a means of efficiently storing
/// multiple different objects with the same value. It does this
/// by keeping a static table with the actual values in it, with
/// the object instances just referencing the values in the table.
/// \todo Consider replacing this with the boost flyweight
/// stuff now it's available (boost 1.38.0). Initial investigation
/// suggests the boost implementation to be equivalent in speed but
/// much more flexible. However, without using intermodule_holder
/// we get crashes at program termination, and using it we get gcc
/// crashes during compilation (with gcc 4.0.2).
template<typename T, typename Hash=Hash<T> >
class Interned
{
	public :
	
		Interned( const T &value );
		Interned( const Interned<T, Hash> &other );
		~Interned();

		inline bool operator == ( const Interned<T, Hash> &other ) const;
		inline bool operator < ( const Interned<T, Hash> &other ) const;
		
		inline operator const T & () const;

		const T &value() const;
		
		static size_t size();
		
	private :

		typedef boost::multi_index::multi_index_container<
			T,
			boost::multi_index::indexed_by<
				boost::multi_index::hashed_unique<
					boost::multi_index::identity<T>,
					Hash
				>
			> 
		> HashSet;
		
		typedef typename HashSet::template nth_index<0>::type Index;
		typedef typename HashSet::template nth_index_const_iterator<0>::type ConstIterator;
		
		const T *m_value;
				
		static HashSet *hashSet();

};

typedef Interned<std::string> InternedString;

} // namespace IECore

#include "IECore/Interned.inl"

#endif // IECORE_INTERNED_H
