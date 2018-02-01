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

#ifndef IECOREARNOLD_UNIVERSEBLOCK_H
#define IECOREARNOLD_UNIVERSEBLOCK_H

#include "IECoreArnold/Export.h"

#include "boost/noncopyable.hpp"

namespace IECoreArnold
{

/// Manages the Arnold universe. This is problematic because there
/// can be only one instance at a time, but many applications have
/// need for more than one.
class IECOREARNOLD_API UniverseBlock : public boost::noncopyable
{

	public :

		/// Ensures that the Arnold universe has been created and
		/// that all plugins and metadata files on the ARNOLD_PLUGIN_PATH
		/// have been loaded. If writable is true, then throws if
		/// there is already a writer.
		UniverseBlock( bool writable );
		/// "Releases" the universe. Currently we only actually
		/// call `AiEnd()` for writable universes, because it is
		/// essential to clean them up properly. We leave readable
		/// universes active to avoid the startup cost the next
		/// time around.
		~UniverseBlock();

	private :

		bool m_writable;

};

} // namespace IECoreArnold

#endif // IECOREARNOLD_UNIVERSEBLOCK_H
