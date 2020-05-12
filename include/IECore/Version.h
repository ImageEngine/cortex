//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_VERSION_H
#define IECORE_VERSION_H

#include "boost/format.hpp"

#include <string>

#define CORTEX_MILESTONE_VERSION IE_CORE_MILESTONEVERSION
#define CORTEX_MAJOR_VERSION IE_CORE_MAJORVERSION
#define CORTEX_MINOR_VERSION IE_CORE_MINORVERSION
#define CORTEX_PATCH_VERSION IE_CORE_PATCHVERSION

#define MAKE_CORTEX_COMPATIBILITY_VERSION( MILESTONE_VERSION, MAJOR_VERSION ) \
	( MILESTONE_VERSION * 1000 + MAJOR_VERSION  )

#define CORTEX_COMPATIBILITY_VERSION ( MAKE_CORTEX_COMPATIBILITY_VERSION( CORTEX_MILESTONE_VERSION, CORTEX_MAJOR_VERSION ) )

namespace IECore
{

/// Returns the milestone version for the IECore library
IECORE_API inline int milestoneVersion()
{
	return CORTEX_MILESTONE_VERSION;
}

/// Returns the major version for the IECore library
IECORE_API inline int majorVersion()
{
	return CORTEX_MAJOR_VERSION;
}

/// Returns the minor version for the IECore library
IECORE_API inline int minorVersion()
{
	return CORTEX_MINOR_VERSION;
}

/// Returns the patch version for the IECore library
IECORE_API inline int patchVersion()
{
	return CORTEX_PATCH_VERSION;
}

/// Returns an integer representation of the compatibility version for the IECore library
IECORE_API inline int compatibilityVersion()
{
	return CORTEX_COMPATIBILITY_VERSION;
}

/// Returns a string representation of the compatibility version for the IECore library (eg "milestone.major")
IECORE_API inline const std::string &compatibilityVersionString()
{
	static std::string v = boost::str( boost::format( "%d.%d" ) % milestoneVersion() % majorVersion() );
	return v;
}

/// Returns a string of the form "milestone.major.minor.patch"
IECORE_API inline const std::string &versionString()
{
	static std::string v = boost::str( boost::format( "%d.%d.%d.%d" ) % milestoneVersion() % majorVersion() % minorVersion() % patchVersion() );
	return v;
}

} // namespace IECore

#endif // IECORE_VERSION_H
