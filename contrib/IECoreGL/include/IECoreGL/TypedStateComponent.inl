//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_TYPEDSTATECOMPONENT_INL
#define IECOREGL_TYPEDSTATECOMPONENT_INL

namespace IECoreGL
{


template<typename T, unsigned int TId>
StateComponent::Description<TypedStateComponent<T, TId> > TypedStateComponent<T,TId>::g_description;

template<typename T, unsigned int TId>
TypedStateComponent<T,TId>::TypedStateComponent()
	:	m_value( defaultValue() )
{
}

template<typename T, unsigned int TId>
TypedStateComponent<T,TId>::TypedStateComponent( const T &value )
	:	m_value( value )
{
}

template<typename T, unsigned int TId>
IECore::TypeId TypedStateComponent<T,TId>::typeId() const
{
	return (IECore::TypeId)TId;
}
		
template<typename T, unsigned int TId>
bool TypedStateComponent<T,TId>::isInstanceOf( IECore::TypeId typeId ) const
{
	if( typeId==staticTypeId() )
	{
		return true;
	}
	return StateComponent::isInstanceOf( typeId );
}

template<typename T, unsigned int TId>
bool TypedStateComponent<T,TId>::isInstanceOf( const std::string &typeName ) const
{
	if( typeName==staticTypeName() )
	{
		return true;
	}
	return StateComponent::isInstanceOf( typeName );
}

template<typename T, unsigned int TId>
IECore::TypeId TypedStateComponent<T,TId>::staticTypeId()
{
	return (IECore::TypeId)TId;
}

template<typename T, unsigned int TId>
bool TypedStateComponent<T,TId>::inheritsFrom( IECore::TypeId typeId )
{
	return StateComponent::staticTypeId()==typeId ? true : State::inheritsFrom( typeId );
}

template<typename T, unsigned int TId>
bool TypedStateComponent<T,TId>::inheritsFrom( const std::string &typeName )
{
	return StateComponent::staticTypeName()==typeName ? true : StateComponent::inheritsFrom( typeName );
}

template<typename T, unsigned int TId>
IECore::TypeId TypedStateComponent<T,TId>::baseTypeId()
{
	return (IECore::TypeId)(StateComponent::staticTypeId());
}

template<typename T, unsigned int TId>
std::string TypedStateComponent<T,TId>::baseTypeName()
{
	return StateComponent::staticTypeName();
}

template<typename T, unsigned int TId>
const T &TypedStateComponent<T,TId>::value() const
{
	return m_value;
}

template<typename T, unsigned int TId>
void TypedStateComponent<T,TId>::bind() const
{
}

template<typename T, unsigned int TId>
GLbitfield TypedStateComponent<T,TId>::mask() const
{
	return 0;
}

} // namespace IECoreGL

#endif // IECOREGL_TYPEDSTATECOMPONENT_INL
