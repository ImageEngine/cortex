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
	
// color specialisation and instantiation
/////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( Color, Color4f, Color4f( 1, 1, 1, 1 ) );

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

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( BlendColorStateComponent, Color4f, Color4f( 1, 1, 1, 1 ) );

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

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( BlendFuncStateComponent, BlendFactors, BlendFactors( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

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

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( BlendEquationStateComponent, GLenum, GL_FUNC_ADD );

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

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( DoubleSidedStateComponent, bool, true );

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

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( RightHandedOrientationStateComponent, bool, true );

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

// smoothing specialisations and instantiations
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( LineSmoothingStateComponent, bool, false );

template<>
void LineSmoothingStateComponent::bind() const
{
	if( m_value )
	{
		glEnable( GL_LINE_SMOOTH );
	}
	else
	{
		glDisable( GL_LINE_SMOOTH );
	}
}

template<>
GLbitfield LineSmoothingStateComponent::mask() const
{
	return GL_ENABLE_BIT;
}

template class TypedStateComponent<bool, LineSmoothingStateComponentTypeId>;

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( PointSmoothingStateComponent, bool, false );

template<>
void PointSmoothingStateComponent::bind() const
{
	if( m_value )
	{
		glEnable( GL_POINT_SMOOTH );
	}
	else
	{
		glDisable( GL_POINT_SMOOTH );
	}
}

template<>
GLbitfield PointSmoothingStateComponent::mask() const
{
	return GL_ENABLE_BIT;
}

template class TypedStateComponent<bool, PointSmoothingStateComponentTypeId>;

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( PolygonSmoothingStateComponent, bool, false );

template<>
void PolygonSmoothingStateComponent::bind() const
{
	if( m_value )
	{
		glEnable( GL_POLYGON_SMOOTH );
	}
	else
	{
		glDisable( GL_POLYGON_SMOOTH );
	}
}

template<>
GLbitfield PolygonSmoothingStateComponent::mask() const
{
	return GL_ENABLE_BIT;
}

template class TypedStateComponent<bool, PolygonSmoothingStateComponentTypeId>;

// instantiation of simple mask()==0 types
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveBound, PrimitiveBoundTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveWireframe, PrimitiveWireframeTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveWireframeWidth, PrimitiveWireframeWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveSolid, PrimitiveSolidTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveOutline, PrimitiveOutlineTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveOutlineWidth, PrimitiveOutlineWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitivePoints, PrimitivePointsTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitivePointWidth, PrimitivePointWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PrimitiveTransparencySortStateComponent, PrimitiveTransparencySortStateComponentTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TransparentShadingStateComponent, TransparentShadingStateComponentTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( BoundColorStateComponent, BoundColorStateComponentTypeId, Color4f, Color4f( 0.36, 0.8, 0.85, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( WireframeColorStateComponent, WireframeColorStateComponentTypeId, Color4f, Color4f( 0.25, 0.6, 0.85, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( OutlineColorStateComponent, OutlineColorStateComponentTypeId, Color4f, Color4f( 0.85, 0.75, 0.45, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointColorStateComponent, PointColorStateComponentTypeId, Color4f, Color4f( 0.85, 0.45, 0, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitiveUseGLPoints, PointsPrimitiveUseGLPointsTypeId, UseGLPoints, ForPointsOnly );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitiveGLPointWidth, PointsPrimitiveGLPointWidthTypeId, float, 1.0f );

} // namespace IECoreGL
