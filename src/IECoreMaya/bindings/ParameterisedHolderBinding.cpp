//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"
#include <iostream>
#include <cassert>

#include "maya/MSelectionList.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnDagNode.h"

#include "IECore/MessageHandler.h"
#include "IECore/Parameterised.h"

#include "IECoreMaya/bindings/ParameterisedHolderBinding.h"
#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/bindings/PlugBinding.h"

#include "IECore/Object.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

ParameterisedHolderWrapper::ParameterisedHolderWrapper( const MObject &object )
	:	Node( object )
{
	initInterface();
}

ParameterisedHolderWrapper::ParameterisedHolderWrapper( const char *name )
	:	Node( name )
{
	initInterface();
}

void ParameterisedHolderWrapper::initInterface()
{
	MFnDependencyNode fnDN( object() );
	MPxNode *userNode = fnDN.userNode();
	if( userNode )
	{
		ParameterisedHolderInterface *interface = dynamic_cast<ParameterisedHolderInterface *>( userNode );
		if( interface )
		{
			m_interface = interface;
			return;
		}
	}
	// failed
	throw Exception( "Object is not a ParameterisedHolder" );
}

void ParameterisedHolderWrapper::setParameterised( IECore::ParameterisedPtr parameterised )
{
	object(); // to check it's alive
	StatusException::throwIfError( m_interface->setParameterised( parameterised ) );
	return;
}

void ParameterisedHolderWrapper::setParameterised( const std::string &className, int classVersion, const std::string &envVarName )
{
	object(); // to check it's alive
	StatusException::throwIfError( m_interface->setParameterised( className, classVersion, envVarName ) );
	return;
}

boost::python::tuple ParameterisedHolderWrapper::getParameterised()
{
	object(); // to check it's alive
	std::string className; int classVersion = 0; std::string searchPath;
	ParameterisedPtr p = m_interface->getParameterised( &className, &classVersion, &searchPath );
	return boost::python::make_tuple( p, className, classVersion, searchPath );	
}

void ParameterisedHolderWrapper::setNodeValues()
{
	object(); // to check it's alive
	StatusException::throwIfError( m_interface->setNodeValues() );			
}

void ParameterisedHolderWrapper::setParameterisedValues()
{
	object(); // to check it's alive
	StatusException::throwIfError( m_interface->setParameterisedValues() );			
}

Plug ParameterisedHolderWrapper::parameterPlug( ParameterPtr pa )
{
	object(); // to check it's alive

	return Plug( m_interface->parameterPlug( pa ) );
}

ParameterPtr ParameterisedHolderWrapper::plugParameter( const char *plugName )
{
	MObject o = object();
	MFnDependencyNode fnDN( o );
	MObject attribute = fnDN.attribute( plugName );
	MPlug plug( o, attribute );
	return m_interface->plugParameter( plug );
}

/// \todo Move the implementation of this into the ParameterisedHolder itself on the next major version change
void ParameterisedHolderWrapper::setNodeValue( ParameterPtr pa )
{
	object(); // to check it's alive
	
	StatusException::throwIfError( m_interface->setNodeValue( pa ) );	
}

/// \todo Move the implementation of this into the ParameterisedHolder itself on the next major version change
void ParameterisedHolderWrapper::setParameterisedValue( ParameterPtr pa )
{
	object(); // to check it's alive
	
	StatusException::throwIfError( m_interface->setParameterisedValue( pa ) );	
}

void IECoreMaya::bindParameterisedHolder()
{
	void (ParameterisedHolderWrapper::*setParameterised1)( IECore::ParameterisedPtr ) = &ParameterisedHolderWrapper::setParameterised;
	void (ParameterisedHolderWrapper::*setParameterised2)( const std::string &, int , const std::string & ) = &ParameterisedHolderWrapper::setParameterised;	
	
	class_<ParameterisedHolderWrapper, boost::noncopyable, bases<Node> >( "ParameterisedHolder", init<const char *>() )
		.def( "getParameterised", &ParameterisedHolderWrapper::getParameterised )
		.def( "setParameterised", setParameterised1 )
		.def( "setParameterised", setParameterised2 )
		.def( "setNodeValues", &ParameterisedHolderWrapper::setNodeValues )
		.def( "setParameterisedValues", &ParameterisedHolderWrapper::setParameterisedValues )
		.def( "parameterPlug", &ParameterisedHolderWrapper::parameterPlug )
		.def( "plugParameter", &ParameterisedHolderWrapper::plugParameter )
		
		.def( "setNodeValue", &ParameterisedHolderWrapper::setNodeValue )
		.def( "setParameterisedValue", &ParameterisedHolderWrapper::setParameterisedValue )
	;
}
