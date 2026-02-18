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

#include "IECore/RefCounted.h"

#include <atomic>
#include <chrono>
#include <mutex>

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
/// CancellerPtr c = new Canceller;
/// thread t(
///     [&c] {
///         while( 1 ) {
///             Canceller::check( c.get() );
///         }
///     }
/// );
/// c.cancel();
/// t.join();
/// ```
class IECORE_API Canceller : public IECore::RefCounted
{

	public :

		Canceller();

		IE_CORE_DECLAREMEMBERPTR( Canceller )

		void cancel();
		bool cancelled() const;

		/// Throws `IECore::Cancelled` if `canceller` is non-null and has
		/// been cancelled, otherwise does nothing.
		static void check( const Canceller *canceller );

		/// Returns the time passed since `cancel()` was first called, or `0` if
		/// it has not been called yet.
		std::chrono::steady_clock::duration elapsedTime() const;

		/// Adds a child canceller that will be cancelled automatically
		/// when this is cancelled. If this is already cancels, then the
		/// child is cancelled immediately.
		void addChild( const Ptr &child );

		/// Removes a child canceller. Additions are counted, and actual
		/// removal only occurs when the number of removals equals the number
		/// of additions.
		void removeChild( const Ptr &child );

		/// Convenience class to manage removal of children in an
		/// exception-safe way.
		class IECORE_API ScopedChild : boost::noncopyable
		{

			public :

				/// Adds `child` to `parent`.
				ScopedChild( Canceller *parent, const Ptr &child );
				/// Removes `child` from `parent`.
				~ScopedChild();

			private :

				Canceller *m_parent;
				Ptr m_child;

		};


	private :

		std::atomic_bool m_cancelled;
		std::atomic<std::chrono::steady_clock::time_point::rep> m_cancellationTime;

		std::mutex m_childrenMutex;
		std::unordered_map<Ptr, size_t> m_children;

};

IE_CORE_DECLAREPTR( Canceller )

}; // namespace IECore

#include "IECore/Canceller.inl"

#endif // IECORE_CANCELLER_H
