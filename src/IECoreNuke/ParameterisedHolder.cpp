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

#include "IECoreNuke/ParameterisedHolder.h"

#include "IECoreNuke/ParameterHandler.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "DDImage/Enumeration_KnobI.h"
#include "DDImage/Iop.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/Op.h"

#include "boost/format.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECoreNuke;

static IECore::RunTimeTypedPtr g_getParameterisedResult = 0;
static IECore::RunTimeTypedPtr g_modifiedParametersInput = 0;

template<typename BaseType>
ParameterisedHolder<BaseType>::ParameterisedHolder( Node *node )
	:	BaseType( node ),
		m_classSpecifierKnob( 0 ),
		m_classReloadKnob( 0 ),
		m_classDividerKnob( 0 ),
		m_parameterised( 0 ),
		m_currentClassSpecification( 0 ),
		m_parameterHandler( 0 ),
		m_numParameterKnobs( 0 ),
		m_getParameterisedKnob( 0 ),
		m_modifiedParametersKnob( 0 )
{
	this->inputs( 0 );
}

template<typename BaseType>
ParameterisedHolder<BaseType>::~ParameterisedHolder()
{
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::minimum_inputs() const
{
	const ParameterisedInterface *parameterisedInterface = dynamic_cast<const ParameterisedInterface *>( m_parameterised.get() );
	if( parameterisedInterface )
	{
		return m_parameterHandler->minimumInputs( parameterisedInterface->parameters() );
	}
	return 0;
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::maximum_inputs() const
{
	const ParameterisedInterface *parameterisedInterface = dynamic_cast<const ParameterisedInterface *>( m_parameterised.get() );
	if( parameterisedInterface )
	{
		return m_parameterHandler->maximumInputs( parameterisedInterface->parameters() );
	}
	return 0;
}

template<typename BaseType>
bool ParameterisedHolder<BaseType>::test_input( int input, DD::Image::Op *op ) const
{
	const ParameterisedInterface *parameterisedInterface = dynamic_cast<const ParameterisedInterface *>( m_parameterised.get() );
	if( parameterisedInterface )
	{
		return m_parameterHandler->testInput( parameterisedInterface->parameters(), input, op );
	}
	return false;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::knobs( DD::Image::Knob_Callback f )
{
	BaseType::knobs( f );

	m_classSpecifierKnob = ObjectKnob::objectKnob( f, 0, "classSpecifier", "classSpecifier" );
	SetFlags( f, DD::Image::Knob::KNOB_CHANGED_ALWAYS );

	m_getParameterisedKnob = Button( f, "__getParameterised" );
	SetFlags( f, DD::Image::Knob::KNOB_CHANGED_ALWAYS | DD::Image::Knob::INVISIBLE );

	m_modifiedParametersKnob = Button( f, "__modifiedParameters" );
	SetFlags( f, DD::Image::Knob::KNOB_CHANGED_ALWAYS | DD::Image::Knob::INVISIBLE );

	static const char *noVersions[] = { "No class loaded", "", 0 };
	m_versionChooserKnob = PyPulldown_knob( f, noVersions, "versionChooser", "No class loaded" );

	m_classReloadKnob = Button( f, "classReload", "Reload" );
	SetFlags( f, DD::Image::Knob::KNOB_CHANGED_ALWAYS );

	DD::Image::Knob *classDividerKnob = Divider( f, "" );
	if( classDividerKnob )
	{
		// the Divider() call only returns a value during knob creation, and
		// returns 0 the rest of the time.
		m_classDividerKnob = classDividerKnob;
	}

	// add on the knobs for the parameters. first we must make sure that our held class
	// is up to date. although we update it in knob_changed when a new class or state has
	// been requested, this will only be applied to the one lucky ParameterisedHolder instance
	// that nuke chooses to send knob_changed to. nuke will frequently have other ParameterisedHolder
	// instances for the same node, and those don't get a chance to update. so we call updateParameterised
	// here so they can get synced up too.

	updateParameterised( false );

	BaseType::add_knobs( parameterKnobs, this, f );
}

template<typename BaseType>
int ParameterisedHolder<BaseType>::knob_changed( DD::Image::Knob *knob )
{
	if( knob==m_classSpecifierKnob || knob==m_classReloadKnob )
	{
		// reload the class, or load a new class

		updateParameterised( knob==m_classReloadKnob );

		// regenerate the knobs used to represent the parameters

		replaceKnobs();

		// update the version menu

		updateVersionChooser();

		return 1;
	}
	else if( knob==m_getParameterisedKnob )
	{
		// this is triggered by the FnParameterisedHolder.getParameterised implementation.
		// currently there's no way to get an Op * and call a method on it from
		// python, so we use the knob_changed() mechanism to simulate a function call by
		// shoving the result into g_getParameterisedResult for subsequent retrieval.

		g_getParameterisedResult = loadClass( false );
		if( g_getParameterisedResult )
		{
			ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( g_getParameterisedResult.get() );
			// apply current state
			ConstCompoundObjectPtr classSpecifier = runTimeCast<const CompoundObject>( m_classSpecifierKnob->getValue() );
			ConstObjectPtr handlerState = classSpecifier->member<Object>( "handlerState" );
			if( handlerState )
			{
				m_parameterHandler->setState( parameterisedInterface->parameters(), handlerState.get() );
			}
			// get values directly from knobs as they haven't been stored at this point
			m_parameterHandler->setParameterValue( parameterisedInterface->parameters(), ParameterHandler::Knob );
		}

		return 1;
	}
	else if( knob==m_modifiedParametersKnob )
	{
		// this is triggered by the FnParameterisedHolder.classModificationContext() implementation.
		// as above, we use this method in lieu of being able to call a method on this class.

		// get the new handler state and store it so we have it for save/load copy/paste etc
		////////////////////////////////////////////////////////////////////////////////////

		ParameterisedInterface *inputParameterisedInterface = dynamic_cast<ParameterisedInterface *>( g_modifiedParametersInput.get() );
		ObjectPtr handlerState = m_parameterHandler->getState( inputParameterisedInterface->parameters() );
		CompoundObjectPtr classSpecifier = runTimeCast<CompoundObject>( m_classSpecifierKnob->getValue()->copy() );
		if( handlerState )
		{
			classSpecifier->members()["handlerState"] = handlerState;
		}
		else
		{
			classSpecifier->members().erase( "handlerState" );
		}
		// it seems that setting the value from inside knob_changed() doesn't emit a new knob_changed(), which
		// is fortunately what we want.
		m_classSpecifierKnob->setValue( classSpecifier );

		// apply the new state to the current parameterised object
		////////////////////////////////////////////////////////////////////////////////////

		ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
		if( handlerState )
		{
			m_parameterHandler->setState( parameterisedInterface->parameters(), handlerState.get() );
		}
		parameterisedInterface->parameters()->setValue( inputParameterisedInterface->parameters()->getValue() );

		// update the knobs using our newly updated parameterised object
		////////////////////////////////////////////////////////////////////////////////////

		replaceKnobs();
		setKnobValues();

		// forget the input

		g_modifiedParametersInput = 0;

		return 1;
	}

	return BaseType::knob_changed( knob );
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::_validate( bool forReal )
{
	BaseType::_validate( forReal );

	const std::vector<DD::Image::Op *> &inputs = BaseType::getInputs();
	for( std::vector<DD::Image::Op *>::const_iterator it=inputs.begin(); it!=inputs.end(); it++ )
	{
		if( DD::Image::Iop *iOp = dynamic_cast<DD::Image::Iop *>( *it ) )
		{
			iOp->request( DD::Image::Mask_All, 1 );
		}
	}

	setParameterValues();
}

template<typename BaseType>
IECore::ConstRunTimeTypedPtr ParameterisedHolder<BaseType>::parameterised()
{
	return m_parameterised;
}

template<typename BaseType>
const IECore::ParameterisedInterface *ParameterisedHolder<BaseType>::parameterisedInterface()
{
	return dynamic_cast<const IECore::ParameterisedInterface *>( m_parameterised.get() );
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setParameterValuesFromInputs()
{
	if( m_parameterHandler )
	{
		ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
		m_parameterHandler->setParameterValue( parameterisedInterface->parameters(), BaseType::getInputs().begin(), BaseType::getInputs().end() );
	}
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setParameterValues()
{
	if( m_parameterHandler )
	{
		ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
		m_parameterHandler->setParameterValue( parameterisedInterface->parameters() );
	}
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setKnobValues()
{
	if( m_parameterHandler )
	{
		ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
		m_parameterHandler->setKnobValue( parameterisedInterface->parameters() );
	}
}


template<typename BaseType>
void ParameterisedHolder<BaseType>::buildParameterKnobHandles( DD::Image::ViewerContext *ctx ) const
{
	if( !BaseType::panel_visible() )
	{
		return;
	}

	int knobIndex = 0;
	while( DD::Image::Knob *k = BaseType::knob( knobIndex++ ) )
	{
		if( k->name().compare( 0, 5, "parm_" ) == 0 )
		{
			if( k->build_handle( ctx ) )
			{
				k->add_draw_handle( ctx );
			}
		}
	}
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::replaceKnobs()
{

	// in an ideal world, nuke would notice when the new knobs are the same name/type as the old ones,
	// and keep the values and animation and wotnot - they even have a comment in the docs saying how
	// nice that would be. but that doesn't exist right now, so we do it ourselves. we're doing
	// it here rather than complicating the ParameterHandler mechanism in the hope that in the future
	// Nuke will do it for us and we can then just remove this little bit of code rather than rejig
	// the actual API.

	std::map<std::string, std::string> knobScripts;
	DD::Image::Knob *pKnob = 0;
	for( int i=0; (pKnob = BaseType::knob( i )); i++ )
	{
		if( pKnob->name().compare( 0, 5, "parm_" ) == 0 )
		{
			ostringstream ss;
			pKnob->to_script( ss, 0, false );
			knobScripts[pKnob->name()] = ss.str();
		}
	}

	m_numParameterKnobs = this->replace_knobs( m_classDividerKnob, m_numParameterKnobs, parameterKnobs, this );

	for( int i=0; (pKnob = BaseType::knob( i )); i++ )
	{
		std::map<std::string, std::string>::const_iterator it = knobScripts.find( pKnob->name() );
		if( it!=knobScripts.end() )
		{
			if( pKnob->from_script( it->second.c_str() ) )
			{
				pKnob->changed();
			}
		}
	}

}

template<typename BaseType>
void ParameterisedHolder<BaseType>::parameterKnobs( void *that, DD::Image::Knob_Callback f )
{
	const ParameterisedHolder *parameterisedHolder = static_cast<const ParameterisedHolder *>( that );

	if( parameterisedHolder->m_parameterHandler )
	{
		const ParameterisedInterface *parameterisedInterface = dynamic_cast<const ParameterisedInterface *>( parameterisedHolder->m_parameterised.get() );
		parameterisedHolder->m_parameterHandler->knobs( parameterisedInterface->parameters(), "parm", f );
	}
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::updateVersionChooser()
{
	IECore::ConstCompoundObjectPtr d = IECore::runTimeCast<const IECore::CompoundObject>( m_classSpecifierKnob->getValue() );
	if( !d )
	{
		return;
	}

	// get the versions if there are any

	std::string className = d->member<IECore::StringData>( "className" )->readable();
	int classVersion = d->member<IECore::IntData>( "classVersion" )->readable();
	std::vector<int> classVersions;
	if( m_parameterised )
	{

		std::string classSearchPathEnvVar = d->member<IECore::StringData>( "classSearchPathEnvVar" )->readable();

		{
			IECorePython::ScopedGILLock gilLock;

			object ieCore = import( "IECore" );
			object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultLoader" )( classSearchPathEnvVar );
			object classVersionsObject = classLoader.attr( "versions" )( className );

			container_utils::extend_container( classVersions, classVersionsObject );
		}

	}

	// and update the knob with menu items for each version

	string label;
	vector<string> menuItems;
	if( m_parameterised )
	{
		label = className + " v" + lexical_cast<string>( classVersion );

		for( unsigned i=0; i<classVersions.size(); i++ )
		{
			menuItems.push_back( string( "v" ) + lexical_cast<string>( classVersions[i] ) );

			std::string s =

				"fnPH = IECoreNuke.FnParameterisedHolder( nuke.thisNode() )\n"
				"current = fnPH.getParameterised()\n"
				"fnPH.setParameterised( current[1], " + lexical_cast<string>( classVersions[i] ) + ", current[3] )";

			menuItems.push_back( s );
		}
	}
	else
	{
		label = "No class loaded";
		menuItems.push_back( label );
		menuItems.push_back( "" );
	}

	m_versionChooserKnob->label( label.c_str() );
	m_versionChooserKnob->enumerationKnob()->menu( menuItems );
}

template<typename BaseType>
IECore::RunTimeTypedPtr ParameterisedHolder<BaseType>::loadClass( bool refreshLoader )
{

	std::string className;
	int classVersion;
	std::string classSearchPathEnvVar;

	IECore::ConstCompoundObjectPtr d = IECore::runTimeCast<const IECore::CompoundObject>( ((ObjectKnob *)BaseType::knob( "classSpecifier" ))->getValue() );

	if( d )
	{
		className = d->member<IECore::StringData>( "className" )->readable();
		classVersion = d->member<IECore::IntData>( "classVersion" )->readable();
		classSearchPathEnvVar = d->member<IECore::StringData>( "classSearchPathEnvVar" )->readable();
	}

	if( className=="" )
	{
		return 0;
	}

	IECorePython::ScopedGILLock gilLock;

	try
	{

		// get the loader

		object ieCore = import( "IECore" );
		object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultLoader" )( classSearchPathEnvVar );

		// make sure it's refreshed if required

		if( refreshLoader )
		{
			classLoader.attr( "refresh" )();
		}

		// then load an instance of the class

		object result = classLoader.attr( "load" )( className, classVersion )();

		// and return it

		return extract<RunTimeTypedPtr>( result )();
	}
	catch( error_already_set & )
	{
		msg( Msg::Error, "ParameterisedHolder::loadClass",
			boost::format( "Unable to load class \"%s\" version %d into node %s." ) % className % classVersion % BaseType::node_name() );

		PyErr_Print();
	}
	catch( ... )
	{
		msg( Msg::Error, "ParameterisedHolder::loadClass",
			boost::format( "Unable to load class \"%s\" version %d into node %s." ) % className % classVersion % BaseType::node_name() );
	}
	return 0;

}

template<typename BaseType>
void ParameterisedHolder<BaseType>::updateParameterised( bool reload )
{
	if( !reload && m_currentClassSpecification==m_classSpecifierKnob->getValue() )
	{
		return;
	}

	m_parameterised = loadClass( reload );

	m_parameterHandler = 0;
	ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
	if( parameterisedInterface )
	{
		m_parameterHandler = ParameterHandler::create( parameterisedInterface->parameters() );

		// apply the previously stored handler state

		ConstCompoundObjectPtr classSpecifier = runTimeCast<const CompoundObject>( m_classSpecifierKnob->getValue() );
		ConstObjectPtr handlerState = classSpecifier->member<Object>( "handlerState" );
		if( handlerState )
		{
			m_parameterHandler->setState( parameterisedInterface->parameters(), handlerState.get() );
		}
	}

	m_currentClassSpecification = m_classSpecifierKnob->getValue();
}

template<typename BaseType>
IECore::RunTimeTypedPtr ParameterisedHolder<BaseType>::getParameterisedResult()
{
	IECore::RunTimeTypedPtr result = g_getParameterisedResult;
	g_getParameterisedResult = 0;
	return result;
}

template<typename BaseType>
void ParameterisedHolder<BaseType>::setModifiedParametersInput( IECore::RunTimeTypedPtr parameterised )
{
	g_modifiedParametersInput = parameterised;
}

// explicit instantiation

template class ParameterisedHolder<DD::Image::Op>;

