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

#include "IECore/ValidatedStringParameter.h"
#include "IECore/CompoundObject.h"

#include "boost/regex.h"

using namespace boost;
using namespace std;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ValidatedStringParameter );

ValidatedStringParameter::ValidatedStringParameter( const std::string &name, const std::string &description,
	const std::string &regex, const std::string &regexDescription, const std::string &defaultValue, bool allowEmptyString, const StringParameter::PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	StringParameter( name, description, defaultValue, presets, presetsOnly, userData ), m_regex( regex ), m_regexDescription( regexDescription ), m_allowEmptyString( allowEmptyString )
{
}

const std::string &ValidatedStringParameter::regex() const
{
	return m_regex;
}

const std::string &ValidatedStringParameter::regexDescription() const
{
	return m_regex;
}

bool ValidatedStringParameter::allowEmptyString() const
{
	return m_allowEmptyString;
}

bool ValidatedStringParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if( !StringParameter::valueValid( value, reason ) )
	{
		return false;
	}
	// if the above passed we know we have a string
	ConstStringDataPtr s = static_pointer_cast<const StringData>( value );
	if( m_allowEmptyString && s->readable()=="" )
	{
		return true;
	}
	if( regex_match( s->readable(), boost::regex( m_regex ) ) )
	{
		return true;
	}
	if( reason )
	{
		if( m_regexDescription!="" )
		{
			*reason = m_regexDescription;
		}
		else
		{
			*reason = "String doesn't match the regular expression \"" + m_regex + "\"";
			if( m_allowEmptyString )
			{
				*reason += " or \"\"";
			}
		}
	}
	return false;
}
