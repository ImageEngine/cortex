//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "boost/python.hpp"

#include "IECoreArnold/UniverseBlock.h"

using namespace boost::python;
using namespace IECoreArnold;

namespace
{

object universeWrapper( UniverseBlock &universeBlock )
{
	AtUniverse *universe = universeBlock.universe();
	if( universe )
	{
		object arnold = import( "arnold" );
		object ctypes = import( "ctypes" );
		return ctypes.attr( "cast" )(
			(uint64_t)universeBlock.universe(),
			ctypes.attr( "POINTER" )( object( arnold.attr( "AtUniverse" ) ) )
		);
	}
	else
	{
		// Default universe, represented as `None` in Python.
		return object();
	}
}

} // namespace

namespace IECoreArnold
{

void bindUniverseBlock()
{

	// This is bound with a preceding _ and then turned into a context
	// manager for the "with" statement in IECoreArnold/UniverseBlock.py
	class_<UniverseBlock, boost::noncopyable>( "_UniverseBlock", init<bool>( ( arg( "writable" ) ) ) )
		.def( "universe", &universeWrapper )
	;

}

} // namespace IECoreArnold
