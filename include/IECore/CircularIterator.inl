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

#ifndef IECORE_CIRCULARITERATOR_INL
#define IECORE_CIRCULARITERATOR_INL

namespace IECore
{

namespace Detail
{

template<typename Iterator, typename Container>
struct CircularIteratorHelp
{
	static Iterator begin( Iterator b, Container *c )
	{
		return c ? c->begin() : b;
	}
	
	static Iterator end( Iterator b, Container *c )
	{
		return c ? c->end() : b;
	}
};

template<typename Iterator>
struct CircularIteratorHelp<Iterator, void>
{
	static Iterator begin( Iterator b, void *c )
	{
		return b;
	}

	static Iterator end( Iterator b, void *c )
	{
		return b;
	}
};

} // namespace Detail

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::CircularIterator()
	:	m_container( 0 )
{
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::CircularIterator( BaseIterator begin, BaseIterator end )
	:	m_container( 0 ), m_begin( begin ), m_end( end ), m_it( begin )
{
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::CircularIterator( BaseIterator begin, BaseIterator end, BaseIterator position )
	:	m_container( 0 ), m_begin( begin ), m_end( end ), m_it( position )
{
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::CircularIterator( Container *container )
	:	m_container( container ), m_it( container->begin() )
{
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::CircularIterator( Container *container, BaseIterator it )
	:	m_container( container ), m_it( it )
{
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container> &CircularIterator<Iterator, Container>::operator++()
{
	m_it++;
	if( m_it==end() )
	{
		m_it = begin();
	}
	return *this;
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container> CircularIterator<Iterator, Container>::operator++( int )
{
	CircularIterator r = *this;
	++(*this);
	return r;
}

template<typename Iterator, typename Container>
typename CircularIterator<Iterator, Container>::reference CircularIterator<Iterator, Container>::operator*() const
{
	return *m_it;
}

template<typename Iterator, typename Container>
typename CircularIterator<Iterator, Container>::pointer CircularIterator<Iterator, Container>::operator->() const
{
	return &( operator*() );
}

template<typename Iterator, typename Container>
bool CircularIterator<Iterator, Container>::operator==( const CircularIterator &rhs ) const
{
	return m_it==rhs.m_it;
}

template<typename Iterator, typename Container>
bool CircularIterator<Iterator, Container>::operator==( const BaseIterator &rhs ) const
{
	return m_it==rhs;
}

template<typename Iterator, typename Container>
bool CircularIterator<Iterator, Container>::operator!=( const CircularIterator &rhs ) const
{
	return m_it!=rhs.m_it;
}

template<typename Iterator, typename Container>
bool CircularIterator<Iterator, Container>::operator!=( const BaseIterator &rhs ) const
{
	return m_it!=rhs;
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container>::operator BaseIterator() const
{
	return m_it;
}

template<typename Iterator, typename Container>
CircularIterator<Iterator, Container> &CircularIterator<Iterator, Container>::operator=( const BaseIterator &rhs )
{
	if( rhs==end() )
	{
		m_it = begin();
	}
	else
	{
		m_it = rhs;
	}
	return (*this);
}

template<typename Iterator, typename Container>
typename CircularIterator<Iterator, Container>::BaseIterator CircularIterator<Iterator, Container>::begin()
{
	return Detail::CircularIteratorHelp<Iterator, Container>::begin( m_begin, m_container );
}

template<typename Iterator, typename Container>
typename CircularIterator<Iterator, Container>::BaseIterator CircularIterator<Iterator, Container>::end()
{
	return Detail::CircularIteratorHelp<Iterator, Container>::end( m_end, m_container );
}

} // namespace IECore

#endif // IECORE_CIRCULARITERATOR_INL
