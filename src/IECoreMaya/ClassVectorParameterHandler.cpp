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
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MFnIntArrayData.h"
#include "maya/MGlobal.h"

#include "IECore/CompoundObject.h"
#include "IECore/SimpleTypedData.h"

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
		boost::python::object pythonParameter( boost::const_pointer_cast<IECore::Parameter>( parameter ) );
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
	MObject attribute = plug.attribute();
	MFnTypedAttribute fnTAttr( attribute );
	if ( !fnTAttr.hasObj( attribute ) || fnTAttr.attrType() != MFnData::kStringArray )
	{
		// compatibility for the deprecated compound plug behaviour
		MObject parameterNamesObject = plug.child( 0 ).asMObject();
		MFnStringArrayData fnSAD( parameterNamesObject );
		fnSAD.copyTo( parameterNames );

		MObject classNamesObject = plug.child( 1 ).asMObject();
		fnSAD.setObject( classNamesObject );
		fnSAD.copyTo( classNames );

		MObject classVersionsObject = plug.child( 2 ).asMObject();
		MFnIntArrayData fnIAD( classVersionsObject );
		fnIAD.copyTo( classVersions );
		return;
	}

	parameterNames.clear();
	classNames.clear();
	classVersions.clear();

	MFnStringArrayData fnSAD( plug.asMObject() );
	if ( fnSAD.length() == 0 )
	{
		return;
	}

	if ( fnSAD.length() % 3 != 0 )
	{
		throw( IECore::InvalidArgumentException( ( plug.name() + " needs 3 values per class. Expected a series of name, className, version." ).asChar() ) );
	}

	MStringArray storedClassInfo = fnSAD.array();
	for ( unsigned i=0; i < storedClassInfo.length(); i+=3 )
	{
		if ( !storedClassInfo[i+2].isInt() )
		{
			throw( IECore::InvalidArgumentException( ( "Version values of " + plug.name() + " must represent an integer" ).asChar() ) );
		}

		parameterNames.append( storedClassInfo[i] );
		classNames.append( storedClassInfo[i+1] );
		classVersions.append( storedClassInfo[i+2].asInt() );
	}
}

MStatus ClassVectorParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnTypedAttribute fnTAttr( attribute );
	// compatibility for the deprecated compound plug behaviour: should return MS::kFailure
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
	}

	if( !storeClasses( parameter, plug ) )
	{
		return MS::kFailure;
	}

	return finishUpdating( parameter, plug );
}

MStatus ClassVectorParameterHandler::doRestore( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	MStringArray parameterNames;
	MStringArray classNames;
	MIntArray classVersions;

	try
	{
		currentClasses( plug, parameterNames, classNames, classVersions );
	}
	catch( const std::exception &e )
	{
		MGlobal::displayError( MString( "ClassVectorParameterHandler::doRestore : " ) + e.what() );
		return MS::kFailure;
	}

	return setClasses( parameter, parameterNames, classNames, classVersions );
}

MPlug ClassVectorParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	if( !parameter || !parameter->isInstanceOf( IECore::ClassVectorParameterTypeId ) )
	{
		return MPlug();
	}

	MFnTypedAttribute fnTAttr;
	MObject attribute = fnTAttr.create( plugName, plugName, MFnData::kStringArray );
	MPlug result = finishCreating( parameter, attribute, node );

	if( !storeClasses( parameter, result ) )
	{
		return MPlug(); // failure
	}

	if( !finishUpdating( parameter, result ) )
	{
		return MPlug(); // failure
	}

	return result;
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
		boost::python::object pythonParameter( boost::const_pointer_cast<IECore::Parameter>( parameter ) );
		boost::python::object classes = pythonParameter.attr( "getClasses" )( true );

		MStringArray storedParameterNames;
		MStringArray storedClassNames;
		MIntArray storedClassVersions;
		currentClasses( plug, storedParameterNames, storedClassNames, storedClassVersions );
		unsigned storedParameterNamesLength = storedParameterNames.length();
		unsigned storedClassNamesLength = storedClassNames.length();
		unsigned storedClassVersionsLength = storedClassVersions.length();

		MStringArray updatedClassInfo;
		// compatibility for the deprecated compound plug behaviour
		MStringArray parameterNames;
		MStringArray classNames;
		MIntArray classVersions;

		size_t l = IECorePython::len( classes );
		bool changed = l != storedParameterNamesLength || l != storedClassNamesLength || l != storedClassVersionsLength;

		for( size_t i=0; i<l; i++ )
		{
			object cl = classes[i];

			MString parameterName = boost::python::extract<const char *>( cl[1] )();
			updatedClassInfo.append( parameterName );
			parameterNames.append( parameterName );
			if( i < storedParameterNamesLength && parameterName != storedParameterNames[i] )
			{
				changed = true;
			}

			MString className = boost::python::extract<const char *>( cl[2] )();
			updatedClassInfo.append( className );
			classNames.append( className );
			if( i < storedClassNamesLength && className != storedClassNames[i] )
			{
				changed = true;
			}

			int classVersion = boost::python::extract<int>( cl[3] );
			MString classVersionStr;
			classVersionStr.set( classVersion, 0 );
			updatedClassInfo.append( classVersionStr );
			classVersions.append( classVersion );
			if( i < storedClassVersionsLength && classVersion != storedClassVersions[i] )
			{
				changed = true;
			}
		}

		// only set the plug values if the new value is genuinely different, as otherwise
		// we end up generating unwanted reference edits.
		if ( changed )
		{
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
				MObject parameterNamesObject = MFnStringArrayData().create( parameterNames );
				plug.child( 0 ).setValue( parameterNamesObject );
				MObject classNamesObject = MFnStringArrayData().create( classNames );
				plug.child( 1 ).setValue( classNamesObject );
				MObject classVersionsObject = MFnIntArrayData().create( classVersions );
				plug.child( 2 ).setValue( classVersionsObject );
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
		MGlobal::displayError( MString( "ClassVectorParameterHandler::setClass : " ) + e.what() );
		return MS::kFailure;
	}

	return MS::kSuccess;
}
