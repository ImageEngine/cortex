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

#ifndef IECORE_CANCELLER_H
#define IECORE_CANCELLER_H

#include "IECore/Export.h"

#include "boost/noncopyable.hpp"

#include <atomic>
#include <chrono>

namespace IECore
{

/// Exception thrown by `Canceller::check()`.
/// Deliberately _not_ derived from `std::exception`
/// to minimise the chances of it being accidentally
/// suppressed or mistaken for an error. In typical
/// use there is no need to catch exceptions of this
/// type.
struct IECORE_API Cancelled
{
};

/// Class used to cancel long-running background
/// operations.
///
/// Example :
///
/// ```
/// Canceller c;
/// thread t(
///     [&c] {
///         while( 1 ) {
///             Canceller::check( &c );
///         }
///     }
/// );
/// c.cancel();
/// t.join();
/// ```
class Canceller : public boost::noncopyable
{

	public :

		Canceller()
			:	m_cancelled( false ), m_cancellationTime( 0 )
		{
		}

		void cancel()
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
			m_cancelled = true;
		}

		bool cancelled() const
		{
			/// \todo Can we reduce overhead by reading
			/// with a more relaxed memory ordering here?
			return m_cancelled;
		}

		static void check( const Canceller *canceller )
		{
			if( canceller && canceller->cancelled() )
			{
				throw Cancelled();
			}
		}

		/// Returns the time passed since `cancel()` was first called, or `0` if
		/// it has not been called yet.
		std::chrono::steady_clock::duration elapsedTime() const
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

	private :

		std::atomic_bool m_cancelled;
		std::atomic<std::chrono::steady_clock::time_point::rep> m_cancellationTime;

};

}; // namespace IECore

#endif // IECORE_CANCELLER_H
