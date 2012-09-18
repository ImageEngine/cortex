//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/SOP_ParameterisedHolder.h"
#include "IECoreHoudini/FnParameterisedHolder.h"

using namespace IECoreHoudini;

FnParameterisedHolder::FnParameterisedHolder( SOP_Node *sop ) : m_handle()
{
	if ( !sop )
	{
		return;
	}

	if ( getHolder( sop ) )
	{
		setHolder( sop );
	}
	else
	{
		UT_String path;
		sop->getFullPath( path );
		std::cerr << path << " was not a valid ieParameterisedHolder!" << std::endl;
	}
}

FnParameterisedHolder::~FnParameterisedHolder()
{
}

bool FnParameterisedHolder::hasParameterised()
{
	if ( hasHolder() )
	{
		SOP_ParameterisedHolder *holder = getHolder( static_cast<SOP_Node*>( m_handle.node() ) );
		if ( holder )
		{
			return holder->hasParameterised();
		}
	}
	
	return false;
}

void FnParameterisedHolder::setParameterised( IECore::RunTimeTypedPtr p )
{
	if ( !p || !hasHolder() )
	{
		return;
	}
	
	SOP_ParameterisedHolder *holder = getHolder( static_cast<SOP_Node*>( m_handle.node() ) );
	if ( !holder )
	{
		return;
	}
	
	holder->setParameterised( p );
}

void FnParameterisedHolder::setParameterised( const std::string &className, int classVerison, const std::string &seachPathEnvVar )
{
	if ( !hasHolder() )
	{
		return;
	}
	
	SOP_ParameterisedHolder *holder = getHolder( static_cast<SOP_Node*>( m_handle.node() ) );
	if ( !holder )
	{
		return;
	}
	
	holder->setParameterised( className, classVerison, seachPathEnvVar );
}

IECore::RunTimeTypedPtr FnParameterisedHolder::getParameterised()
{
	if ( hasHolder() )
	{
		SOP_ParameterisedHolder *holder = getHolder( static_cast<SOP_Node*>( m_handle.node() ) );
		if ( holder )
		{
			return holder->getParameterised();
		}
	}
	
	return 0;
}

bool FnParameterisedHolder::hasHolder()
{
	return m_handle.alive();
}

void FnParameterisedHolder::setHolder( SOP_Node *sop )
{
	m_handle = sop;
}

SOP_ParameterisedHolder *FnParameterisedHolder::getHolder( SOP_Node *sop )
{
	if ( sop )
	{
		return dynamic_cast<SOP_ParameterisedHolder*>( sop );
	}

	return 0;
}
