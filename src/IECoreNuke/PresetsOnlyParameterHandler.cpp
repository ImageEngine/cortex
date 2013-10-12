//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/PresetsOnlyParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

PresetsOnlyParameterHandler::PresetsOnlyParameterHandler()
	:	m_knob( 0 )
{
}
		
void PresetsOnlyParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( f.makeKnobs() )
	{
		m_names.clear();
		const Parameter::PresetsContainer &presets = parameter->getPresets();
		for( Parameter::PresetsContainer::const_iterator it = presets.begin(); it!=presets.end(); it++ )
		{
			if( it->second->isEqualTo( parameter->defaultValue() ) )
			{
				m_storage = m_names.size();
			}

			InternedString presetName( it->first );
			m_names.push_back( presetName.value().c_str() );
		}
		m_names.push_back( 0 );
	}
	
	std::string label = knobLabel( parameter );
	m_knob = Enumeration_knob( f, &m_storage, &(m_names[0]), knobName, label.c_str() );
	
	setKnobProperties( parameter, f, m_knob );
}

void PresetsOnlyParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
	int presetIndex = 0;
	if( valueSource==Storage )
	{
		presetIndex = m_storage;
	}
	else
	{
		presetIndex = (int)m_knob->get_value();
	}
	parameter->setValue( parameter->getPresets()[m_storage].second );
}

void PresetsOnlyParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
	const Parameter::PresetsContainer &presets = parameter->getPresets();
	std::string currentPresetName = parameter->getCurrentPresetName();
	size_t presetIndex = 0;
	for( Parameter::PresetsContainer::const_iterator it = presets.begin(); it!=presets.end(); it++, presetIndex++ )
	{
		if( it->first==currentPresetName )
		{
			m_knob->set_value( presetIndex );
		}
	}
}
