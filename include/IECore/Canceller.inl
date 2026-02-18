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

#ifndef IECORE_CANCELLER_INL
#define IECORE_CANCELLER_INL

namespace IECore
{

inline Canceller::Canceller()
	:	m_cancelled( false ), m_cancellationTime( 0 )
{
}

inline void Canceller::cancel()
{
	// Store time of first cancellation. We use `compare_exchange_weak()`
	// to avoid unwanted updates on subsequent calls.
	std::chrono::steady_clock::rep epoch( 0 );
	m_cancellationTime.compare_exchange_weak(
		epoch,
		std::chrono::steady_clock::now().time_since_epoch().count()
	);
	// Set cancellation flag _after_ storing time, so that
	// `elapsedTime()` always sees a valid time.
	if( !m_cancelled.exchange( true ) )
	{
		std::lock_guard lock( m_childrenMutex );
		for( const auto &[child, count] : m_children )
		{
			child->cancel();
		}
	};
}

inline bool Canceller::cancelled() const
{
	/// \todo Can we reduce overhead by reading
	/// with a more relaxed memory ordering here?
	return m_cancelled;
}

inline void Canceller::check( const Canceller *canceller )
{
	if( canceller && canceller->cancelled() )
	{
		throw Cancelled();
	}
}

inline std::chrono::steady_clock::duration Canceller::elapsedTime() const
{
	if( m_cancelled )
	{
		return std::chrono::steady_clock::now() - std::chrono::steady_clock::time_point( std::chrono::steady_clock::duration( m_cancellationTime ) );
	}
	else
	{
		return std::chrono::steady_clock::duration( 0 );
	}
}

inline void Canceller::addChild( const Ptr &child )
{
	std::lock_guard lock( m_childrenMutex );
	m_children[child]++;
	if( m_cancelled )
	{
		child->cancel();
	}
}

inline void Canceller::removeChild( const Ptr &child )
{
	std::lock_guard lock( m_childrenMutex );
	auto it = m_children.find( child );
	if( it != m_children.end() )
	{
		if( --it->second == 0 )
		{
			m_children.erase( it );
		}
	}
}

inline Canceller::ScopedChild::ScopedChild( Canceller *parent, const Ptr &child )
	:	m_parent( parent ), m_child( child )
{
	m_parent->addChild( m_child );
}

inline Canceller::ScopedChild::~ScopedChild()
{
	m_parent->removeChild( m_child );
}

}; // namespace IECore

#endif // IECORE_CANCELLER_INL
