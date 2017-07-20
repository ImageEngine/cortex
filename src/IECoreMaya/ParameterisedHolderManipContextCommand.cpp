//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/ParameterisedHolderManipContextCommand.h"

#include "IECoreMaya/ParameterisedHolderManipContext.h"

#include <maya/MArgParser.h>

using namespace IECoreMaya;

ParameterisedHolderManipContextCommand::ParameterisedHolderManipContextCommand()
{
}

void *ParameterisedHolderManipContextCommand::creator()
{
	return new ParameterisedHolderManipContextCommand;
}

MPxContext *ParameterisedHolderManipContextCommand::makeObj()
{
	m_context = new ParameterisedHolderManipContext;
	return m_context;
}


MStatus ParameterisedHolderManipContextCommand::doEditFlags()
{
	 MStatus stat;
	 
	 MArgParser args = parser();
	 
	 if( args.isFlagSet( kModeFlag ) )
	 {
	 	MString mode;
		stat = args.getFlagArgument( kModeFlag, 0, mode );
		if( !stat )
		{
			stat.perror( "Invalid argument passed to " kModeFlagLong " flag. Valid options are: 'all', 'first' or 'targeted'." );
			return stat;
		}
		if( mode == "all" )
		{
			m_context->setMode( ParameterisedHolderManipContext::All );
		}
		else if( mode == "first" )
		{
			m_context->setMode( ParameterisedHolderManipContext::First );
		}
		else if( mode == "targeted" )
		{
			m_context->setMode( ParameterisedHolderManipContext::Targeted );
		}
		else
		{
			stat.perror( "Unknkown mode passed to " kModeFlagLong " flag. Valid options are: 'all', 'first' or 'targeted'." );
			return stat;
		}
	 }
	 
	 if( args.isFlagSet( kTargetFlag ) )
	 {
	 	MString param;
		stat = args.getFlagArgument( kTargetFlag, 0, param );
		if( !stat )
		{
			stat.perror( "Invalid argument passed to " kTargetFlagLong " flag." );
			return stat;
		}
		m_context->setTarget( param );
	 }
	 
	 return MS::kSuccess;
}

MStatus ParameterisedHolderManipContextCommand::doQueryFlags()
{
	MArgParser args = parser();

	if( args.isFlagSet( kModeFlag ) )
	{
		ParameterisedHolderManipContext::Mode m = m_context->getMode();
		switch( m )
		{
			case ParameterisedHolderManipContext::All:
				setResult( MString( "all" ) );
				break;

			case ParameterisedHolderManipContext::First:
				setResult( MString( "first" ) );
				break;
			
			case ParameterisedHolderManipContext::Targeted:
				setResult( MString( "targeted" ) );
				break;
			
			default:	
				setResult( MString( "unknown" ) );
		}		
	}
	
	if( args.isFlagSet( kTargetFlag ) )
	{
		setResult( m_context->getTarget() );
	}

	return MS::kSuccess;
}
		
MStatus ParameterisedHolderManipContextCommand::appendSyntax()
{
	MStatus stat;
	
	MSyntax mySyntax = syntax();
	
	stat = mySyntax.addFlag( kModeFlag, kModeFlagLong, MSyntax::kString );
	if ( !stat )
	{
		return MS::kFailure;
	}
	
	stat = mySyntax.addFlag( kTargetFlag, kTargetFlagLong, MSyntax::kString );
	if ( !stat )
	{
		return MS::kFailure;
	}
	
	return MS::kSuccess;
}





