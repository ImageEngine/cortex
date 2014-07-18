//////////////////////////////////////////////////////////////////////////
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

#include "boost/python.hpp"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MGlobal.h"

#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreMaya/ClassParameterHandler.h"

using namespace IECoreMaya;
using namespace boost::python;

static ParameterHandler::Description<ClassParameterHandler> registrar( IECore::ClassParameterTypeId );

MStatus ClassParameterHandler::setClass( IECore::ParameterPtr parameter, const MString &className, int classVersion, const MString &searchPathEnvVar )
{	
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( parameter );
		pythonParameter.attr( "setClass" )( className.asChar(), classVersion, searchPathEnvVar.asChar() );
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
		return MS::kFailure;
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassParameterHandler::setClass : " ) + e.what() );
		return MS::kFailure;
	}
	return MS::kSuccess;
}

MStatus ClassParameterHandler::getClass( IECore::ConstParameterPtr parameter, MString &className, int &classVersion, MString &searchPathEnvVar )
{
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( boost::const_pointer_cast<IECore::Parameter>( parameter ) );
		boost::python::tuple classInfo = extract<tuple>( pythonParameter.attr( "getClass" )( true ) );
		
		className = extract<const char *>( classInfo[1] );
		classVersion = extract<int>( classInfo[2] );
		searchPathEnvVar = extract<const char *>( classInfo[3] );
		
		return MS::kSuccess;
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassParameterHandler::getClass : " ) + e.what() );
	}
	return MS::kFailure;
}

void ClassParameterHandler::currentClass( const MPlug &plug, MString &className, int &classVersion, MString &searchPathEnvVar )
{
	MObject attribute = plug.attribute();
	MFnTypedAttribute fnTAttr( attribute );
	if ( !fnTAttr.hasObj( attribute ) || fnTAttr.attrType() != MFnData::kStringArray )
	{
		// compatibility for the deprecated compound plug behaviour
		className = plug.child( 0 ).asString();
		classVersion = plug.child( 1 ).asInt();
		searchPathEnvVar = plug.child( 2 ).asString();
		return;
	}
	
	MFnStringArrayData fnSAD( plug.asMObject() );
	if ( fnSAD.length() == 0 )
	{
		className = "";
		classVersion = 0;
		searchPathEnvVar = "";
		return;
	}
	
	if ( fnSAD.length() != 3 )
	{
		throw( IECore::InvalidArgumentException( ( plug.name() + " has more than 3 values. Expected name, version, searchPath only." ).asChar() ) );
	}

	MStringArray storedClassInfo = fnSAD.array();
	if ( !storedClassInfo[1].isInt() )
	{
		throw( IECore::InvalidArgumentException( ( "Second value of " + plug.name() + " must represent an integer" ).asChar() ) );
	}
	
	className = storedClassInfo[0];
	classVersion = storedClassInfo[1].asInt();
	searchPathEnvVar = storedClassInfo[2];
}

MStatus ClassParameterHandler::doRestore( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	MString className;
	int classVersion;
	MString searchPathEnvVar;
	
	try
	{
		currentClass( plug, className, classVersion, searchPathEnvVar );
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassParameterHandler::doRestore : " ) + e.what() );
		return MS::kFailure;
	}
	
	return setClass( parameter, className, classVersion, searchPathEnvVar );
}
				
