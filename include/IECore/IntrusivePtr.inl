//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include <functional>

#include "boost/assert.hpp"

namespace IECore
{

template<class T> 
IntrusivePtr<T>::IntrusivePtr() : m_p( 0 )
{
}

template<class T> 
IntrusivePtr<T>::IntrusivePtr( T * p ) : m_p( p )
{
	if( m_p != 0 ) 
	{
		intrusive_ptr_add_ref( m_p );
	}
}

template<class T> 
template<typename U>
IntrusivePtr<T>::IntrusivePtr( IntrusivePtr<U> const & rhs ) : m_p( rhs.get() )
{
	if ( m_p != 0 )
	{
		intrusive_ptr_add_ref( m_p );
	}
}

template<class T> 
IntrusivePtr<T>::IntrusivePtr( IntrusivePtr const & rhs) : m_p( rhs.m_p )
{
	if ( m_p != 0 ) 
	{
		intrusive_ptr_add_ref( m_p );
	}
}

template<class T> 
IntrusivePtr<T>::~IntrusivePtr()
{
	if ( m_p != 0 )
	{
		intrusive_ptr_release( m_p );
	}
}

template<class T> 
template<typename U> 
IntrusivePtr<T> & IntrusivePtr<T>::operator=( IntrusivePtr<U> const & rhs)
{
	IntrusivePtr(rhs).swap(*this);
	return *this;
}

template<class T> 
IntrusivePtr<T> & IntrusivePtr<T>::operator=( IntrusivePtr const & rhs)
{
	IntrusivePtr(rhs).swap(*this);
	return *this;
}

template<class T> 
IntrusivePtr<T> & IntrusivePtr<T>::operator=(T * rhs)
{
	IntrusivePtr(rhs).swap(*this);
	return *this;
}

template<class T> 
void IntrusivePtr<T>::reset()
{
	IntrusivePtr().swap( *this );
}

template<class T> 
void IntrusivePtr<T>::reset( T * rhs )
{
	IntrusivePtr( rhs ).swap( *this );
}

template<class T> 
T * IntrusivePtr<T>::get() const
{
	return m_p;
}

template<class T> 
T & IntrusivePtr<T>::operator*() const
{
	BOOST_ASSERT( m_p != 0 );
	return *m_p;
}

template<class T> 
T * IntrusivePtr<T>::operator->() const
{
	BOOST_ASSERT( m_p != 0 );
	return m_p;
}

template<class T> 
IntrusivePtr<T>::operator T *()
{
	return m_p;
}

template<class T> 
IntrusivePtr<T>::operator T const * () const
{
	return m_p;
}

template<class T> 
template<class U> 
inline bool IntrusivePtr<T>::operator==(IntrusivePtr<U> const & b) const
{
	return m_p == b.get();
}

template<class T> 
template<class U> 
inline bool IntrusivePtr<T>::operator!=(IntrusivePtr<U> const & b) const
{
	return m_p != b.get();
}

template<class T> 
template<class U> 
inline bool IntrusivePtr<T>::operator==(U * b) const
{
	return m_p == b;
}

template<class T> 
template<class U> 
inline bool IntrusivePtr<T>::operator!=(U * b) const
{
	return m_p != b;
}

template<class T> 
inline bool IntrusivePtr<T>::operator<(IntrusivePtr const & b) const
{
    return std::less<T *>()(m_p, b.get());
}

template<class T> 
void IntrusivePtr<T>::swap(IntrusivePtr & rhs)
{
	T * tmp = m_p;
	m_p = rhs.m_p;
	rhs.m_p = tmp;
}


template<class T, class U> 
inline bool operator==(T * a, IntrusivePtr<U> const & b)
{
    return a == b.get();
}

template<class T, class U> 
inline bool operator!=(T * a, IntrusivePtr<U> const & b)
{
    return a != b.get();
}

template<class T> 
void swap( IntrusivePtr<T> & lhs, IntrusivePtr<T> & rhs)
{
    lhs.swap(rhs);
}

template<class T> 
T * get_pointer( IntrusivePtr<T> const & p)
{
    return p.get();
}

template<class T, class U> 
IntrusivePtr<T> staticPointerCast( IntrusivePtr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> 
IntrusivePtr<T> constPointerCast( IntrusivePtr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> 
IntrusivePtr<T> dynamicPointerCast(IntrusivePtr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

template<class E, class T, class Y> 
std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, IntrusivePtr<Y> const & p)
{
    os << p.get();
    return os;
}

} // namespace IECore
