//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_ENTITYPTR_H
#define IECOREAPPLESEED_ENTITYPTR_H

#include "boost/noncopyable.hpp"

#include "renderer/api/entity.h"

namespace IECoreAppleseed
{

/// Smart ptr that holds an appleseed entity and keeps track of ownership.
template <typename T>
class EntityPtr
  : public boost::noncopyable
{
  public:
	EntityPtr()
	  : m_ptr( nullptr )
	  , m_releaseObj( false )
	{
	}

	explicit EntityPtr( foundation::auto_release_ptr<T> ptr )
	  : m_ptr( ptr.release() )
	  , m_releaseObj( true )
	{
	}

	EntityPtr( T* ptr, const bool release )
	  : m_ptr( ptr )
	  , m_releaseObj( release )
	{
	}

	~EntityPtr()
	{
		if ( m_releaseObj )
			m_ptr->release();
	}

	void reset()
	{
		if ( m_releaseObj )
			m_ptr->release();

		m_releaseObj = false;
		m_ptr = nullptr;
	}

	void reset( foundation::auto_release_ptr<T> ptr )
	{
		reset();
		m_releaseObj = true;
		m_ptr = ptr.release();
	}

	template <typename U>
	void reset( foundation::auto_release_ptr<U> ptr )
	{
		reset();
		m_releaseObj = true;
		m_ptr = static_cast<T*>( ptr.release() );
	}

	void reset( T *ptr, bool release )
	{
		reset();
		m_releaseObj = release;
		m_ptr = ptr;
	}

	EntityPtr& operator=( foundation::auto_release_ptr<T> ptr )
	{
		reset( ptr );
		return *this;
	}

	foundation::auto_release_ptr<T> release()
	{
		assert( m_releaseObj );

		m_releaseObj = false;
		return foundation::auto_release_ptr<T>( m_ptr );
	}

	template <typename U>
	foundation::auto_release_ptr<U> releaseAs()
	{
		assert( m_releaseObj );

		m_releaseObj = false;
		return foundation::auto_release_ptr<U>( m_ptr );
	}

	T& operator*() const noexcept
	{
		assert( m_ptr );
		return *m_ptr;
	}
	T* operator->() const noexcept
	{
		assert( m_ptr );
		return m_ptr;
	}

	T *get() const noexcept
	{
		return m_ptr;
	}

  private:
	T *m_ptr;
	bool m_releaseObj;
};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_ENTITYPTR_H
