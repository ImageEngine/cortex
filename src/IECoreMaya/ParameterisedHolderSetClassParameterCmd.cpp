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
#include "maya/MFnStringArrayData.h"
#include "maya/MFnIntArrayData.h"

#include "IECore/Object.h"
#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"

#include "IECoreMaya/ParameterisedHolderSetClassParameterCmd.h"
#include "IECoreMaya/ClassParameterHandler.h"
#include "IECoreMaya/ClassVectorParameterHandler.h"

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

bool ParameterisedHolderSetClassParameterCmd::isUndoable() const
{
	return true;
}

bool ParameterisedHolderSetClassParameterCmd::hasSyntax() const
{
	return false;
}

MStatus ParameterisedHolderSetClassParameterCmd::doIt( const MArgList &argList )
{	
	// get the node and plug we're operating on
	MSelectionList plugSelection;
	plugSelection.add( argList.asString( 0 ) );
	
	MPlug plug;
	plugSelection.getPlug( 0, plug );
	if( plug.isNull() )
	{
		return MS::kFailure;
	}
	
	MObject node = plug.node();
	MFnDependencyNode fnNode( node );
	MPxNode *userNode = fnNode.userNode();
	ParameterisedHolderInterface *parameterisedHolder = dynamic_cast<ParameterisedHolderInterface *>( userNode );
	if( !parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	// get the ClassParameter we're operating on
	IECore::ParameterPtr parameter = parameterisedHolder->plugParameter( plug );
	if( !parameter )
	{
		return MS::kFailure;
	}
	if( !parameter->isInstanceOf( IECore::ClassParameterTypeId ) && !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MS::kFailure;
	}

	// store the details of the class we want to set
	
	if( parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		if( argList.length() != 4 )
		{
			displayError( "ieParameterisedHolderSetClassParameter : wrong number of arguments." );
			return MS::kFailure;
		}
		
		m_newClassNames.append( argList.asString( 1 ) );
		m_newClassVersions.append( argList.asInt( 2 ) );
		m_newSearchPathEnvVar = argList.asString( 3 );
	}
	else
	{
		// ClassVectorParameter
		
		int numClasses = argList.asInt( 1 );
		if( (int)argList.length() != numClasses * 3 + 2 )
		{
			displayError( "ieParameterisedHolderSetClassParameter : wrong number of arguments." );
			return MS::kFailure;		
		}
		
		int argIndex = 2;
		for( int classIndex=0; classIndex<numClasses; classIndex++ )
		{
			m_newParameterNames.append( argList.asString( argIndex++ ) );
			m_newClassNames.append( argList.asString( argIndex++ ) );
			m_newClassVersions.append( argList.asInt( argIndex++ ) );
		}
	}

	// store the details of the class we're about to replace

	if( parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		m_originalClassNames.append( plug.child( 0 ).asString() );
		m_originalClassVersions.append( plug.child( 1 ).asInt() );
		m_originalSearchPathEnvVar = plug.child( 2 ).asString();
	}
	else
	{
		// ClassVectorParameter
		MFnStringArrayData fnSAD( plug.child( 0 ).asMObject() );
		fnSAD.copyTo( m_originalParameterNames );
		
		fnSAD.setObject( plug.child( 1 ).asMObject() );
		fnSAD.copyTo( m_originalClassNames );
		
		MFnIntArrayData fnIAD( plug.child( 2 ).asMObject() );
		fnIAD.copyTo( m_originalClassVersions );
	}
		
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
	
	MStatus s;
	if( m_parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		s = ClassParameterHandler::setClass( m_parameter, plug,
			m_newClassNames[0], m_newClassVersions[0], m_newSearchPathEnvVar );
	}
	else
	{
		// ClassVectorParameter
		s = ClassVectorParameterHandler::setClasses( m_parameter, plug,
			m_newParameterNames, m_newClassNames, m_newClassVersions );
	}
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
	
	MStatus s;
	if( m_parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		s = ClassParameterHandler::setClass( m_parameter, plug,
			m_originalClassNames[0], m_originalClassVersions[0], m_originalSearchPathEnvVar );
	}
	else
	{
		// ClassVectorParameter
				
		s = ClassVectorParameterHandler::setClasses( m_parameter, plug,
			m_originalParameterNames, m_originalClassNames, m_originalClassVersions );
	}
		
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
