//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Parameter.h"

#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"
#include "IECore/NullObject.h"

#include <algorithm>

using namespace IECore;
using namespace boost;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Parameter );

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
//////////////////////////////////////////////////////////////////////////////////////////////////////////

Parameter::Parameter( const std::string &name, const std::string &description, ObjectPtr defaultValue,
	const PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	m_name( name ), m_description( description ), m_defaultValue( defaultValue ), m_presetsOnly( presetsOnly ),
		m_userData( userData ? userData->copy() : nullptr )
{
	if ( !defaultValue )
	{
		throw Exception( "Invalid NULL default value!" );
	}

	for( PresetsContainer::const_iterator it=presets.begin(); it!=presets.end(); it++ )
	{
		m_presets.push_back( PresetsContainer::value_type( it->first, it->second->copy() ) );
	}

	/// \todo If presetsOnly is true, doesn't this allow us to set a defaultValue that isn't in the presets list?
	setValue( defaultValue->copy() );
}

Parameter::~Parameter()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Basic Parameter information
//////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string &Parameter::name() const
{
	return m_name;
}

const std::string &Parameter::description() const
{
	return m_description;
}

const Object *Parameter::defaultValue() const
{
	return m_defaultValue.get();
}

const Parameter::PresetsContainer &Parameter::getPresets() const
{
	return m_presets;
}

void Parameter::setPresets( const PresetsContainer &presets )
{
	m_presets = presets;
}

bool Parameter::presetsOnly() const
{
	return m_presetsOnly;
}

CompoundObject *Parameter::userData()
{
	if( !m_userData )
	{
		m_userData = new CompoundObject();
	}
	return m_userData.get();
}

const CompoundObject *Parameter::userData() const
{
	if( !m_userData )
	{
		m_userData = new CompoundObject();
	}
	return m_userData.get();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Parameter::valueValid( const Object *value, std::string *reason ) const
{
	if( !value )
	{
		if( reason )
		{
			*reason = "Value is a null pointer.";
		}
		return false;
	}
	if( value->typeId()==NullObject::staticTypeId() )
	{
		if( reason )
		{
			*reason = "Value is of type NullObject.";
		}
		return false;
	}
	if( !m_presetsOnly )
	{
		return true;
	}
	const PresetsContainer &pr = getPresets();
	for( PresetsContainer::const_iterator it = pr.begin(); it!=pr.end(); it++ )
	{
		if( it->second->isEqualTo( value ) )
		{
			return true;
		}
	}
	if( reason )
	{
		*reason = "Value not present in presets.";
	}
	return false;
}

bool Parameter::valueValid( std::string *reason ) const
{
	return valueValid( getValue(), reason );
}

void Parameter::validate() const
{
	string reason;
	if( !valueValid( getValue(), &reason ) )
	{
		throw Exception( reason );
	}
}

void Parameter::validate( const Object *value ) const
{
	string reason;
	if( !valueValid( value, &reason ) )
	{
		throw Exception( reason );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Value setting
//////////////////////////////////////////////////////////////////////////////////////////////////////////


void Parameter::setValue( ObjectPtr value )
{
	m_value = value;
}

void Parameter::setValidatedValue( ObjectPtr value )
{
	validate( value.get() );
	setValue( value );
}

void Parameter::setValue( const std::string &presetName )
{
	const PresetsContainer &pr = getPresets();

	PresetsContainer::const_iterator it;
	for( it=pr.begin(); it != pr.end(); it++ )
	{
		if ( presetName == it->first )
		{
			break;
		}
	}

	if( it==pr.end() )
	{
		throw Exception( string( "Preset \"" ) + presetName + "\" does not exist." );
	}
	setValue( it->second->copy() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Value getting
//////////////////////////////////////////////////////////////////////////////////////////////////////////

Object *Parameter::getValue()
{
	return m_value.get();
}

const Object *Parameter::getValue() const
{
	return m_value.get();
}

Object *Parameter::getValidatedValue()
{
	validate();
	return getValue();
}

const Object *Parameter::getValidatedValue() const
{
	validate();
	return getValue();
}

std::string Parameter::getCurrentPresetName() const
{
	// ideally we'd just use m_value for currentValue, which would mean we
	// didn't have to do a copy of the value. but that breaks with CompoundParameter
	// as it builds the value dynamically in getValue().
	const Object *currentValue = getValue();
	const PresetsContainer &pr = getPresets();
	PresetsContainer::const_iterator it;
	for( it=pr.begin(); it!=pr.end(); it++ )
	{
		if( it->second->isEqualTo( currentValue ) )
		{
			return it->first;
		}
	}

	return "";
}
