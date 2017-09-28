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

#include "IECore/SimpleTypedParameter.h"

#include "IECoreNuke/StringParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<StringParameterHandler> StringParameterHandler::g_description( StringParameter::staticTypeId() );

StringParameterHandler::StringParameterHandler()
	:	m_storage( 0 ), m_knob( 0 )
{
}

void StringParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( f.makeKnobs() )
	{
		m_storage = static_cast<const StringParameter *>( parameter )->typedDefaultValue().c_str();
	}

	m_knob = knob( parameter, knobName, f, &m_storage );
	/// we have a lot of procedurals which do their own variable expansion using a SubstitutedDict,
	/// and the variables in the strings confuse nuke no end, so we're disabling expressions for now.
	/// \todo Can we do better and allow the two to coexist?
	SetFlags( f, DD::Image::Knob::NO_ANIMATION );
	setKnobProperties( parameter, f, m_knob );
}

void StringParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	StringParameter *stringParameter = static_cast<StringParameter *>( parameter );
	if( valueSource==Storage )
	{
		stringParameter->setTypedValue( m_storage );
	}
	else
	{
		std::ostringstream s;
		m_knob->to_script( s, 0, false );
		stringParameter->setTypedValue( s.str() );
	}
}

void StringParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const StringParameter *stringParameter = static_cast<const StringParameter *>( parameter );
	m_knob->set_text( stringParameter->getTypedValue().c_str() );
}

DD::Image::Knob *StringParameterHandler::knob( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f, const char **storage )
{
	std::string label = knobLabel( parameter );
	return String_knob( f, storage, knobName, label.c_str() );
}
