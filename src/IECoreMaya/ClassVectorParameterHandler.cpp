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

#include "boost/python.hpp"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MFnIntArrayData.h"
#include "maya/MGlobal.h"

#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/IECoreBinding.h"

#include "IECoreMaya/ClassVectorParameterHandler.h"

using namespace IECoreMaya;
using namespace boost::python;

static ParameterHandler::Description<ClassVectorParameterHandler> registrar( IECore::ClassVectorParameterTypeId );


MStatus ClassVectorParameterHandler::setClasses( IECore::ParameterPtr parameter, const MStringArray &parameterNames, const MStringArray &classNames, const MIntArray &classVersions )
{
	assert( parameterNames.length() == classNames.length() );
	assert( parameterNames.length() == classVersions.length() );

	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( parameter );
		
		boost::python::list classes;
		for( unsigned i=0; i<parameterNames.length(); i++ )
		{
			classes.append( make_tuple( parameterNames[i].asChar(), classNames[i].asChar(), classVersions[i] ) );
		}
				
		pythonParameter.attr( "setClasses" )( classes );	
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
		return MS::kFailure;
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassVectorParameterHandler::setClasses : " ) + e.what() );
		return MS::kFailure;
	}
	return MS::kSuccess;
}

MStatus ClassVectorParameterHandler::getClasses( IECore::ConstParameterPtr parameter, MStringArray &parameterNames, MStringArray &classNames, MIntArray &classVersions )
{
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( IECore::constPointerCast<IECore::Parameter>( parameter ) );
		boost::python::list classesInfo = extract<list>( pythonParameter.attr( "getClasses" )( true ) );
		
		int l = boost::python::len( classesInfo );
		for( int i=0; i<l; i++ )
		{
			tuple c = extract<tuple>( classesInfo[i] );
			parameterNames.append( MString( extract<const char *>( c[1] ) ) );
			classNames.append( MString( extract<const char *>( c[2] ) ) );
			classVersions.append( extract<int>( c[3] ) );
		}
				
		return MS::kSuccess;
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassVectorParameterHandler::getClasses : " ) + e.what() );
	}
	return MS::kFailure;
}

void ClassVectorParameterHandler::currentClasses( const MPlug &plug, MStringArray &parameterNames, MStringArray &classNames, MIntArray &classVersions )
{
	MObject parameterNamesObject = plug.child( 0 ).asMObject();
	MFnStringArrayData fnSAD( parameterNamesObject );
	fnSAD.copyTo( parameterNames );
	
	MObject classNamesObject = plug.child( 1 ).asMObject();
	fnSAD.setObject( classNamesObject );
	fnSAD.copyTo( classNames );
	
	MObject classVersionsObject = plug.child( 2 ).asMObject();
	MFnIntArrayData fnIAD( classVersionsObject );
	fnIAD.copyTo( classVersions );
}
				
MStatus ClassVectorParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{	
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	if( fnCAttr.numChildren()!=3 )
	{
		return MS::kFailure;
	}

	MObject parameterNamesAttr = fnCAttr.child( 0 );
	MFnTypedAttribute fnTAttr( parameterNamesAttr );
	if( !fnTAttr.hasObj( parameterNamesAttr ) )
	{
		return MS::kFailure;	
	}
	if( fnTAttr.name() != fnCAttr.name() + "__parameterNames" )
	{
		return MS::kFailure;
	}
	if( fnTAttr.attrType()!=MFnData::kStringArray )
	{
		return MS::kFailure;
	}
	
	MObject classNamesAttr = fnCAttr.child( 1 );
	fnTAttr.setObject( classNamesAttr );
	if( !fnTAttr.hasObj( classNamesAttr ) )
	{
		return MS::kFailure;	
	}
	if( fnTAttr.name() != fnCAttr.name() + "__classNames" )
	{
		return MS::kFailure;
	}
	if( fnTAttr.attrType()!=MFnData::kStringArray )
	{
		return MS::kFailure;
	}
	
	MObject classVersionsAttr = fnCAttr.child( 2 );
	fnTAttr.setObject( classVersionsAttr );
	if( !fnTAttr.hasObj( classVersionsAttr ) )
	{
		return MS::kFailure;	
	}
	if( fnTAttr.name() != fnCAttr.name() + "__classVersions" )
	{
		return MS::kFailure;
	}
	if( fnTAttr.attrType()!=MFnData::kIntArray )
	{
		return MS::kFailure;
	}

	MObject parameterNamesObject = plug.child( 0 ).asMObject();
	MFnStringArrayData fnSAD( parameterNamesObject );
	MStringArray parameterNames = fnSAD.array();
	
	MObject classNamesObject = plug.child( 1 ).asMObject();
	fnSAD.setObject( classNamesObject );
	MStringArray classNames = fnSAD.array();
	
	MObject classVersionsObject = plug.child( 2 ).asMObject();
	MFnIntArrayData fnIAD( classVersionsObject );
	MIntArray classVersions = fnIAD.array();

	return storeClasses( parameter, plug );
}

