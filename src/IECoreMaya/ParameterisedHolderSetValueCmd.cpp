//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/ParameterisedHolderSetValueCmd.h"

using namespace IECoreMaya;

ParameterisedHolderSetValueCmd::ParameterisedHolderSetValueCmd()
	:	m_parameterisedHolder( 0 ), m_parameter( 0 )
{
}

ParameterisedHolderSetValueCmd::~ParameterisedHolderSetValueCmd()
{
}

void *ParameterisedHolderSetValueCmd::creator()
{
	return new ParameterisedHolderSetValueCmd;
}

MSyntax ParameterisedHolderSetValueCmd::newSyntax()
{
	MSyntax s;

	s.addFlag( "p", "plug", MSyntax::kString );

	s.setObjectType( MSyntax::kSelectionList, 1, 1 );

	return s;
}

bool ParameterisedHolderSetValueCmd::isUndoable() const
{
	return true;
}

MStatus ParameterisedHolderSetValueCmd::doIt( const MArgList &argList )
{
	MArgDatabase args( syntax(), argList );

	// get the node we're operating on
	MSelectionList objects;
	args.getObjects( objects );
	MStringArray selStr; objects.getSelectionStrings( selStr );
	assert( objects.length()==1 );
	MObject node;
	objects.getDependNode( 0, node );
	MFnDependencyNode fnNode( node );
	MPxNode *userNode = fnNode.userNode();
	m_parameterisedHolder = dynamic_cast<ParameterisedHolderInterface *>( userNode );
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	// see if we're being asked to operate on a specific parameter or
	// just the whole thing.
	MString plugName = args.flagArgumentString( "plug", 0 );
	if( plugName!="" )
	{
		MPlug plug = fnNode.findPlug( plugName );
		if( plug.isNull() )
		{
			m_parameterisedHolder = 0; // so we won't attempt anything in undoIt or redoIt
			return MS::kFailure;
		}
		m_parameter = m_parameterisedHolder->plugParameter( plug );
		if( !m_parameter )
		{
			m_parameterisedHolder = 0; // so we won't attempt anything in undoIt or redoIt
			return MS::kFailure;
		}
	}

	// store the values we'll be setting in doIt and undoIt

	IECore::ParameterisedInterface *interface = m_parameterisedHolder->getParameterisedInterface();
	IECore::ParameterPtr parameter = m_parameter ? m_parameter : interface->parameters();

	// we must copy the value here, as the m_originalValue = parm->getValue() below updates the value in place,
	// which would modify tmp if it weren't a copy.
	IECore::CompoundObjectPtr tmp = boost::static_pointer_cast<IECore::CompoundObject>( parameter->getValue() )->copy();

		m_newValue = parameter->getValue()->copy();
		if( m_parameter )
		{
			m_parameterisedHolder->setParameterisedValue( parameter );
		}
		else
		{
			m_parameterisedHolder->setParameterisedValues();
		}
		m_originalValue = parameter->getValue();

	parameter->setValue( tmp );

	return redoIt();
}

MStatus ParameterisedHolderSetValueCmd::redoIt()
{
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	IECore::ParameterisedInterface *interface = m_parameterisedHolder->getParameterisedInterface();
	IECore::ParameterPtr parameter = m_parameter ? m_parameter : interface->parameters();

	IECore::ObjectPtr tmp = parameter->getValue();

		parameter->setValue( m_newValue );
		if( m_parameter )
		{
			m_parameterisedHolder->setNodeValue( parameter );
		}
		else
		{
			m_parameterisedHolder->setNodeValues();
		}

	parameter->setValue( tmp );

	return MStatus::kSuccess;
}

MStatus ParameterisedHolderSetValueCmd::undoIt()
{
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	IECore::ParameterisedInterface *interface = m_parameterisedHolder->getParameterisedInterface();
	IECore::ParameterPtr parameter = m_parameter ? m_parameter : interface->parameters();

	IECore::ObjectPtr tmp = parameter->getValue();

		parameter->setValue( m_originalValue );
		if( m_parameter )
		{
			m_parameterisedHolder->setNodeValue( parameter );
		}
		else
		{
			m_parameterisedHolder->setNodeValues();
		}

	parameter->setValue( tmp );

	return MStatus::kSuccess;
}