MStatus ClassParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	// compatibility for the deprecated compound plug behaviour: should return MS::kFailure
	MFnTypedAttribute fnTAttr( attribute );
	if ( !fnTAttr.hasObj( attribute ) || fnTAttr.attrType() != MFnData::kStringArray )
	{
		MFnCompoundAttribute fnCAttr( attribute );
		if( !fnCAttr.hasObj( attribute ) )
		{
			return MS::kFailure;
		}
		
		if( fnCAttr.numChildren()!=3 )
		{
			return MS::kFailure;
		}
		
		MObject classNameAttr = fnCAttr.child( 0 );
		MFnTypedAttribute fnTAttr( classNameAttr );
		if( !fnTAttr.hasObj( classNameAttr ) )
		{
			return MS::kFailure;	
		}
		if( fnTAttr.name() != fnCAttr.name() + "__className" )
		{
			return MS::kFailure;
		}
		if( fnTAttr.attrType()!=MFnData::kString )
		{
			return MS::kFailure;
		}
		
		MObject classVersionAttr = fnCAttr.child( 1 );
		MFnNumericAttribute fnNAttr( classVersionAttr );
		if( !fnNAttr.hasObj( classVersionAttr ) )
		{
			return MS::kFailure;
		}
		if( fnNAttr.name() != fnCAttr.name() + "__classVersion" )
		{
			return MS::kFailure;
		}
		if( fnNAttr.unitType() != MFnNumericData::kInt )
		{
			return MS::kFailure;
		}
		
		MObject searchPathEnvVarAttr = fnCAttr.child( 2 );
		fnTAttr.setObject( searchPathEnvVarAttr );
		if( !fnTAttr.hasObj( searchPathEnvVarAttr ) )
		{
			return MS::kFailure;	
		}
		if( fnTAttr.name() != fnCAttr.name() + "__searchPathEnvVar" )
		{
			return MS::kFailure;
		}
		if( fnTAttr.attrType()!=MFnData::kString )
		{
			return MS::kFailure;
		}
		
		if( !storeClass( parameter, plug ) )
		{
			return MS::kFailure;
		}
	}
	
	if( !storeClass( parameter, plug ) )
	{
		return MS::kFailure;
	}
	
	return finishUpdating( parameter, plug );
}

MPlug ClassParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		return MPlug();
	}
	
	MFnTypedAttribute fnTAttr;
	MObject attribute = fnTAttr.create( plugName, plugName, MFnData::kStringArray );
	MPlug result = finishCreating( parameter, attribute, node );
	
	if( !storeClass( parameter, result ) )
	{
		return MPlug(); // failure
	}
	
	if( !finishUpdating( parameter, result ) )
	{
		return MPlug(); // failure
	}
	
	return result;
}

MStatus ClassParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		return MS::kFailure;
	}
	return MS::kSuccess;
}

MStatus ClassParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassParameterTypeId ) )
	{
		return MS::kFailure;
	}
	
	return MS::kSuccess;
}

MStatus ClassParameterHandler::storeClass( IECore::ConstParameterPtr parameter, MPlug &plug )
{
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( boost::const_pointer_cast<IECore::Parameter>( parameter ) );
		boost::python::object classInfo = pythonParameter.attr( "getClass" )( true );
	
		std::string className = boost::python::extract<std::string>( classInfo[1] );
		int classVersion = boost::python::extract<int>( classInfo[2] );
		std::string searchPathEnvVar = boost::python::extract<std::string>( classInfo[3] );
		
		MString storedClassName;
		int storedClassVersion;
		MString storedSearchPathEnvVar;
		currentClass( plug, storedClassName, storedClassVersion, storedSearchPathEnvVar );
		
		// only set the plug values if the new value is genuinely different, as otherwise
		// we end up generating unwanted reference edits.
		if ( storedClassName != className.c_str() || storedClassVersion != classVersion || storedSearchPathEnvVar != searchPathEnvVar.c_str() )
		{
			MStringArray updatedClassInfo;
			updatedClassInfo.append( className.c_str() );
			MString classVersionStr;
			classVersionStr.set( classVersion, 0 );
			updatedClassInfo.append( classVersionStr );
			updatedClassInfo.append( searchPathEnvVar.c_str() );
			
			MObject attribute = plug.attribute();
			MFnTypedAttribute fnTAttr( attribute );
			if ( fnTAttr.attrType() == MFnData::kStringArray )
			{
				MObject data = MFnStringArrayData().create( updatedClassInfo );
				plug.setValue( data );
			}
			else
			{
				// compatibility for the deprecated compound plug behaviour. keeping this code
				// so we can still read old scenes. creation of these plugs has been removed.
				/// \todo: find all such notes and remove the unnecessary code for Cortex 9.
				plug.child( 0 ).setString( className.c_str() );
				plug.child( 1 ).setInt( classVersion );
				plug.child( 2 ).setString( searchPathEnvVar.c_str() );
			}
		}
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
		return MS::kFailure;
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassParameterHandler::setClass : " ) + e.what() );
		return MS::kFailure;
	}
	
	return MS::kSuccess;
}
