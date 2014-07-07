//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "DDImage/Knobs.h"

#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/MessageHandler.h"

#include "IECoreNuke/CompoundParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<CompoundParameterHandler> CompoundParameterHandler::g_description( CompoundParameter::staticTypeId() );

CompoundParameterHandler::CompoundParameterHandler()
{
}

int CompoundParameterHandler::minimumInputs( const IECore::Parameter *parameter )
{
	int min = 0;
	int max = 0;
	bool error = false;
	inputs( parameter, min, max, error );
	return error ? 0 : min;
}

int CompoundParameterHandler::maximumInputs( const IECore::Parameter *parameter )
{
	int min = 0;
	int max = 0;
	bool error = false;
	inputs( parameter, min, max, error );
	return error ? 0 : max;
}

void CompoundParameterHandler::inputs( const IECore::Parameter *parameter, int &minimum, int &maximum, bool &error )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );

	bool foundOptionalInputs = false;
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), true );
		if( h )
		{
			int min = h->minimumInputs( cIt->get() );
			int max = h->maximumInputs( cIt->get() );
			
			if( min && foundOptionalInputs )
			{
				msg( Msg::Error, "CompoundParameterHandler::inputs", "Parameter needing inputs found after parameter needing optional inputs." );
				error = true;
			}
			
			if( max != min )
			{
				foundOptionalInputs = true;
			}
			
			minimum += min;
			maximum += max;
		}
	}
}

bool CompoundParameterHandler::testInput(  const IECore::Parameter *parameter, int input, const DD::Image::Op *op )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );

	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), true );
		if( h )
		{
			int inputs = h->maximumInputs( cIt->get() );
			if( inputs > input )
			{
				return h->testInput( cIt->get(), input, op );
			}
			input -= inputs; // make indexing relative to the next handler.
		}
	}
	return false;	
}

void CompoundParameterHandler::setParameterValue( IECore::Parameter *parameter, InputIterator first, InputIterator last )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), true );
		if( h )
		{
			int maxInputs = h->maximumInputs( cIt->get() );
			if( maxInputs )
			{
				int minInputs = h->minimumInputs( cIt->get() );
				int numInputs = minInputs==maxInputs ? minInputs : last - first;
				h->setParameterValue( cIt->get(), first, first + numInputs );
				first += numInputs;
			}
		}
	}
}
		
void CompoundParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{	
	beginGroup( parameter, knobName, f );

		childKnobs( parameter, knobName, f );
		
	endGroup( parameter, knobName, f );
}

void CompoundParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), false );
		if( h )
		{
			h->setParameterValue( cIt->get(), valueSource );
		}
	}
}

void CompoundParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), false );
		if( h )
		{
			h->setKnobValue( cIt->get() );
		}
	}
}

void CompoundParameterHandler::setState( IECore::Parameter *parameter, const IECore::Object *state )
{
	const CompoundObject *o = static_cast<const CompoundObject *>( state );
	CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
	
	const CompoundObject::ObjectMap &members = o->members();
	for( CompoundObject::ObjectMap::const_iterator it = members.begin(); it!=members.end(); it++ )
	{
		Parameter *child = compoundParameter->parameter<Parameter>( it->first );
		if( child )
		{
			ParameterHandlerPtr h = handler( child, true );
			if( h )
			{
				h->setState( child, it->second.get() );
			}
		}
	}
}

IECore::ObjectPtr CompoundParameterHandler::getState( const IECore::Parameter *parameter )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	CompoundObjectPtr result = new CompoundObject;
	
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), true );
		if( h )
		{
			ObjectPtr childState = h->getState( cIt->get() );
			if( childState )
			{
				result->members()[(*cIt)->name()] = childState;
			}
		}
	}
	
	if( result->members().size() )
	{
		return result;
	}
	
	return 0;
}

void CompoundParameterHandler::beginGroup( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( 0==strcmp( knobName, "parm" ) )
	{
		// we don't need any grouping for the top level compound parameter
		return;
	}
	
	std::string label = knobLabel( parameter );
	
	switch( containerType( parameter ) )
	{
		case Tab :
			DD::Image::Tab_knob( f, label.c_str() );
			break;
		case Toolbar :
			DD::Image::BeginToolbar( f, knobName, label.c_str() );
			break;
		case Collapsible :
		default :
			DD::Image::BeginClosedGroup( f, knobName, label.c_str() );
			break;
	}
}

