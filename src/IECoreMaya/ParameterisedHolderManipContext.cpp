//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/ParameterisedHolderManipContext.h"
#include "IECoreMaya/ParameterManipContainer.h"
#include "IECoreMaya/ParameterisedHolderInterface.h"

#include <maya/MItSelectionList.h>
#include <maya/MGlobal.h>
#undef None // must come after certain Maya includes which include X11/X.h

#include "IECore/SimpleTypedData.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace IECoreMaya;

ParameterisedHolderManipContext::ParameterisedHolderManipContext() :
	m_toolOn( false ), m_mode( First ), m_targetPlugPath( "" )
{
	setTitleString( "ParameterisedHolder Manipulator" );
}


void ParameterisedHolderManipContext::toolOnSetup( MEvent & )
{
	m_toolOn = true;
	
	updateHelpString();
	
	MStatus status;
	selectionChangeCallback = MModelMessage::addCallback( MModelMessage::kActiveListModified,
									 					  updateManipulators, 
														  this,
														  &status );													  
	if( !status )
	{
		MGlobal::displayError( "ParameterisedHolderManipContext::toolOnSetup() Unable to add "
							   "selectionChanged callback for manipulators." );
	}
	
	updateManipulators( this );
}

void ParameterisedHolderManipContext::toolOffCleanup()
{
	MStatus status = MModelMessage::removeCallback( selectionChangeCallback );
	if( !status ) 
	{
		MGlobal::displayError( "ParameterisedHolderManipContext::toolOffCleanup() Unable to remove "
							   "selectionChanged callback." );
	}
	
	m_toolOn = false;
	
	MPxContext::toolOffCleanup();
}

void ParameterisedHolderManipContext::updateManipulators( void *blindData )
{
	ParameterisedHolderManipContext *ctx = (ParameterisedHolderManipContext *)blindData;
	ctx->updateManipulators();
}

void ParameterisedHolderManipContext::updateManipulators()
{	
	deleteManipulators();

	MStatus stat;
	
	if( m_mode == Targeted && m_targetPlugPath == "" )
	{
		MGlobal::displayError( "ParameterisedHolderManipContext: No target parameter specified to manipulate." );
		return;
	}	
	
	MSelectionList list;
	stat = MGlobal::getActiveSelectionList( list );

	if( !stat )
	{
		MGlobal::displayError( "ParameterisedHolderManipContext::updateManipulators() Unable to get "
							   "the active selection list." );
		return;
	}

	for( MItSelectionList it( list, MFn::kInvalid, &stat ); !it.isDone(); it.next() )
	{
		MObject node;
		it.getDependNode( node );
		dagWalk( node );
	}
		
}

void ParameterisedHolderManipContext::dagWalk( MObject &node )
{
	MStatus stat;
	MFnDagNode dagFn( node, &stat );
	if( !stat )
	{
		processNode( node );
	}
	else
	{
		unsigned int numChildren = dagFn.childCount();
		if( numChildren == 0 )
		{
			processNode( node );
		}
		else
		{
			for( unsigned int i = 0; i < numChildren; i++ )
			{
				MObject child = dagFn.child( i );
				dagWalk( child );
			}
		}
	}
}

void ParameterisedHolderManipContext::processNode( MObject &node )
{
	MStatus stat;
	MFnDependencyNode nodeFn( node, &stat );
	if( !stat )
	{
		return;
	}	
	
	ParameterisedHolderInterface *pHolder = dynamic_cast<ParameterisedHolderInterface *>( nodeFn.userNode() );		
	if( !pHolder ) 
	{
		return;
	}

	if( m_mode == Targeted )
	{
		MStatus stat;
		MPlug targetPlug = nodeFn.findPlug( m_targetPlugPath, &stat );
		if( stat )
		{
			createAndConnectManip( pHolder->plugParameter( targetPlug ), nodeFn );	
		}
	}
	else
	{
		ParameterisedInterface *parameterisedInterface = pHolder->getParameterisedInterface();
		if( parameterisedInterface )
		{
			createManipulatorWalk( parameterisedInterface->parameters(), nodeFn );
		}
	}
}


