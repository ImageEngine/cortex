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

#include "maya/MArgList.h"
#include "maya/MSyntax.h"
#include "maya/MArgDatabase.h"
#include "maya/MSelectionList.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MStringArray.h"

#include "IECore/Object.h"
#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"

#include "IECoreMaya/ParameterisedHolderSetClassParameterCmd.h"
#include "IECoreMaya/ClassParameterHandler.h"

using namespace IECoreMaya;

ParameterisedHolderSetClassParameterCmd::ParameterisedHolderSetClassParameterCmd()
	:	m_parameterisedHolder( 0 ), m_parameter( 0 ), m_originalValues( 0 )
{
}

ParameterisedHolderSetClassParameterCmd::~ParameterisedHolderSetClassParameterCmd()
{
}

void *ParameterisedHolderSetClassParameterCmd::creator()
{
	return new ParameterisedHolderSetClassParameterCmd;
}

MSyntax ParameterisedHolderSetClassParameterCmd::newSyntax()
{
	MSyntax s;

	s.addFlag( "p", "plug", MSyntax::kString );
	s.addFlag( "c", "className", MSyntax::kString );
	s.addFlag( "v", "classVersion", MSyntax::kString );
	s.addFlag( "s", "searchPathEnvVar", MSyntax::kString );

	s.setObjectType( MSyntax::kSelectionList, 1, 1 );

	return s;
}

bool ParameterisedHolderSetClassParameterCmd::isUndoable() const
{
	return true;
}

MStatus ParameterisedHolderSetClassParameterCmd::doIt( const MArgList &argList )
{
	MArgDatabase args( syntax(), argList );
	
	// get the node we're operating on
	MSelectionList objects;
	args.getObjects( objects );
	
	MObject node;
	objects.getDependNode( 0, node );
	MFnDependencyNode fnNode( node );
	MPxNode *userNode = fnNode.userNode();
	ParameterisedHolderInterface *parameterisedHolder = dynamic_cast<ParameterisedHolderInterface *>( userNode );
	if( !parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	// get the ClassParameter we're operating on
	MString plugName = args.flagArgumentString( "plug", 0 );
	if( plugName=="" )
	{
		return MStatus::kFailure;
	}
	
	MPlug plug = fnNode.findPlug( plugName );
	if( plug.isNull() )
	{
		return MS::kFailure;
	}
	IECore::ParameterPtr parameter = parameterisedHolder->plugParameter( plug );
	if( !parameter )
	{
		return MS::kFailure;
	}
	if( !parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		return MS::kFailure;
	}

	// store the details of the class we're about to replace

	m_originalClassName = plug.child( 0 ).asString();
	m_originalClassVersion = plug.child( 1 ).asInt();
	m_originalSearchPathEnvVar = plug.child( 2 ).asString();

	// store the details of the class we want to set
	
	m_newClassName = args.flagArgumentString( "className", 0 );
	m_newClassVersion = args.flagArgumentInt( "classVersion", 0 );
	m_newSearchPathEnvVar = args.flagArgumentString( "searchPathEnvVar", 0 );
	
	parameterisedHolder->setParameterisedValues();
	
	m_originalValues = parameter->getValue()->copy();
	
	m_parameterisedHolder = parameterisedHolder;
	m_parameter = parameter;

	return redoIt();
}

MStatus ParameterisedHolderSetClassParameterCmd::redoIt()
{
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	MPlug plug = m_parameterisedHolder->parameterPlug( m_parameter );
	MStatus s = ClassParameterHandler::setClass( m_parameter, plug,
		m_newClassName, m_newClassVersion, m_newSearchPathEnvVar );
		
	if( !s )
	{
		return s;
	}
	
	return m_parameterisedHolder->updateParameterised();
}

MStatus ParameterisedHolderSetClassParameterCmd::undoIt()
{
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	MPlug plug = m_parameterisedHolder->parameterPlug( m_parameter );
	MStatus s = ClassParameterHandler::setClass( m_parameter, plug,
		m_originalClassName, m_originalClassVersion, m_originalSearchPathEnvVar );
		
	if( !s )
	{
		return s;
	}
	
	m_parameter->setValue( m_originalValues );
	
	s = m_parameterisedHolder->updateParameterised();
		
	if( !s )
	{
		return s;
	}
	
	return m_parameterisedHolder->setNodeValues();
}
