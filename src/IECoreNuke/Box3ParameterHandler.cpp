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

#include "IECoreNuke/Box3ParameterHandler.h"

using namespace IECore;
using namespace IECoreNuke;

template<typename T>
ParameterHandler::Description<Box3ParameterHandler<T> > Box3ParameterHandler<T>::g_description( T::staticTypeId() );

template<typename T>
Box3ParameterHandler<T>::Box3ParameterHandler()
	:	m_knob( 0 )
{
}

template<typename T>
void Box3ParameterHandler<T>::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	if( f.makeKnobs() )
	{
		typename T::ValueType defaultValue = static_cast<const T *>( parameter )->typedDefaultValue();
		m_storage.min = defaultValue.min;
		m_storage.max = defaultValue.max;
	}

	std::string label = knobLabel( parameter );
	m_knob = Box3_knob( f, (float *)&m_storage, knobName, label.c_str() );
	setKnobProperties( parameter, f, m_knob );
}

template<typename T>
void Box3ParameterHandler<T>::setParameterValue(IECore::Parameter *parameter, ValueSource valueSource )
{
	T *boxParameter = static_cast<T *>( parameter );

	typename T::ValueType value;
	if( valueSource==Storage )
	{
		value.min = m_storage.min;
		value.max = m_storage.max;
	}
	else
	{
		value.min.x = m_knob->get_value( 0 );
		value.min.y = m_knob->get_value( 1 );
		value.min.z = m_knob->get_value( 2 );
		value.max.x = m_knob->get_value( 3 );
		value.max.y = m_knob->get_value( 4 );
		value.max.z = m_knob->get_value( 5 );
	}

	boxParameter->setTypedValue( value );
}

template<typename T>
void Box3ParameterHandler<T>::setKnobValue( const IECore::Parameter *parameter )
{
	const T *boxParameter = static_cast<const T *>( parameter );
	typename T::ValueType value = boxParameter->getTypedValue();
	m_knob->set_value( value.min.x, 0 );
	m_knob->set_value( value.min.y, 1 );
	m_knob->set_value( value.min.z, 2 );
	m_knob->set_value( value.max.x, 3 );
	m_knob->set_value( value.max.y, 4 );
	m_knob->set_value( value.max.z, 5 );
}

// explicit instantiation

template class Box3ParameterHandler<Box3fParameter>;
template class Box3ParameterHandler<Box3dParameter>;