MStatus ClassVectorParameterHandler::doRestore( const MPlug &plug, IECore::ParameterPtr parameter )
{
	MStringArray parameterNames;
	MStringArray classNames;
	MIntArray classVersions;
	currentClasses( plug, parameterNames, classNames, classVersions );
	return setClasses( parameter, parameterNames, classNames, classVersions );
}

MPlug ClassVectorParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{	
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MPlug();
	}

	MFnCompoundAttribute fnCAttr;
	MObject attribute = fnCAttr.create( plugName, plugName );

	MFnTypedAttribute fnTAttr;
	MObject parameterNamesAttr = fnTAttr.create( plugName + "__parameterNames", plugName + "__parameterNames", MFnData::kStringArray );
	fnCAttr.addChild( parameterNamesAttr );
	
	MObject classNamesAttr = fnTAttr.create( plugName + "__classNames", plugName + "__classNames", MFnData::kStringArray );
	fnCAttr.addChild( classNamesAttr );
	
	MObject classVersionsAttr = fnTAttr.create( plugName + "__classVersions", plugName + "__classVersions", MFnData::kIntArray );
	fnCAttr.addChild( classVersionsAttr );
	
	MPlug result = finishCreating( parameter, attribute, node );
	
	if( storeClasses( parameter, result ) )
	{
		return result;
	}
	
	return MPlug(); // failure
}

MStatus ClassVectorParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MS::kFailure;
	}
	return MS::kSuccess;
}

MStatus ClassVectorParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MS::kFailure;
	}
	
	return MS::kSuccess;
}

MStatus ClassVectorParameterHandler::storeClasses( IECore::ConstParameterPtr parameter, MPlug &plug )
{
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( IECore::constPointerCast<IECore::Parameter>( parameter ) );
		boost::python::object classes = pythonParameter.attr( "getClasses" )( true );
	
		MStringArray parameterNames;
		MStringArray classNames;
		MIntArray classVersions;
	
		size_t l = IECorePython::len( classes );
		for( size_t i=0; i<l; i++ )
		{
			object cl = classes[i];
			MString parameterName = boost::python::extract<const char *>( cl[1] )();
			parameterNames.append( parameterName );
			MString className = boost::python::extract<const char *>( cl[2] )();
			classNames.append( className );
			classVersions.append( boost::python::extract<int>( cl[3] ) );
		}
	
		MFnStringArrayData fnSAD;
		MFnIntArrayData fnIAD;
	
		MObject parameterNamesObject = fnSAD.create( parameterNames );
		plug.child( 0 ).setValue( parameterNamesObject );
		
		MObject classNamesObject = fnSAD.create( classNames );
		plug.child( 1 ).setValue( classNamesObject );
		
		MObject classVersionsObject = fnIAD.create( classVersions );
		plug.child( 2 ).setValue( classVersionsObject );
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
		return MS::kFailure;
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassVectorParameterHandler::setClass : " ) + e.what() );
		return MS::kFailure;
	}
	
	return MS::kSuccess;
}
