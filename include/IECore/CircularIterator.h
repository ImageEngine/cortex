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

#ifndef IECORE_CIRCULARITERATOR_H
#define IECORE_CIRCULARITERATOR_H

namespace IECore
{

/// The CircularIterator class provides a means of iterating endlessly around
/// a range, cycling back to the beginning once the range is reached. It's simplest
/// instantiation specifies only on an Iterator type, in which case the range may only be
/// specified with begin and end iterators passed to a constructor. Alternatively
/// a Container type may be specified as well, in which case the range may be specified
/// by evaluating the beginning and end of the container whenever needed - this form
/// is of use when items will be removed from the container during iteration.
template<typename Iterator, typename Container=void>
class CircularIterator
{

	public :

		typedef Iterator BaseIterator;
		typedef typename BaseIterator::pointer pointer;
		typedef typename BaseIterator::reference reference;
		typedef typename BaseIterator::value_type value_type;

		/// Uninitialised.
		CircularIterator();
		/// Iteration cycles the range between begin and end, not including end.
		/// Iteration starts at begin. If begin or end become invalid during
		/// iteration then behaviour is undefined.
		CircularIterator( BaseIterator begin, BaseIterator end );
		/// As above but starts iteration at position.
		CircularIterator( BaseIterator begin, BaseIterator end, BaseIterator position );
		/// Iterates over the range container->begin(), container->end(). These
		/// values are evaluated whenever used, so iterators may be removed from the
		/// container provided the current iterator remains valid. Only available
		/// if the Container type is not void.
		CircularIterator( Container *container );
		/// As above but starts iteration at position.
		CircularIterator( Container *container, BaseIterator position );

		CircularIterator &operator++();
		CircularIterator operator++( int );

		reference operator*() const;
		pointer operator->() const;

		bool operator==( const CircularIterator &rhs ) const;
		bool operator==( const BaseIterator &rhs ) const;

		bool operator!=( const CircularIterator &rhs ) const;
		bool operator!=( const BaseIterator &rhs ) const;

		operator BaseIterator() const;

		CircularIterator &operator=( const BaseIterator &rhs );

	private :

		BaseIterator begin();
		BaseIterator end();

		Container *m_container;
		BaseIterator m_begin;
		BaseIterator m_end;
		BaseIterator m_it;

};

} // namespace IECore

#include "IECore/CircularIterator.inl"

#endif // IECORE_CIRCULARITERATOR_H
