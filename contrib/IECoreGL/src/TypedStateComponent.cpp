//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/TypedStateComponent.h"

using namespace Imath;

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

#define SPECIALISE( TYPE, BASETYPE, DEFAULTVALUE )											\
	template<>																				\
	std::string TYPE::typeName() const														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	std::string TYPE::staticTypeName()														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	BASETYPE TYPE::defaultValue()															\
	{																						\
		return DEFAULTVALUE;																\
	}																						\

#define SPECIALISE_AND_INSTANTIATE( TYPE, BASETYPE, DEFAULTVALUE )							\
	SPECIALISE( TYPE, BASETYPE, DEFAULTVALUE )												\
	template class TypedStateComponent<BASETYPE, TYPE ## TypeId>;							\
	
// color specialisation and instantiation
/////////////////////////////////////////////////////////////////////

SPECIALISE( Color, Color4f, Color4f( 1, 1, 1, 1 ) );

template<>
void Color::bind() const
{
	glColor4f( m_value.r, m_value.g, m_value.b, m_value.a );
}

template<>
GLbitfield Color::mask() const
{
	return GL_CURRENT_BIT;
}

template class TypedStateComponent<Color4f, ColorTypeId>;

// blending specialisations and instantiations
//////////////////////////////////////////////////////////////////////

SPECIALISE( BlendColorStateComponent, Color4f, Color4f( 1, 1, 1, 1 ) );

template<>
void BlendColorStateComponent::bind() const
{
	glBlendColor( m_value.r, m_value.g, m_value.b, m_value.a );
}

template<>
GLbitfield BlendColorStateComponent::mask() const
{
	return GL_COLOR_BUFFER_BIT;
}

template class TypedStateComponent<Color4f, BlendColorStateComponentTypeId>;

SPECIALISE( BlendFuncStateComponent, BlendFactors, BlendFactors( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

BlendFactors::BlendFactors( GLenum s, GLenum d )
	:	src( s ), dst( d )
{
}

BlendFactors::BlendFactors( const BlendFactors &other )
	:	src( other.src ), dst( other.dst )
{
}

template<>
void BlendFuncStateComponent::bind() const
{
	glBlendFunc( m_value.src, m_value.dst );
}

template<>
GLbitfield BlendFuncStateComponent::mask() const
{
	return GL_COLOR_BUFFER_BIT;
}

template class TypedStateComponent<BlendFactors, BlendFuncStateComponentTypeId>;

SPECIALISE( BlendEquationStateComponent, GLenum, GL_FUNC_ADD );

template<>
void BlendEquationStateComponent::bind() const
{
	glBlendEquation( m_value );
}

template<>
GLbitfield BlendEquationStateComponent::mask() const
{
	return GL_COLOR_BUFFER_BIT;
}

template class TypedStateComponent<GLenum, BlendEquationStateComponentTypeId>;

// doubleSided specialisations and instantiations
//////////////////////////////////////////////////////////////////////

SPECIALISE( DoubleSidedStateComponent, bool, true );

template<>
void DoubleSidedStateComponent::bind() const
{
	if( m_value )
	{
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
	}
}

template<>
GLbitfield DoubleSidedStateComponent::mask() const
{
	return GL_ENABLE_BIT;
}

template class TypedStateComponent<bool, DoubleSidedStateComponentTypeId>;

// RightHandedOrientation specialisations and instantiations
//////////////////////////////////////////////////////////////////////

SPECIALISE( RightHandedOrientationStateComponent, bool, true );

template<>
void RightHandedOrientationStateComponent::bind() const
{
	glFrontFace( m_value ? GL_CCW : GL_CW );
}

template<>
GLbitfield RightHandedOrientationStateComponent::mask() const
{
	return GL_POLYGON_BIT;
}

template class TypedStateComponent<bool, RightHandedOrientationStateComponentTypeId>;

// instantiation of simple mask()==0 types
//////////////////////////////////////////////////////////////////////

SPECIALISE_AND_INSTANTIATE( PrimitiveBound, bool, false );
SPECIALISE_AND_INSTANTIATE( PrimitiveWireframe, bool, false );
SPECIALISE_AND_INSTANTIATE( PrimitiveWireframeWidth, float, 1.0f );
SPECIALISE_AND_INSTANTIATE( PrimitiveSolid, bool, true );
SPECIALISE_AND_INSTANTIATE( PrimitiveOutline, bool, false );
SPECIALISE_AND_INSTANTIATE( PrimitiveOutlineWidth, float, 1.0f );
SPECIALISE_AND_INSTANTIATE( PrimitivePoints, bool, false );
SPECIALISE_AND_INSTANTIATE( PrimitivePointWidth, float, 1.0f );
SPECIALISE_AND_INSTANTIATE( PrimitiveTransparencySortStateComponent, bool, true );
SPECIALISE_AND_INSTANTIATE( TransparentShadingStateComponent, bool, false );
SPECIALISE_AND_INSTANTIATE( BoundColorStateComponent, Color4f, Color4f( 0.36, 0.8, 0.85, 1 ) );
SPECIALISE_AND_INSTANTIATE( WireframeColorStateComponent, Color4f, Color4f( 0.25, 0.6, 0.85, 1 ) );
SPECIALISE_AND_INSTANTIATE( OutlineColorStateComponent, Color4f, Color4f( 0.85, 0.75, 0.45, 1 ) );
SPECIALISE_AND_INSTANTIATE( PointColorStateComponent, Color4f, Color4f( 0.85, 0.45, 0, 1 ) );
SPECIALISE_AND_INSTANTIATE( PointsPrimitiveUseGLPoints, UseGLPoints, ForPointsOnly );
SPECIALISE_AND_INSTANTIATE( PointsPrimitiveGLPointWidth, float, 1.0f );

} // namespace IECoreGL
