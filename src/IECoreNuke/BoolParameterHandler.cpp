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

#include "IECoreNuke/BoolParameterHandler.h"

#include "IECore/SimpleTypedParameter.h"

#include "DDImage/Knobs.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<BoolParameterHandler> BoolParameterHandler::g_description( BoolParameter::staticTypeId() );

BoolParameterHandler::BoolParameterHandler()
	:	m_knob( 0 )
{
}

void BoolParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f  )
{
	if( f.makeKnobs() )
	{
		m_storage = static_cast<const BoolParameter *>( parameter )->typedDefaultValue();
	}

	std::string label = knobLabel( parameter );
	m_knob = Bool_knob( f, &m_storage, knobName, label.c_str() );
	DD::Image::SetFlags( f, DD::Image::Knob::STARTLINE );
	setKnobProperties( parameter, f, m_knob );
}

void BoolParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	BoolParameter *boolParameter = static_cast<BoolParameter *>( parameter );
	if( valueSource==Storage )
	{
		boolParameter->setTypedValue( m_storage );
	}
	else
	{
		boolParameter->setTypedValue( m_knob->get_value() > 0.0 );
	}
}

void BoolParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const BoolParameter *boolParameter = static_cast<const BoolParameter *>( parameter );
	m_knob->set_value( boolParameter->getTypedValue() ? 1.0 : 0.0 );
}
