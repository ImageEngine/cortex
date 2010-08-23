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

#ifndef IE_CORE_TIMER_H
#define IE_CORE_TIMER_H

#include "boost/timer.hpp"

namespace IECore
{

/// A class useful for simple timing of operations.
/// This simply wraps boost::timer with a more flexible interface
/// that allows for starting and stopping while accumulating
/// elapsed time. Time values returned are in seconds.
class Timer
{

	public :

		/// Creates a new timer. If startAlready
		/// is true then calls start(), otherwise
		/// the timer is created stopped.
		Timer( bool start = true );

		/// Starts the timer. Throws an Exception if
		/// it's already running.
		void start();
		/// Stops the timer. Throws an Exception if
		/// it's not running. Returns the time elapsed
		/// since the last call to start().
		double stop();
		/// Returns true if the timer is running.
		bool running();
		/// Returns the time elapsed since the last call to
		/// start(), or 0 if timer is not running.
		double currentElapsed() const;
		/// Returns the total time this timer has been running
		/// for. This includes previous start()/stop() time periods
		/// and the current period if running() is true.
		double totalElapsed() const;

	private :

		bool m_running;
		double m_accumulated;
		boost::timer m_timer;

};

} // namespace IECore

#endif // IE_CORE_TIMER_H
