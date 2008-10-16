//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MGlobal.h"

#include "IECoreMaya/ScopedUndoDisabler.h"

using namespace IECoreMaya;

ScopedUndoDisabler::ScopedUndoDisabler() : m_oldState( false )
{	
	m_oldState = getUndoState();

	if ( m_oldState )
	{
		setUndoState( false );
	}
	
	assert( getUndoState() == false );
}

ScopedUndoDisabler::~ScopedUndoDisabler()
{	
	if ( m_oldState )
	{
		setUndoState( true );	
	}
	
	assert( getUndoState() == m_oldState );
}

bool ScopedUndoDisabler::getUndoState()
{
	int state;
	MStatus s = MGlobal::executeCommand( "undoInfo -query -state", state );
	assert( s );
	
	return state != 0;
}

void ScopedUndoDisabler::setUndoState( bool state )
{	
	MStatus s = MGlobal::executeCommand( MString( "undoInfo -stateWithoutFlush ") + ( state ? "1" : "0" ) );
	assert( s );
	
	assert( getUndoState() == s );
}
