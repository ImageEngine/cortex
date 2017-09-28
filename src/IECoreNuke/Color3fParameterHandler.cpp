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

#include "IECoreNuke/Color3fParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<Color3fParameterHandler> Color3fParameterHandler::g_description( Color3fParameter::staticTypeId() );

Color3fParameterHandler::Color3fParameterHandler()
{
}

void Color3fParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	const Color3fParameter *color3fParameter = static_cast<const Color3fParameter *>( parameter );

	if( f.makeKnobs() )
	{
		m_storage = color3fParameter->typedDefaultValue();
	}

	std::string label = knobLabel( parameter );
	m_knob = Color_knob( f, &m_storage.x, knobName, label.c_str() );
	setKnobProperties( parameter, f, m_knob );
}

void Color3fParameterHandler::setParameterValue( Parameter *parameter, ValueSource valueSource )
{
	Imath::Color3f value;
	if( valueSource==Storage )
	{
		value = m_storage;
	}
	else
	{
		value[0] = m_knob->get_value( 0 );
		value[1] = m_knob->get_value( 1 );
		value[2] = m_knob->get_value( 2 );
	}

	static_cast<Color3fParameter *>( parameter )->setTypedValue( value );
}

void Color3fParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	Imath::Color3f value = static_cast<const Color3fParameter *>( parameter )->getTypedValue();
	m_knob->set_value( value[0], 0 );
	m_knob->set_value( value[1], 1 );
	m_knob->set_value( value[2], 2 );
}

