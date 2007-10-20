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

#include "IECore/Parameter.h"
#include "IECore/Exception.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"

using namespace IECore;
using namespace boost;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
//////////////////////////////////////////////////////////////////////////////////////////////////////////

Parameter::Parameter( const std::string &name, const std::string &description, ObjectPtr defaultValue,
	const PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	m_name( name ), m_description( description ), m_defaultValue( defaultValue ), m_presetsOnly( presetsOnly ),
		m_userData( userData ? userData->copy() : new CompoundObject() )
{
	for( PresetsMap::const_iterator it=presets.begin(); it!=presets.end(); it++ )
	{
		m_presets.insert( PresetsMap::value_type( it->first, it->second->copy() ) );
	}
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

ConstObjectPtr Parameter::defaultValue() const
{
	return m_defaultValue;
}

const Parameter::PresetsMap &Parameter::presets() const
{
	return m_presets;
}

bool Parameter::presetsOnly() const
{
	return m_presetsOnly;
}

CompoundObjectPtr Parameter::userData()
{
	return m_userData;
}

ConstCompoundObjectPtr Parameter::userData() const
{
	return m_userData;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Parameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
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
	const PresetsMap &pr = presets();
	for( PresetsMap::const_iterator it = pr.begin(); it!=pr.end(); it++ )
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

void Parameter::validate( ConstObjectPtr value ) const
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
	validate( value );
	setValue( value );
}

void Parameter::setValue( const std::string &presetName )
{
	const PresetsMap &pr = presets();
	PresetsMap::const_iterator it = pr.find( presetName );
	if( it==pr.end() )
	{
		throw Exception( string( "Preset \"" ) + presetName + "\" does not exist." );
	}
	setValue( it->second->copy() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Value getting
//////////////////////////////////////////////////////////////////////////////////////////////////////////

ObjectPtr Parameter::getValue()
{
	return m_value;
}

ConstObjectPtr Parameter::getValue() const
{
	return m_value;
}

ObjectPtr Parameter::getValidatedValue()
{
	validate();
	return getValue();
}
		
ConstObjectPtr Parameter::getValidatedValue() const
{
	validate();
	return getValue();
}
		
std::string Parameter::getCurrentPresetName() const
{
	// ideally we'd just use m_value for currentValue, which would mean we
	// didn't have to do a copy of the value. but that breaks with CompoundParameter
	// as it builds the value dynamically in getValue().
	ConstObjectPtr currentValue = getValue();
	const PresetsMap &pr = presets();
	PresetsMap::const_iterator it;
	for( it=pr.begin(); it!=pr.end(); it++ )
	{
		if( it->second->isEqualTo( currentValue ) )
		{
			return it->first;
		}
	}
	return "";
}