void CompoundParameterHandler::endGroup( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( 0==strcmp( knobName, "parm" ) )
	{
		// we don't need any grouping for the top level compound parameter
		return;
	}
	
	switch( containerType( parameter ) )
	{
		case Tab :
			// don't need to do anything to close a tab
			break;
		case Toolbar :
			DD::Image::EndToolbar( f );
			break;
		case Collapsible :
		default :
			DD::Image::EndGroup( f );
			break;
	}
}		
				
void CompoundParameterHandler::childKnobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );

	bool inTabGroup = false;
	const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
	for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
	{
		ParameterHandlerPtr h = handler( cIt->get(), true );
		if( h )
		{
			bool wantTabGroup = false;
			const CompoundParameter *childCompound = runTimeCast<CompoundParameter>( cIt->get() );
			if( childCompound )
			{
				wantTabGroup = containerType( childCompound ) == Tab;
			}
			if( wantTabGroup && !inTabGroup )
			{
				DD::Image::BeginTabGroup( f, "" );
				inTabGroup = true;
			}
			else if( !wantTabGroup && inTabGroup )
			{
				DD::Image::EndTabGroup( f );
				inTabGroup = false;
			}
		
			std::string childKnobName = std::string( knobName ) + "_" + (*cIt)->name();
			h->knobs( cIt->get(), childKnobName.c_str(), f );
		}
	}
}

CompoundParameterHandler::ContainerType CompoundParameterHandler::containerType( const IECore::Parameter *parameter )
{
	const CompoundObject *userData = parameter->userData();
	const CompoundObject *ui = userData->member<CompoundObject>( "UI" );
	if( !ui )
	{
		return Collapsible;
	}
	
	const StringData *typeHint = ui->member<StringData>( "typeHint" );
	if( !typeHint )
	{
		return Collapsible;	
	}
	
	if( typeHint->readable()=="collapsible" || typeHint->readable()=="collapsable" )
	{
		return Collapsible;
	}
	else if( typeHint->readable()=="tab" )
	{
		return Tab;
	}
	else if( typeHint->readable()=="toolbar" )
	{
		return Toolbar;
	}

	return Collapsible;
}

std::string CompoundParameterHandler::knobLabel( const IECore::Parameter *parameter ) const
{	
	// Code to display the same label as would be displayed in maya.
	// this relies on the convention of having an invisible StringParameter named
	// label immediately under the CompoundParameter. not very pretty.
	/// \todo Perhaps we could come up with a better means of specifying labels and header
	/// parameters for ClassParameter and ClassVectorParameter.
	const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
	const StringParameter *labelParameter = compoundParameter->parameter<StringParameter>( "label" );
	if( labelParameter )
	{
		const CompoundObject *userData = labelParameter->userData();
		const CompoundObject *ui = userData->member<CompoundObject>( "UI" );
		if( ui )
		{
			const BoolData *visible = ui->member<BoolData>( "visible" );
			if( visible && !visible->readable() )
			{
				return labelParameter->getTypedValue();
			}
		}
	}
		
	return ParameterHandler::knobLabel( parameter );
}

ParameterHandlerPtr CompoundParameterHandler::handler( const Parameter *child, bool createIfMissing )
{
	HandlerMap::const_iterator it = m_handlers.find( child->internedName() );
	if( it!=m_handlers.end() )
	{
		return it->second;
	}
	
	if( !createIfMissing )
	{
		return 0;
	}
	
	ParameterHandlerPtr h = ParameterHandler::create( child );
	if( !h )
	{
		IECore::msg( IECore::Msg::Warning, "IECoreNuke::CompoundParameterHandler", boost::format(  "Unable to create handler for parameter \"%s\" of type \"%s\"" ) % child->name() % child->typeName() );
	}
	
	m_handlers[child->internedName()] = h;
	return h;
}
