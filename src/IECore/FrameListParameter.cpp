//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <iostream>

#include "IECore/FrameList.h"
#include "IECore/CompoundObject.h"
#include "IECore/FrameListParameter.h"

using namespace IECore;

static StringParameter::ObjectPresetsContainer convertPresets( const FrameListParameter::PresetsContainer p )
{
	StringParameter::ObjectPresetsContainer result;
	for( FrameListParameter::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.push_back( StringParameter::ObjectPresetsContainer::value_type( it->first, new StringData( it->second ) ) );
	}
	return result;
}


FrameListParameter::FrameListParameter( const std::string &name, const std::string &description, const std::string &defaultValue, bool allowEmptyList,
			const PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
			: StringParameter( name, description, new StringData( defaultValue ), convertPresets( presets ), presetsOnly, userData ), 
			m_allowEmptyList( allowEmptyList )
{

}

FrameListParameter::FrameListParameter( const std::string &name, const std::string &description, StringDataPtr defaultValue, bool allowEmptyList,
			const ObjectPresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData  )
			: StringParameter( name, description, defaultValue, presets, presetsOnly, userData ), 
			m_allowEmptyList( allowEmptyList )
{
}

FrameListParameter::~FrameListParameter()
{
}		
	
bool FrameListParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if( !StringParameter::valueValid( value, reason ) )
	{
		return false;
	}
	
	ConstStringDataPtr stringValue = assertedStaticCast<const StringData>( value );
	
	try
	{
		FrameListPtr frameList = FrameList::parse( stringValue->readable() );
		assert( frameList ); // If we didn't throw an exception, we must have successfully created a FrameList instance
		
		if ( !m_allowEmptyList && frameList->isInstanceOf( EmptyFrameListTypeId ) )
		{
			if ( reason )
			{
				*reason = "Value must not be empty.";
			}
			
			return false;
		}
		
	}
	catch ( std::exception &e )
	{
		if ( reason )
		{
			*reason = e.what();
		}
		
		return false;
	}
			
	return true;
}
		
void FrameListParameter::setFrameListValue( ConstFrameListPtr frameList )
{
	assert( frameList );
	setTypedValue( frameList->asString() );
}

FrameListPtr FrameListParameter::getFrameListValue() const
{
	return FrameList::parse( getTypedValue() );
}
