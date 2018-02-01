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

#include "IECoreNuke/VecParameterHandler.h"

#include "IECore/SimpleTypedParameter.h"

#include "DDImage/Knobs.h"

using namespace IECore;
using namespace IECoreNuke;

template<typename T>
ParameterHandler::Description<VecParameterHandler<T> > VecParameterHandler<T>::g_description( T::staticTypeId() );

template<typename T>
VecParameterHandler<T>::VecParameterHandler()
{
}

template<typename T>
void VecParameterHandler<T>::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
	const T *vecParameter = static_cast<const T *>( parameter );

	if( f.makeKnobs() )
	{
		typename T::ValueType defaultValue = vecParameter->typedDefaultValue();
		for( unsigned i=0; i<T::ValueType::dimensions(); i++ )
		{
			m_storage[i] = defaultValue[i];
		}
	}

	std::string label = knobLabel( parameter );
	if( T::ValueType::dimensions()==2 )
	{
		m_knob = XY_knob( f, m_storage, knobName, label.c_str() );
		SetFlags( f, DD::Image::Knob::NO_PROXYSCALE | DD::Image::Knob::NO_HANDLES );
	}
	else
	{
		assert( T::ValueType::dimensions()==3 );
		m_knob = XYZ_knob( f, m_storage, knobName, label.c_str() );
	}

	setKnobProperties( parameter, f, m_knob );
}

template<typename T>
void VecParameterHandler<T>::setParameterValue( Parameter *parameter, ValueSource valueSource )
{
	typename T::ValueType value;
	if( valueSource==Storage )
	{
		for( unsigned i=0; i<T::ValueType::dimensions(); i++ )
		{
			value[i] = m_storage[i];
		}
	}
	else
	{
		for( unsigned i=0; i<T::ValueType::dimensions(); i++ )
		{
			value[i] = m_knob->get_value( i );
		}
	}
	static_cast<T *>( parameter )->setTypedValue( value );
}

template<typename T>
void VecParameterHandler<T>::setKnobValue( const IECore::Parameter *parameter )
{
	typename T::ValueType value = static_cast<const T *>( parameter )->getTypedValue();
	for( unsigned i=0; i<T::ValueType::dimensions(); i++ )
	{
		m_knob->set_value( value[i], i );
	}
}

// explicit instantiation

template class VecParameterHandler<V2fParameter>;
template class VecParameterHandler<V2dParameter>;
template class VecParameterHandler<V3fParameter>;
template class VecParameterHandler<V3dParameter>;
