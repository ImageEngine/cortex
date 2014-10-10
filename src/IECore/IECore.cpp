//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/IECore.h"

#include "boost/format.hpp"

namespace IECore
{

int majorVersion()
{
	return IE_CORE_MAJORVERSION;
}

int minorVersion()
{
	return IE_CORE_MINORVERSION;
}

int patchVersion()
{
	return IE_CORE_PATCHVERSION;
}

const std::string &versionString()
{
	static std::string v;
	if( !v.size() )
	{
		v = boost::str( boost::format( "%d.%d.%d" ) % majorVersion() % minorVersion() % patchVersion() );
	}
	return v;
}

bool withASIO()
{
#ifdef IECORE_WITH_ASIO
	return true;
#else
	return false;
#endif
}

bool withSignals()
{
#ifdef IECORE_WITH_SIGNALS
	return true;
#else
	return false;
#endif
}

bool withBoostFactorial()
{
#ifdef IECORE_WITH_BOOSTFACTORIAL
	return true;
#else
	return false;
#endif
}

bool withTIFF()
{
#ifdef IECORE_WITH_TIFF
	return true;
#else
	return false;
#endif
}

bool withJPEG()
{
#ifdef IECORE_WITH_JPEG
	return true;
#else
	return false;
#endif
}

bool withFreeType()
{
#ifdef IECORE_WITH_FREETYPE
	return true;
#else
	return false;
#endif
}

bool withPNG()
{
#ifdef IECORE_WITH_PNG
	return true;
#else
	return false;
#endif
}

bool withDeepEXR()
{
#ifdef IECORE_WITH_DEEPEXR
	return true;
#else
	return false;
#endif
}

}
