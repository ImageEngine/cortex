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

#include "ai.h"

#include "IECore/ClassData.h"
#include "IECore/Exception.h"

#include "IECoreArnold/UniverseBlock.h"

using namespace IECore;
using namespace IECoreArnold;

static int g_count = 0;
static bool g_haveWriter = false;
static ClassData<UniverseBlock, bool> g_writable;

UniverseBlock::UniverseBlock()
{
	// Deprecated constructor existed before the
	// writeable concept, so for backwards compatibility
	// we register as read only.
	init( /* writable = */ false );
}

UniverseBlock::UniverseBlock( bool writable )
{
	init( writable );
}

void UniverseBlock::init( bool writable )
{
	if( writable )
	{
		if( g_haveWriter )
		{
			throw IECore::Exception( "Arnold is already in use" );
		}
		else
		{
			g_haveWriter = true;
		}
	}
	g_writable.create( this, writable );

	g_count++;
	if( AiUniverseIsActive() )
	{
		return;
	}

	AiBegin();

	const char *pluginPaths = getenv( "ARNOLD_PLUGIN_PATH" );
	if( pluginPaths )
	{
		AiLoadPlugins( pluginPaths );
	}
}

UniverseBlock::~UniverseBlock()
{
	if( g_writable[this] )
	{
		g_haveWriter = false;
	}
	g_writable.erase( this );

	if( --g_count == 0 )
	{
		AiEnd();
	}
}

