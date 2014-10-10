//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/bind.hpp"
#include "boost/python.hpp"
#include "boost/format.hpp"

#include "CH/CH_LocalVariable.h"
#include "OBJ/OBJ_Node.h"
#include "PRM/PRM_Include.h"
#include "PRM/PRM_Parm.h"
#include "SOP/SOP_Node.h"
#include "ROP/ROP_Node.h"

#include "IECore/LevelFilteredMessageHandler.h"
#include "IECore/ParameterisedInterface.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/OStreamMessageHandler.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreHoudini/CoreHoudini.h"
#include "IECoreHoudini/ParameterisedHolder.h"

using namespace boost;
using namespace boost::python;

using namespace IECore;
using namespace IECoreHoudini;

template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pParameterisedClassCategory( "__classCategory", "Category:" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pParameterisedClassName( "__className", "Class:" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pParameterisedVersion( "__classVersion", "  Version:" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pParameterisedSearchPathEnvVar( "__classSearchPathEnvVar", "SearchPathEnvVar:" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pMatchString( "__classMatchString", "MatchString" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pReloadButton( "__classReloadButton", "Reload" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pEvaluateParameters( "__evaluateParameters", "ParameterEval" );
template<typename BaseType>
PRM_Name ParameterisedHolder<BaseType>::pSwitcher( "__parameterSwitcher", "Switcher" );

template<typename BaseType>
PRM_Default ParameterisedHolder<BaseType>::matchStringDefault( 0, "*" );
template<typename BaseType>
PRM_Default ParameterisedHolder<BaseType>::switcherDefaults[] = { PRM_Default( 0, "Parameters" ) };

template<typename BaseType>
PRM_ChoiceList ParameterisedHolder<BaseType>::classCategoryMenu( PRM_CHOICELIST_SINGLE, &ParameterisedHolder<BaseType>::buildClassCategoryMenu );
template<typename BaseType>
PRM_ChoiceList ParameterisedHolder<BaseType>::classNameMenu( PRM_CHOICELIST_SINGLE, &ParameterisedHolder<BaseType>::buildClassNameMenu );
template<typename BaseType>
PRM_ChoiceList ParameterisedHolder<BaseType>::classVersionMenu( PRM_CHOICELIST_SINGLE, &ParameterisedHolder<BaseType>::buildVersionMenu );

template<typename BaseType>
PRM_Template ParameterisedHolder<BaseType>::parameters[] = {
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT, 1, &pParameterisedClassCategory, 0, &classCategoryMenu, 0, &ParameterisedHolder<BaseType>::reloadClassCallback ),
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT, 1, &pParameterisedClassName, 0, &classNameMenu, 0, &ParameterisedHolder<BaseType>::reloadClassCallback ),
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT, 1, &pParameterisedVersion, 0, &classVersionMenu, 0, &ParameterisedHolder<BaseType>::reloadClassCallback ),
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT|PRM_TYPE_INVISIBLE, 1, &pParameterisedSearchPathEnvVar, 0, 0, 0, &ParameterisedHolder<BaseType>::reloadClassCallback ),
	PRM_Template( PRM_STRING|PRM_TYPE_INVISIBLE, 1, &pMatchString, &matchStringDefault ),
	PRM_Template( PRM_CALLBACK, 1, &pReloadButton, 0, 0, 0, &ParameterisedHolder<BaseType>::reloadButtonCallback ),
	PRM_Template( PRM_INT|PRM_TYPE_INVISIBLE, 1, &pEvaluateParameters ),
	PRM_Template( PRM_SWITCHER, 1, &pSwitcher, switcherDefaults ),
	PRM_Template()
};

template<typename BaseType>
CH_LocalVariable ParameterisedHolder<BaseType>::variables[] = {
	{ 0, 0, 0 },
};

template<typename BaseType>
ParameterisedHolder<BaseType>::ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op ) : BaseType( net, name, op )
{
	CoreHoudini::initPython();
	
	this->getParm( "__evaluateParameters" ).setExpression( 0, "val = 0\nreturn val", CH_PYTHON, 0 );
	this->getParm( "__evaluateParameters" ).setLockedFlag( 0, 1 );
}

template<typename BaseType>
ParameterisedHolder<BaseType>::~ParameterisedHolder()
{
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::buildClassCategoryMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	ParameterisedHolder<BaseType> *holder = reinterpret_cast<ParameterisedHolder<BaseType>*>( data );
	if ( !holder )
	{
		return;
	}
	
	menu[0].setToken( "" );
	menu[0].setLabel( "< No Category Selected >" );
	unsigned int pos = 1;

	UT_String value;
	holder->evalString( value, pMatchString.getToken(), 0, 0 );
	std::string matchString( value.toStdString() );
	size_t padding = matchString.length();
	
	holder->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	std::string searchPathEnvVar( value.toStdString() );
	
	std::vector<std::string> names;
	classNames( searchPathEnvVar, matchString, names );
	
	std::set<std::string> categories;
	for ( std::vector<std::string>::const_iterator it=names.begin(); it != names.end(); it++ )
	{
		size_t divider = (*it).rfind( "/" );
		if ( divider != std::string::npos )
		{
			categories.insert( (*it).substr( 0, divider ) );
		}
	}
	
	for ( std::set<std::string>::const_iterator it=categories.begin(); it != categories.end(); it++, pos++ )
	{
		menu[pos].setToken( (*it).c_str() );
		menu[pos].setLabel( (*it).substr( (*it).find( matchString ) + padding ).c_str() );
	}

	// mark the end of our menu
	menu[pos].setToken( 0 );
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::buildClassNameMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	ParameterisedHolder<BaseType> *holder = reinterpret_cast<ParameterisedHolder<BaseType>*>( data );
	if ( !holder )
	{
		return;
	}
	
	menu[0].setToken( "" );
	menu[0].setLabel( "< No Class Loaded >" );
	unsigned int pos = 1;

	UT_String value;
	holder->evalString( value, pMatchString.getToken(), 0, 0 );
	std::string matchString( value.toStdString() );
	
	holder->evalString( value, pParameterisedClassCategory.getToken(), 0, 0 );
	std::string category( value.toStdString() );
	if ( category != "" )
	{
		matchString = category + "/*";
	}
	
	size_t padding = matchString.length();
	
	holder->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	std::string searchPathEnvVar( value.toStdString() );
	
	std::vector<std::string> names;
	classNames( searchPathEnvVar, matchString, names );

	for ( std::vector<std::string>::const_iterator it=names.begin(); it != names.end(); it++, pos++ )
	{
		menu[pos].setToken( (*it).c_str() );
		menu[pos].setLabel( (*it).substr( (*it).find( matchString ) + padding ).c_str() );
	}

	// mark the end of our menu
	menu[pos].setToken( 0 );
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::buildVersionMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	ParameterisedHolder<BaseType> *holder = reinterpret_cast<ParameterisedHolder<BaseType>*>( data );
	if ( !holder )
	{
		return;
	}

	unsigned int pos = 0;
	
	UT_String value;
	holder->evalString( value, pParameterisedClassName.getToken(), 0, 0 );
	std::string className( value.toStdString() );
	
	if ( className != "" )
	{
		holder->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
		std::string searchPathEnvVar( value.toStdString() );
	
		std::vector<int> versions;
		classVersions( className, searchPathEnvVar, versions );
		for ( std::vector<int>::iterator it=versions.begin(); it != versions.end(); it++, pos++ )
		{
			std::stringstream ss;
			ss << (*it);
			menu[pos].setToken( ss.str().c_str() );
			menu[pos].setLabel( ss.str().c_str() );
		}
	}

	if ( !pos )
	{
		menu[0].setToken( "" );
		menu[0].setLabel( "< No Version >" );
		pos++;
	}

	// mark the end of our menu
	menu[pos].setToken( 0 );
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::reloadClassCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	ParameterisedHolder<BaseType> *holder = reinterpret_cast<ParameterisedHolder<BaseType>*>( data );
	if ( !holder )
	{
		return 0;
	}

	UT_String value;
	holder->evalString( value, pParameterisedClassCategory.getToken(), 0, 0 );
	std::string category( value.toStdString() );
	
	holder->evalString( value, pParameterisedClassName.getToken(), 0, 0 );
	std::string className( value.toStdString() );
	
	holder->evalString( value, pParameterisedVersion.getToken(), 0, 0 );
	int version = -1;
	if ( value != "" )
	{
		version = boost::lexical_cast<int>( value.buffer() );
	}
	
	holder->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	std::string searchPathEnvVar( value.toStdString() );
	
	size_t divider = holder->m_loadedClassName.rfind( "/" );
	if ( holder->m_loadedClassName != ""  && divider != std::string::npos && category != holder->m_loadedClassName.substr( 0, divider ) )
	{
		className = "";
	}
	
	if ( className != holder->m_loadedClassName )
	{
		version = -1;
	}
	
	if ( className == "" )
	{
		version = -1;
		holder->setParameterised( 0 );
	}
	else if ( version == -1 )
	{
		version = defaultClassVersion( className, searchPathEnvVar );
		holder->setString( boost::lexical_cast<std::string>( version ).c_str(), CH_STRING_LITERAL, pParameterisedVersion.getToken(), 0, 0 );
	}

	holder->load( className, version, searchPathEnvVar );

	return 1;
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	ParameterisedHolder<BaseType> *holder = reinterpret_cast<ParameterisedHolder<BaseType>*>( data );
	if ( !holder )
	{
		return 0;
	}

	UT_String value;
	holder->evalString( value, pParameterisedClassName.getToken(), 0, 0 );
	std::string className( value.toStdString() );
	
	holder->evalString( value, pParameterisedVersion.getToken(), 0, 0 );
	int version = -1;
	if ( value != "" )
	{
		version = boost::lexical_cast<int>( value.buffer() );
	}
	
	holder->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	std::string searchPathEnvVar( value.toStdString() );

	CoreHoudini::evalPython( "IECore.ClassLoader.defaultLoader( \"" + searchPathEnvVar + "\" ).refresh()" );
	holder->load( className, version, searchPathEnvVar );

	return 1;
}

template<typename BaseType>
const char *ParameterisedHolder<BaseType>::inputLabel( unsigned pos ) const
{
	if ( (int)pos > (int)m_inputParameters.size() - 1 )
	{
		return "";
	}
	
	const Parameter *parm = m_inputParameters[pos].get();
	return ( parm->name() + ": " + parm->description() ).c_str();
}

template<typename BaseType>
unsigned ParameterisedHolder<BaseType>::minInputs() const
{
	/// \todo: need to check for 'required' inputs and increase this number accordingly
	return 0;
}

template<typename BaseType>
unsigned ParameterisedHolder<BaseType>::maxInputs() const
{
	// this makes sure when we first load we have 4 inputs
	// the wires get connected before the Op is loaded onto the node
	if ( !m_parameterised || m_inputParameters.size() > 4 )
	{
		return 4;
	}
	
	return m_inputParameters.size();
}

template<typename BaseType>
bool ParameterisedHolder<BaseType>::setNodeValues()
{
	RunTimeTypedPtr parameterised = getParameterised();
	if ( !parameterised )
	{
		return false;
	}
	
	IECore::MessageHandler::Scope handlerScope( getMessageHandler() );
	
	UT_String path;
	this->getFullPath( path );
	std::string cmd = "IECoreHoudini.FnParameterisedHolder( hou.node( \"";
	cmd += path.buffer();
	cmd += "\") )";
	{
		IECorePython::ScopedGILLock lock;
		try
		{
			handle<> resultHandle( PyRun_String( cmd.c_str(), Py_eval_input, CoreHoudini::globalContext().ptr(), CoreHoudini::globalContext().ptr() ) );
			object fn( resultHandle );
			fn.attr( "updateParameters" )( parameterised );
		}
		catch( ... )
		{
			PyErr_Print();
		}
	}
	
	return true;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setParameterisedValues( double time )
{
	IECore::ParameterisedInterface *parameterised = getParameterisedInterface();
	if ( !parameterised )
	{
		return;
	}
	
	// push the input values into the associated parameters
	setInputParameterValues( time );
	
	// update the remaining parameters to match the node values
	updateParameter( parameterised->parameters(), time, "", true );
}

template<typename BaseType>
bool ParameterisedHolder<BaseType>::hasParameterised()
{
	return (bool)(m_parameterised.get() != 0);
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setParameterised( IECore::RunTimeTypedPtr p )
{
	this->setString( "", CH_STRING_LITERAL, pParameterisedClassName.getToken(), 0, 0 );
	this->setString( boost::lexical_cast<std::string>( -1 ).c_str(), CH_STRING_LITERAL, pParameterisedVersion.getToken(), 0, 0 );

	m_parameterised = p;
	m_loadedClassName = "";
	
	refreshInputConnections();
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar )
{
	size_t divider = className.rfind( "/" );
	std::string category = ( divider == std::string::npos ) ? "" : className.substr( 0, divider );
	this->setString( category.c_str(), CH_STRING_LITERAL, pParameterisedClassCategory.getToken(), 0, 0 );
	this->setString( className.c_str(), CH_STRING_LITERAL, pParameterisedClassName.getToken(), 0, 0 );
	this->setString( boost::lexical_cast<std::string>( classVersion ).c_str(), CH_STRING_LITERAL, pParameterisedVersion.getToken(), 0, 0 );
	this->setString( searchPathEnvVar.c_str(), CH_STRING_LITERAL, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	
	m_parameterised = loadParameterised( className, classVersion, searchPathEnvVar );
	m_loadedClassName = m_parameterised ? className : "";
	
	refreshInputConnections();
}

template<typename BaseType>
IECore::RunTimeTypedPtr ParameterisedHolder<BaseType>::getParameterised()
{
	return m_parameterised;
}

template<typename BaseType>
IECore::RunTimeTypedPtr ParameterisedHolder<BaseType>::loadParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar )
{
	IECorePython::ScopedGILLock gilLock;

	std::string pythonCmd = boost::str( format( "IECore.ClassLoader.defaultLoader( \"%s\" ).load( \"%s\", %d )()\n" ) % searchPathEnvVar % className % classVersion );

	try
	{
		boost::python::handle<> resultHandle( PyRun_String( pythonCmd.c_str(), Py_eval_input, CoreHoudini::globalContext().ptr(), CoreHoudini::globalContext().ptr() )	);
		boost::python::object result( resultHandle );
		return boost::python::extract<IECore::RunTimeTypedPtr>( result )();
	}
	catch( ... )
	{
		PyErr_Print();
	}

	return 0;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::load( const std::string &className, int classVersion, const std::string &searchPathEnvVar, bool updateGui )
{
	RunTimeTypedPtr parameterised = 0;
	if ( className != "" && classVersion != -1 && searchPathEnvVar != "" )
	{
		setParameterised( className, classVersion, searchPathEnvVar );
		parameterised = getParameterised();
		m_loadedClassName = className;
	}

	if ( !parameterised )
	{
		m_loadedClassName = "";
		m_inputParameters.clear();
		this->addError( SOP_MESSAGE, "ParameterisedHolder has no parameterised class to operate on!" );
	}

	m_dirty = true;
	
	if ( !updateGui )
	{
		return;
	}
	
	setNodeValues();
}

template<typename BaseType>
bool ParameterisedHolder<BaseType>::load( UT_IStream &is, const char *ext, const char *path )
{
	bool loaded = OP_Node::load( is, ext, path );

	UT_String value;
	this->evalString( value, pParameterisedClassName.getToken(), 0, 0 );
	std::string className( value.toStdString() );
	
	this->evalString( value, pParameterisedVersion.getToken(), 0, 0 );
	int version = -1;
	if ( value != "" )
	{
		version = boost::lexical_cast<int>( value.buffer() );
	}
	
	this->evalString( value, pParameterisedSearchPathEnvVar.getToken(), 0, 0 );
	std::string searchPathEnvVar( value.toStdString() );
	
	if ( className != "" && version != -1 && searchPathEnvVar != "" )
	{
		load( className, version, searchPathEnvVar, false );
	}
	
	/// \todo: not entirely certain this is returning the correct thing
	return loaded;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::updateParameter( ParameterPtr parm, float now, std::string prefix, bool top_level )
{
	try
	{
		// find out our parameter name
		std::string parm_name = prefix + std::string( parm->name() );

		// compoundParameters - recursively calling updateParameter on children
		if ( parm->isInstanceOf( IECore::CompoundParameterTypeId ) )
		{
			if ( top_level==true )
			{
				// only our top-level compound parameter should apply the generic prefix
				parm_name = "parm_";
			}
			else
			{
				parm_name += "_";
			}

			IECore::CompoundParameterPtr compound = IECore::runTimeCast<CompoundParameter>(parm);
			if ( parm )
			{
				const CompoundParameter::ParameterMap &childParms = compound->parameters();
				for( CompoundParameter::ParameterMap::const_iterator it=childParms.begin(); it != childParms.end(); it++ )
				{
					updateParameter( it->second, now, parm_name );
				}
			}
			return;
		}

		// check we can find the parameter on our Houdini node
		if ( !this->getParmList()->getParmPtr( parm_name.c_str() ) )
		{
			return;
		}

		// does this parameter cause a gui refresh?
		bool do_update = true;
		if( CompoundObjectPtr uiData = parm->userData()->member<CompoundObject>( "UI" ) )
		{
			if( BoolDataPtr update_data = uiData->member<BoolData>( "update" ) )
			{
				do_update = update_data->readable();
			}
		}

		// handle the different parameter types
		switch( parm->typeId() )
		{
			// int parameter
			case IECore::IntParameterTypeId:
			{
				int val = static_cast<const IntData *>( parm->defaultValue() )->readable();
				
				// horrible hack to accomodate Houdini's MenuParmTemplate for IntParameters
				// We really need ParameterHandlers in c++ and Houdini really needs to
				// support proper menus on any ParmTemplate.
				if ( parm->presetsOnly() )
				{
					UT_String hStr;
					this->evalString( hStr, parm_name.c_str(), 0, now );
					if ( !hStr.isInteger() )
					{
						throw IECore::InvalidArgumentException( "Attempt to set IntParameter " + parm->name() + " to a non-int value " + hStr.toStdString() );
					}
					
					val = hStr.toInt();
				}
				else
				{
					val = this->evalInt( parm_name.c_str(), 0, now );
				}
				
				checkForUpdate<int, IntData>( do_update, val, parm );
				parm->setValue( new IECore::IntData(val) );
				break;
			}

			// V2f parameter
			case IECore::V2iParameterTypeId:
			{
				int vals[2];
				for ( unsigned int i=0; i<2; i++ )
					vals[i] = this->evalInt( parm_name.c_str(), i, now );
				Imath::V2i val(vals[0], vals[1]);
				checkForUpdate<Imath::V2i, V2iData>( do_update, val, parm );
				parm->setValue( new IECore::V2iData(val) );
				break;
			}

			// V3i parameter
			case IECore::V3iParameterTypeId:
			{
				int vals[3];
				for ( unsigned int i=0; i<3; i++ )
					vals[i] = this->evalInt( parm_name.c_str(), i, now );
				Imath::V3i val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3i, V3iData>( do_update, val, parm );
				parm->setValue( new IECore::V3iData(val) );
				break;
			}

			// float parameter
			case IECore::FloatParameterTypeId:
			{
				float val = this->evalFloat( parm_name.c_str(), 0, now );
				checkForUpdate<float, FloatData>( do_update, val, parm );
				parm->setValue( new IECore::FloatData(val) );
				break;
			}

			// V2f parameter
			case IECore::V2fParameterTypeId:
			{
				float vals[2];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::V2f val(vals[0], vals[1]);
				checkForUpdate<Imath::V2f, V2fData>( do_update, val, parm );
				parm->setValue( new IECore::V2fData(val) );
				break;
			}

			// V3f parameter
			case IECore::V3fParameterTypeId:
			{
				float vals[3];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::V3f val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3f, V3fData>( do_update, val, parm );
				parm->setValue( new IECore::V3fData(val) );
				break;
			}

			// double parameter
			case IECore::DoubleParameterTypeId:
			{
				float val = this->evalFloat( parm_name.c_str(), 0, now );
				checkForUpdate<float, DoubleData>( do_update, val, parm );
				parm->setValue( new IECore::DoubleData(val) );
				break;
			}

			// V2d parameter
			case IECore::V2dParameterTypeId:
			{
				float vals[2];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::V2d val(vals[0], vals[1]);
				checkForUpdate<Imath::V2d, V2dData>( do_update, val, parm );
				parm->setValue( new IECore::V2dData(val) );
				break;
			}

			// V3d parameter
			case IECore::V3dParameterTypeId:
			{
				float vals[3];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::V3d val(vals[0], vals[1], vals[2]);
				checkForUpdate<Imath::V3d, V3dData>( do_update, val, parm );
				parm->setValue( new IECore::V3dData(val) );
				break;
			}

			// bool parameter
			case IECore::BoolParameterTypeId:
			{
				bool val = (bool)this->evalInt( parm_name.c_str(), 0, now );
				checkForUpdate<bool, BoolData>( do_update, val, parm );
				parm->setValue( new IECore::BoolData(val) );
				break;
			}

			// string parameter
			case IECore::StringParameterTypeId:
			case IECore::ValidatedStringParameterTypeId:
			case IECore::PathParameterTypeId:
			case IECore::DirNameParameterTypeId:
			case IECore::FileNameParameterTypeId:
			case IECore::FileSequenceParameterTypeId:
			{
				UT_String h_str;
				this->evalString( h_str, parm_name.c_str(), 0, now );
				std::string val( h_str.buffer() );
				checkForUpdate<std::string, StringData>( do_update, val, parm );
				parm->setValue( new IECore::StringData(val) );
				break;
			}

			// colour 3f parameter
			case IECore::Color3fParameterTypeId:
			{
				float vals[3];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Color3f val( vals[0], vals[1], vals[2] );
				checkForUpdate<Imath::Color3f, Color3fData>( do_update, val, parm );
				parm->setValue( new IECore::Color3fData( val ) );
				break;
			}

			// colour 4f parameter
			case IECore::Color4fParameterTypeId:
			{
				float vals[4];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Color4f val( vals[0], vals[1], vals[2], vals[3] );
				checkForUpdate<Imath::Color4f, Color4fData>( do_update, val, parm );
				parm->setValue( new IECore::Color4fData( val ) );
				break;
			}

			// M44f
			case IECore::M44fParameterTypeId:
			{
				float vals[16];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::M44f val(vals[0], vals[1], vals[2], vals[3],
								vals[4], vals[5], vals[6], vals[7],
								vals[8], vals[9], vals[10], vals[11],
								vals[12], vals[13], vals[14], vals[15]);
				checkForUpdate<Imath::M44f, M44fData>( do_update, val, parm );
				parm->setValue( new IECore::M44fData(val) );
				break;
			}

			// M44d
			case IECore::M44dParameterTypeId:
			{
				float vals[16];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::M44d val(vals[0], vals[1], vals[2], vals[3],
								vals[4], vals[5], vals[6], vals[7],
								vals[8], vals[9], vals[10], vals[11],
								vals[12], vals[13], vals[14], vals[15]);
				checkForUpdate<Imath::M44d, M44dData>( do_update, val, parm );
				parm->setValue( new IECore::M44dData(val) );
				break;
			}

			// Box2i
			case IECore::Box2iParameterTypeId:
			{
				int vals[4];
				for ( unsigned int i=0; i<4; i++ )
					vals[i] = this->evalInt( parm_name.c_str(), i, now );
				Imath::Box2i val( Imath::V2i( vals[0], vals[1] ),
									Imath::V2i( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2i, Box2iData>( do_update, val, parm );
				parm->setValue( new IECore::Box2iData(val) );
				break;
			}

			// Box2f
			case IECore::Box2fParameterTypeId:
			{
				float vals[4];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Box2f val( Imath::V2f( vals[0], vals[1] ),
									Imath::V2f( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2f, Box2fData>( do_update, val, parm );
				parm->setValue( new IECore::Box2fData(val) );
				break;
			}

			// Box2d
			case IECore::Box2dParameterTypeId:
			{
				float vals[4];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Box2d val( Imath::V2d( vals[0], vals[1] ),
									Imath::V2d( vals[2], vals[3] ) );
				checkForUpdate<Imath::Box2d, Box2dData>( do_update, val, parm );
				parm->setValue( new IECore::Box2dData(val) );
				break;
			}

			// Box3i
			case IECore::Box3iParameterTypeId:
			{
				int vals[6];
				for ( unsigned int i=0; i<6; i++ )
					vals[i] = this->evalInt( parm_name.c_str(), i, now );
				Imath::Box3i val( Imath::V3i( vals[0], vals[1], vals[2] ),
									Imath::V3i( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3i, Box3iData>( do_update, val, parm );
				parm->setValue( new IECore::Box3iData(val) );
				break;
			}

			// Box3f
			case IECore::Box3fParameterTypeId:
			{
				float vals[6];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Box3f val( Imath::V3f( vals[0], vals[1], vals[2] ),
									Imath::V3f( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3f, Box3fData>( do_update, val, parm );
				parm->setValue( new IECore::Box3fData(val) );
				break;
			}

			// Box3d
			case IECore::Box3dParameterTypeId:
			{
				float vals[6];
				this->evalFloats( parm_name.c_str(), vals, now );
				Imath::Box3d val( Imath::V3d( vals[0], vals[1], vals[2] ),
									Imath::V3d( vals[3], vals[4], vals[5] ) );
				checkForUpdate<Imath::Box3d, Box3dData>( do_update, val, parm );
				parm->setValue( new IECore::Box3dData(val) );
				break;
			}

			// Compound
			case IECore::CompoundParameterTypeId:
			{
				IECore::msg( IECore::Msg::Warning, "ParameterisedHolder::updateParameter", "todo: need to add code to evaluate compoundParameters and it's children." );
				break;
			}

			default:
				IECore::msg( IECore::Msg::Warning, "ParameterisedHolder::updateParameter", "Could not get parameter values from '" + parm_name + "' of type " + parm->typeName() );
				break;
		}
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "ParameterisedHolder::updateParameter", e.what() );
	}
	catch( ... )
	{
		IECore::msg( IECore::Msg::Error, "ParameterisedHolder::updateParameter", "Caught unknown exception" );
	}
}

template<typename BaseType>
IECore::MessageHandler *ParameterisedHolder<BaseType>::getMessageHandler()
{
	return m_messageHandler.get();
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setMessageHandler( IECore::MessageHandler *handler )
{
	m_messageHandler = handler;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::classNames( const std::string searchPathEnvVar, const std::string &matchString, std::vector<std::string> &names )
{
	IECorePython::ScopedGILLock lock;
	std::vector<std::string> classNames;
	try
	{
		std::string pythonCmd = boost::str( format( "IECore.ClassLoader.defaultLoader( \"%s\" ).classNames(\"\%s\")" ) % searchPathEnvVar % matchString );
		object result = CoreHoudini::evalPython( pythonCmd );
		boost::python::list extractedNames = extract<boost::python::list>( result )();
		
		names.clear();
		for ( unsigned i=0; i < extractedNames.attr( "__len__" )(); i++ )
		{
			names.push_back( extract<std::string>( extractedNames[i] ) );
		}
	}
	catch( ... )
	{
		PyErr_Print();
	}
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::classVersions( const std::string className, const std::string searchPathEnvVar, std::vector<int> &versions )
{
	IECorePython::ScopedGILLock lock;
	try
	{
		std::string pythonCmd = boost::str( format( "IECore.ClassLoader.defaultLoader( \"%s\" ).versions( \"%s\" )" ) % searchPathEnvVar % className );
		object result = CoreHoudini::evalPython( pythonCmd );
		boost::python::list extractedVersions = extract<boost::python::list>( result )();
		
		versions.clear();
		for ( unsigned i=0; i < extractedVersions.attr( "__len__" )(); i++ )
		{
			versions.push_back( extract<int>( extractedVersions[i] ) );
		}
	}
	catch( ... )
	{
		PyErr_Print();
	}
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::defaultClassVersion( const std::string className, const std::string searchPathEnvVar )
{
	std::vector<int> versions;
	classVersions( className, searchPathEnvVar, versions );
	
	return versions.empty() ? -1 : versions[versions.size()-1];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Known Specializations
//////////////////////////////////////////////////////////////////////////////////////////

template class ParameterisedHolder<OBJ_Node>;
template class ParameterisedHolder<SOP_Node>;
template class ParameterisedHolder<ROP_Node>;