MPxManipContainer *ParameterisedHolderManipContext::createManipulatorWalk( ParameterPtr parameter, MFnDependencyNode &nodeFn )
{
	if( CompoundParameterPtr c = runTimeCast<CompoundParameter>( parameter ) )
	{
		MPxManipContainer *manip = 0;
		for( CompoundParameter::ParameterVector::const_iterator it = c->orderedParameters().begin(); it != c->orderedParameters().end(); it++ )
		{	
			manip = createManipulatorWalk( *it, nodeFn );
			if( m_mode == First && manip )
			{
				return manip;
			}
		}
		return manip;
	}
	else
	{
		return createAndConnectManip( parameter, nodeFn );
	}
}

MPxManipContainer *ParameterisedHolderManipContext::createAndConnectManip( ParameterPtr parameter, MFnDependencyNode &nodeFn )
{
	if( !parameter )
	{
		return 0;
	}
	
	// The 'name' of the manipulator to create is: ie<manipulatorTypeHint><parameterTypeName>Manipulator
	MString manipLabel( "" );
	MString manipName( "ie" );
	
	CompoundObjectPtr userData = parameter->userData();
	if( CompoundObjectPtr manipData = userData->member<CompoundObject>( "UI" ) )
	{
		// Allow the parameter to opt-out of having a manipulator
		if( BoolDataPtr enableData = manipData->member<BoolData>( "disableManip" ) )
		{
			if( enableData->readable() == true )
			{
				return 0;
			}
		}	
		
		if( StringDataPtr labelData = manipData->member<StringData>( "manipLabel" ) )
		{
			manipLabel += MString( labelData->readable().c_str() );
		}	
		
		if( StringDataPtr hintData = manipData->member<StringData>( "manipTypeHint" ) )
		{
			manipName += MString( hintData->readable().c_str() );
		}	
	}
	
	manipName += parameter->typeName();
	manipName += "Manipulator";
		
	MObject manipObj;
	MPxManipContainer *manip = MPxManipContainer::newManipulator( manipName, manipObj );
	
	if( !manip )
	{
		return 0;
	}
	
	// If we are derived from our custom manipulator base, then we can set the 
	// desired plug name into the manipulator, incase it wishes to use it.
	ParameterManipContainer *paramManip = dynamic_cast<ParameterManipContainer *>( manip );
	if( paramManip )
	{
		ParameterisedHolderInterface *pHolder = dynamic_cast<ParameterisedHolderInterface *>( nodeFn.userNode() );	
		MPlug targetPlug = pHolder->parameterPlug( parameter );
		paramManip->setPlug( targetPlug );
		if( manipLabel != "" )
		{
			paramManip->setLabel( manipLabel );
		}
	}
		
	addManipulator( manipObj );
	MStatus stat = manip->connectToDependNode( nodeFn.object() );
	
	if( !stat )
	{
		MGlobal::displayError( "ParameterisedHolderManipContext::createAndConnectManip() Unable to connect manipulator." );
		return 0;
	}
	
	return manip;
}


void ParameterisedHolderManipContext::setTarget( MString &plugName )
{
	m_targetPlugPath = plugName;
	if( m_toolOn )
	{
		updateHelpString();
		updateManipulators();
	}
}

MString ParameterisedHolderManipContext::getTarget()
{
	return m_targetPlugPath;
}

void ParameterisedHolderManipContext::setMode( ParameterisedHolderManipContext::Mode m )
{
	m_mode = m;
	if( m_toolOn )
	{
		updateHelpString();
		updateManipulators();
	}
}

ParameterisedHolderManipContext::Mode ParameterisedHolderManipContext::getMode()
{
	return m_mode;
}

void ParameterisedHolderManipContext::updateHelpString()
{
	switch( m_mode )
	{
		case First:
			setHelpString( "Adjusting the first manipulatable parameter on the selection." );
			break;
			
		case All:
			setHelpString( "Adjusting all manipulatable parameter on the selection." );
			break;	
			
		case Targeted:
			setHelpString( "Adjusting the parameter named '" + m_targetPlugPath + "' on the selection, if available." );
			break;	
	}
}





