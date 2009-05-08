//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>

#include "boost/bind.hpp"

#include "IECore/Parameter.h"
#include "IECore/Exception.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"

using namespace IECore;
using namespace boost;
using namespace std;

const unsigned int Parameter::g_ioVersion = 1;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( Parameter );

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
//////////////////////////////////////////////////////////////////////////////////////////////////////////

Parameter::Parameter()
	:	m_name( "" ), m_description( "" )
{
}

Parameter::Parameter( const std::string &name, const std::string &description, ObjectPtr defaultValue,
	const PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	m_name( name ), m_description( description ), m_defaultValue( defaultValue ), m_presetsOnly( presetsOnly ),
		m_userData( userData ? userData->copy() : 0 )
{
	assert( defaultValue );

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
// Object implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Parameter::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const Parameter *tOther = static_cast<const Parameter *>( other.get() );

	m_name = tOther->m_name;
	m_description = tOther->m_description;

	m_value = tOther->m_value ? context->copy<Object>( tOther->m_value ) : 0;
	m_defaultValue = tOther->m_defaultValue ? context->copy<Object>( tOther->m_defaultValue ) : 0;

	m_presets.clear();
	m_presets.reserve( tOther->m_presets.size() );
	for( PresetsContainer::const_iterator it=tOther->m_presets.begin(); it!=tOther->m_presets.end(); it++ )
	{
		m_presets.push_back( Preset( it->first, context->copy<Object>( it->second ) ) );
	}

	m_presetsOnly = tOther->m_presetsOnly;
	m_userData = tOther->m_userData ? context->copy<CompoundObject>( tOther->m_userData ) : 0;
}

void Parameter::save( SaveContext *context ) const
{
	Object::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), g_ioVersion );

	container->write( "name", m_name.value() );
	container->write( "description", m_description.value() );

	context->save( m_value, container, "value" );
	context->save( m_defaultValue, container, "defaultValue" );

	container->mkdir( "presets" );
	container->chdir( "presets" );

		for( PresetsContainer::const_iterator it=m_presets.begin(); it!=m_presets.end(); it++ )
		{
			context->save( it->second, container, it->first );
		}

	container->chdir( ".." );

	unsigned char tmp = m_presetsOnly;
	container->write( "presetsOnly", tmp );

}

void Parameter::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = g_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	string tmp;
	container->read( "name", tmp ); m_name = tmp;
	container->read( "description", tmp ); m_description = tmp;

	m_value = context->load<Object>( container, "value" );
	m_defaultValue = context->load<Object>( container, "defaultValue" );

	container->chdir( "presets" );

		IndexedIO::EntryList l = container->ls();
		m_presets.clear(); m_presets.resize( l.size() ); unsigned i = 0;
		for( IndexedIO::EntryList::const_iterator it=l.begin(); it!=l.end(); it++ )
		{
			m_presets[i++] = Preset( it->id(), context->load<Object>( container, it->id() ) );
		}

	container->chdir( ".." );

	unsigned char tmpC;
	container->read( "presetsOnly", tmpC );
	m_presetsOnly = tmpC;

}

bool Parameter::isEqualTo( ConstObjectPtr other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}

	const Parameter *tOther = static_cast<const Parameter *>( other.get() );
	if( m_name!=tOther->m_name )
	{
		return false;
	}
	if( m_description!=tOther->m_description )
	{
		return false;
	}

	if( m_value && tOther->m_value )
	{
		if( *m_value!=*tOther->m_value )
		{
			return false;
		}
	}
	else
	{
		if( m_value || tOther->m_value )
		{
			return false;
		}
	}

	if( m_defaultValue && tOther->m_value )
	{
		if( *m_defaultValue!=*tOther->m_defaultValue )
		{
			return false;
		}
	}
	else
	{
		if( m_defaultValue || tOther->m_defaultValue )
		{
			return false;
		}
	}

	if( m_presets.size()!=tOther->m_presets.size() )
	{
		return false;
	}
	for( unsigned i=0; i<m_presets.size(); i++ )
	{
		if( m_presets[i].first != tOther->m_presets[i].first )
		{
			return false;
		}
		if( *(m_presets[i].second) != *(tOther->m_presets[i].second) )
		{
			return false;
		}
	}

	return m_presetsOnly == tOther->m_presetsOnly;
}

void Parameter::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );

	a.accumulate( &m_name.value(), m_name.value().capacity() );
	a.accumulate( &m_description.value(), m_description.value().capacity() );
	a.accumulate( m_value );
	a.accumulate( m_defaultValue );
	for( unsigned i=0; i<m_presets.size(); i++ )
	{
		a.accumulate( m_presets[i].first.capacity() );
		a.accumulate( m_presets[i].second );
	}
	a.accumulate( sizeof( m_presets ) );
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

const Parameter::PresetsContainer &Parameter::presets() const
{
	return m_presets;
}

bool Parameter::presetsOnly() const
{
	return m_presetsOnly;
}

CompoundObjectPtr Parameter::userData()
{
	if( !m_userData )
	{
		m_userData = new CompoundObject();
	}
	return m_userData;
}

ConstCompoundObjectPtr Parameter::userData() const
{
	if( !m_userData )
	{
		m_userData = new CompoundObject();
	}
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
	const PresetsContainer &pr = presets();
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
	const PresetsContainer &pr = presets();
	PresetsContainer::const_iterator it = find_if( pr.begin(), pr.end(), bind( &Preset::first, _1 )==presetName );
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
	const PresetsContainer &pr = presets();
	PresetsContainer::const_iterator it;
	for( it=pr.begin(); it!=pr.end(); it++ )
	{
		if( it->second->isEqualTo( currentValue ) )
		{
			return it->first;
		}
	}

	/// \todo Surely we should never get here if m_presetsOnly is true? Assert that, if safe to do so.

	return "";
}
