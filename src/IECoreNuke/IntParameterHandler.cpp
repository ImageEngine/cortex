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

#include "IECore/NumericParameter.h"

#include "IECoreNuke/IntParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::Description<IntParameterHandler> IntParameterHandler::g_description( IntParameter::staticTypeId() );

IntParameterHandler::IntParameterHandler()
	:	m_storage( 0 ),
		m_knob( 0 )
{
}

void IntParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	const IntParameter *intParameter = static_cast<const IntParameter *>( parameter );

	if( f.makeKnobs() )
	{
		m_storage = intParameter->numericDefaultValue();
	}

	std::string label = knobLabel( parameter );
	DD::Image::IRange range( intParameter->minValue(), intParameter->maxValue() );
	m_knob = Int_knob( f, &m_storage, range, knobName, label.c_str() );
	DD::Image::SetFlags( f, DD::Image::Knob::FORCE_RANGE );
	setKnobProperties( parameter, f, m_knob );
}

void IntParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	IntParameter *intParameter = static_cast<IntParameter *>( parameter );
	if( valueSource==Storage )
	{
		intParameter->setNumericValue( m_storage );
	}
	else
	{
		intParameter->setNumericValue( (int)m_knob->get_value() );
	}
}

void IntParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const IntParameter *intParameter = static_cast<const IntParameter *>( parameter );
	m_knob->set_value( intParameter->getNumericValue(), 0 );
}

