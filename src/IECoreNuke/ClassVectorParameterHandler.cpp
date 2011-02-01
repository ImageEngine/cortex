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

#include "boost/python/suite/indexing/container_utils.hpp"
#include "boost/algorithm/string/replace.hpp"

#include "DDImage/Knobs.h"
#include "DDImage/Enumeration_KnobI.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundObject.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreNuke/ClassVectorParameterHandler.h"

using namespace boost;
using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<ClassVectorParameterHandler> ClassVectorParameterHandler::g_description( ClassVectorParameterTypeId );

ClassVectorParameterHandler::ClassVectorParameterHandler()
{
}
		
void ClassVectorParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{	
	DD::Image::BeginClosedGroup( f, knobName, knobLabel( parameter ) );
		
		childKnobs( parameter, knobName, f );
		
		addClassKnob( parameter, knobName, f );

	DD::Image::EndGroup( f );
}

void ClassVectorParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	CompoundParameterHandler::setParameterValue( parameter, valueSource );
}

void ClassVectorParameterHandler::setState( IECore::Parameter *parameter, const IECore::Object *state )
{
	const CompoundObject *d = static_cast<const CompoundObject *>( state );
	
	const vector<string> &parameterNames = d->member<StringVectorData>( "__parameterNames" )->readable();
	const vector<string> &classNames = d->member<StringVectorData>( "__classNames" )->readable();
	const vector<int> &classVersions = d->member<IntVectorData>( "__classVersions" )->readable();

	/// \todo C++ code shouldn't have to call python explicity to do this stuff.
	/// We could define an abstract C++ class with the right interface and then derive
	/// from it to implement it in python.
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::list classes;
		for( size_t i=0; i<parameterNames.size(); i++ )
		{
			classes.append( boost::python::make_tuple( parameterNames[i], classNames[i], classVersions[i] ) );
		}
		
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		pythonParameter.attr( "setClasses" )( classes );
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassVectorParameterHandler::setState", e.what() );
	}
	
	CompoundParameterHandler::setState( parameter, state );
}

IECore::ObjectPtr ClassVectorParameterHandler::getState( const IECore::Parameter *parameter )
{
	CompoundObjectPtr result = staticPointerCast<CompoundObject>( CompoundParameterHandler::getState( parameter ) );
	if( !result )
	{
		result = new CompoundObject;
	}
	
	IECorePython::ScopedGILLock gilLock;
	try
	{
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		boost::python::list classes = extract<boost::python::list>( pythonParameter.attr( "getClasses" )( true ) );
		
		IECore::StringVectorDataPtr parameterNames = new StringVectorData;
		IECore::StringVectorDataPtr classNames = new StringVectorData;
		IECore::IntVectorDataPtr classVersions = new IntVectorData;
		int numClasses = len( classes );
		for( int i=0; i<numClasses; i++ )
		{
			parameterNames->writable().push_back( extract<string>( classes[i][1] ) );
			classNames->writable().push_back( extract<string>( classes[i][2] ) );
			classVersions->writable().push_back( extract<int>( classes[i][3] ) );
		}
		
		result->members()["__parameterNames"] = parameterNames;
		result->members()["__classNames"] = classNames;
		result->members()["__classVersions"] = classVersions;
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassVectorParameterHandler::getState", e.what() );
	}
	return result;
}

void ClassVectorParameterHandler::addClassKnob( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	std::string classChooserName = string( knobName ) + "__addClass";
	
	static const char *emptyMenu[] = { " ", "", 0 };
	DD::Image::Knob *classChooser = PyPulldown_knob( f, emptyMenu, classChooserName.c_str(), "Add" );
	
	if( !f.makeKnobs() )
	{
		// making the menu is slow, and only needs doing when we're making knobs (not storing for instance),
		// so early out now to avoid massive slowdown.
		return;
	}
	
	vector<string> menuItems;
	menuItems.push_back( " " );
	menuItems.push_back( "" );
	
	IECorePython::ScopedGILLock gilLock;
	try
	{
		std::string classNameFilter = "*";
		const CompoundObject *userData = parameter->userData();
		if( const CompoundObject *ui = userData->member<CompoundObject>( "UI" ) )
		{
			if( const StringData *classNameFilterData = ui->member<StringData>( "classNameFilter" ) )
			{
				classNameFilter = classNameFilterData->readable();
			}
		}
		
		object mainModule = import( "__main__" );
		object mainModuleNamespace = mainModule.attr( "__dict__" );
	
		boost::python::object pythonParameter( ParameterPtr( const_cast<Parameter *>( parameter ) ) );
		std::string searchPathEnvVar = extract<const char *>( pythonParameter.attr( "searchPathEnvVar" )() )();
		
		object ieCore = import( "IECore" );
		object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultLoader" )( searchPathEnvVar );
		
		object classNames = classLoader.attr( "classNames" )( classNameFilter );
		
		int numClasses = len( classNames );
		for( int i=0; i<numClasses; i++ )
		{
			string className = extract<string>( classNames[i] )();
			object classVersions = classLoader.attr( "versions" )( object( classNames[i] ) );
			int numVersions = len( classVersions );
			
			for( int j=0; j<numVersions; j++ )
			{
				string versionString = extract<const char *>( classVersions[j].attr( "__str__" )() )();
				if( numVersions > 1 )
				{
					/// \todo We need to make this menu nice and hierarchical.
					/// We need the nuke boys to sort that out though.
					menuItems.push_back( className + " v" + versionString );
				}
				else
				{
					menuItems.push_back( className );
				}
				
				std::string parameterPath = knobName + 5; // naughty! we're not meant to know the knob name format
				replace_all( parameterPath, "_", "']['" );
				
				std::string cmd = ( boost::format(
				
					"with IECoreNuke.FnParameterisedHolder( nuke.thisNode() ).parameterModificationContext() as parameters :"
					"	parameter = parameters['%s']; parameter.setClass( parameter.newParameterName(), '%s', %d );"
					
				) % parameterPath % className % versionString ).str();
					
					
				menuItems.push_back( cmd );
			}
			
		}
		
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ClassVectorParameterHandler::addClassKnob", e.what() );
	}
	
	classChooser->enumerationKnob()->menu( menuItems );
}
