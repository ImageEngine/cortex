//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/LineSegment3ParameterHandler.h"

#include "IECore/SimpleTypedParameter.h"

#include "DDImage/Knobs.h"

using namespace IECore;
using namespace IECoreNuke;

template<typename T>
ParameterHandler::Description<LineSegment3ParameterHandler<T> > LineSegment3ParameterHandler<T>::g_description( T::staticTypeId() );

template<typename T>
LineSegment3ParameterHandler<T>::LineSegment3ParameterHandler()
	:	m_startKnob( 0 ), m_endKnob( 0 )
{
}

template<typename T>
void LineSegment3ParameterHandler<T>::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( f.makeKnobs() )
	{
		typename T::ValueType defaultValue = static_cast<const T *>( parameter )->typedDefaultValue();
		m_storage.p0 = defaultValue.p0;
		m_storage.p1 = defaultValue.p1 - defaultValue.p0; // second value in nuke is relative to first
	}

	std::string label = knobLabel( parameter );

	std::string startKnobName = std::string( knobName ) + "Start";
	std::string startKnobLabel = label + " Start";
	m_startKnob = XYZ_knob( f, (float *)&(m_storage.p0), startKnobName.c_str(), startKnobLabel.c_str() );
	setKnobProperties( parameter, f, m_startKnob );

	std::string endKnobName = std::string( knobName ) + "End";
	std::string endKnobLabel = label + " End";
	m_endKnob = XYZ_knob( f, (float *)&(m_storage.p1), endKnobName.c_str(), endKnobLabel.c_str(), m_startKnob );
	setKnobProperties( parameter, f, m_endKnob );
}

template<typename T>
void LineSegment3ParameterHandler<T>::setParameterValue(IECore::Parameter *parameter, ValueSource valueSource )
{
	T *lineParameter = static_cast<T *>( parameter );

	typename T::ValueType value;
	if( valueSource==Storage )
	{
		value.p0 = m_storage.p0;
		value.p1 = m_storage.p0 + m_storage.p1;
	}
	else
	{
		value.p0.x = m_startKnob->get_value( 0 );
		value.p0.y = m_startKnob->get_value( 1 );
		value.p0.z = m_startKnob->get_value( 2 );
		value.p1.x = m_endKnob->get_value( 0 );
		value.p1.y = m_endKnob->get_value( 1 );
		value.p1.z = m_endKnob->get_value( 2 );
		value.p1 += value.p0; // second value in nuke is relative to first
	}

	lineParameter->setTypedValue( value );
}

template<typename T>
void LineSegment3ParameterHandler<T>::setKnobValue( const IECore::Parameter *parameter )
{
	const T *lineParameter = static_cast<const T *>( parameter );
	typename T::ValueType value = lineParameter->getTypedValue();
	m_startKnob->set_value( value.p0.x, 0 );
	m_startKnob->set_value( value.p0.y, 1 );
	m_startKnob->set_value( value.p0.z, 2 );
	m_endKnob->set_value( value.p1.x - value.p0.x, 0 );
	m_endKnob->set_value( value.p1.y - value.p0.y, 1 );
	m_endKnob->set_value( value.p1.z - value.p0.z, 2 );
}

// explicit instantiation

template class LineSegment3ParameterHandler<LineSegment3fParameter>;
template class LineSegment3ParameterHandler<LineSegment3dParameter>;
