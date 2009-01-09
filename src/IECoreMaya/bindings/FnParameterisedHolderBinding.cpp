//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MFnDependencyNode.h"

#include "IECore/Object.h"
#include "IECore/Parameterised.h"
#include "IECore/bindings/PointerFromSWIG.h"

#include "IECoreMaya/bindings/FnParameterisedHolderBinding.h"
#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/ParameterisedHolder.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

static ParameterisedHolderInterface *interface( MFnDependencyNode *fnDN )
{
	assert( fnDN );
	
	MPxNode *userNode = fnDN->userNode();
	if( userNode )
	{
		ParameterisedHolderInterface *interface = dynamic_cast<ParameterisedHolderInterface *>( userNode );
		if( interface )
		{
			return interface;
		}
	}
	// failed
	throw Exception( ( MString("Node \"") + fnDN->name() + "\" is not a ParameterisedHolder" ).asChar() );
}

static void setParameterised( MFnDependencyNode *fnDN, ParameterisedPtr p )
{
	assert( fnDN );
	
	StatusException::throwIfError( interface(fnDN)->setParameterised( p ) );
}

static void setParameterised2( MFnDependencyNode *fnDN, const std::string &className, int classVersion, const std::string &envVarName )
{
	assert( fnDN );

	StatusException::throwIfError( interface(fnDN)->setParameterised( className, classVersion, envVarName ) );
}

static boost::python::tuple getParameterised( MFnDependencyNode *fnDN )
{
	assert( fnDN );

	std::string className; int classVersion = 0; std::string searchPath;
	ParameterisedPtr p = interface( fnDN )->getParameterised( &className, &classVersion, &searchPath );
	return boost::python::make_tuple( p, className, classVersion, searchPath );	
}

static void setNodeValues( MFnDependencyNode *fnDN )
{
	assert( fnDN );

	StatusException::throwIfError( interface( fnDN )->setNodeValues() );			
}

static void setNodeValue( MFnDependencyNode *fnDN, ParameterPtr pa )
{	
	assert( fnDN );
	
	StatusException::throwIfError( interface( fnDN )->setNodeValue( pa ) );	
}

static void setParameterisedValues( MFnDependencyNode *fnDN )
{
	assert( fnDN );

	StatusException::throwIfError( interface( fnDN )->setParameterisedValues() );			
}

static void setParameterisedValue( MFnDependencyNode *fnDN, ParameterPtr pa )
{	
	assert( fnDN );

	StatusException::throwIfError( interface( fnDN )->setParameterisedValue( pa ) );	
}

static std::string parameterPlug( MFnDependencyNode *fnDN, ParameterPtr pa )
{
	assert( fnDN );

	// we don't know how to push a swig wrapped MPlug into python,
	// so we have to push the name and then let the python half of
	// MFnParameterisedHolder construct an MPlug from it.
	MString name = interface( fnDN )->parameterPlug( pa ).partialName();
	return name.asChar();
}

static ParameterPtr plugParameter( MFnDependencyNode *fnDN, MPlug *plug )
{
	assert( fnDN );

	return interface( fnDN )->plugParameter( *plug );
}

void IECoreMaya::bindFnParameterisedHolder()
{

	def( "_parameterisedHolderSetParameterised", &setParameterised );
	def( "_parameterisedHolderSetParameterised", &setParameterised2 );
	def( "_parameterisedHolderGetParameterised", &getParameterised );
	def( "_parameterisedHolderSetNodeValues", &setNodeValues );
	def( "_parameterisedHolderSetNodeValue", &setNodeValue );
	def( "_parameterisedHolderSetParameterisedValues", &setParameterisedValues );
	def( "_parameterisedHolderSetParameterisedValue", &setParameterisedValue );
	def( "_parameterisedHolderParameterPlug", &parameterPlug );
	def( "_parameterisedHolderPlugParameter", &plugParameter );
	
	PointerFromSWIG<MFnDependencyNode>();
	PointerFromSWIG<MPlug>();
	
}
