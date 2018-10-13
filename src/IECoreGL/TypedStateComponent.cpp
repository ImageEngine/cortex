//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

template class TypedStateComponent<Color4f, ColorTypeId>;

// blending specialisations and instantiations
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( BlendColorStateComponent, Color4f, Color4f( 1, 1, 1, 1 ) );

template<>
void BlendColorStateComponent::bind() const
{
	glBlendColor( m_value.r, m_value.g, m_value.b, m_value.a );
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

template class TypedStateComponent<BlendFactors, BlendFuncStateComponentTypeId>;

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( BlendEquationStateComponent, GLenum, GL_FUNC_ADD );

template<>
void BlendEquationStateComponent::bind() const
{
	glBlendEquation( m_value );
}

template class TypedStateComponent<GLenum, BlendEquationStateComponentTypeId>;


IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( AlphaTestStateComponent, bool, false );

template<>
void AlphaTestStateComponent::bind() const
{
	if( m_value )
	{
		glEnable( GL_ALPHA_TEST );
	}
	else
	{
		glDisable( GL_ALPHA_TEST );
	}
}

template class TypedStateComponent<bool, AlphaTestStateComponentTypeId>;



AlphaFunc::AlphaFunc( GLenum m, GLfloat v )
	:	mode( m ), value( v )
{
}

AlphaFunc::AlphaFunc( const AlphaFunc &other )
	:	mode( other.mode ), value( other.value )
{
}

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( AlphaFuncStateComponent, AlphaFunc, AlphaFunc( GL_ALWAYS, 0.0f ) );

template<>
void AlphaFuncStateComponent::bind() const
{
	glAlphaFunc( m_value.mode, m_value.value );
}

template class TypedStateComponent<AlphaFunc, AlphaFuncStateComponentTypeId>;




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

template class TypedStateComponent<bool, DoubleSidedStateComponentTypeId>;

// RightHandedOrientation specialisations and instantiations
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( RightHandedOrientationStateComponent, bool, true );

template<>
void RightHandedOrientationStateComponent::bind() const
{
	glFrontFace( m_value ? GL_CCW : GL_CW );
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

template class TypedStateComponent<bool, PolygonSmoothingStateComponentTypeId>;

// depth specialisations and instantiations
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( DepthTestStateComponent, bool, true );

template<>
void DepthTestStateComponent::bind() const
{
	if( m_value )
	{
		glEnable( GL_DEPTH_TEST );
	}
	else
	{
		glDisable( GL_DEPTH_TEST );
	}
}

template class TypedStateComponent<bool, DepthTestStateComponentTypeId>;

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( DepthMaskStateComponent, bool, true );

template<>
void DepthMaskStateComponent::bind() const
{
	if( m_value )
	{
		glDepthMask( GL_TRUE );
	}
	else
	{
		glDepthMask( GL_FALSE );
	}
}

template class TypedStateComponent<bool, DepthMaskStateComponentTypeId>;

// instantiation of simple types
//////////////////////////////////////////////////////////////////////

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TransparentShadingStateComponent, TransparentShadingStateComponentTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( BoundColorStateComponent, BoundColorStateComponentTypeId, Color4f, Color4f( 0.36, 0.8, 0.85, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( WireframeColorStateComponent, WireframeColorStateComponentTypeId, Color4f, Color4f( 0.25, 0.6, 0.85, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( OutlineColorStateComponent, OutlineColorStateComponentTypeId, Color4f, Color4f( 0.85, 0.75, 0.45, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointColorStateComponent, PointColorStateComponentTypeId, Color4f, Color4f( 0.85, 0.45, 0, 1 ) );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CullingSpaceStateComponent, CullingSpaceStateComponentTypeId, RendererSpace, ObjectSpace );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CullingBoxStateComponent, CullingBoxStateComponentTypeId, Imath::Box3f, Imath::Box3f() );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( ProceduralThreadingStateComponent, ProceduralThreadingStateComponentTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CameraVisibilityStateComponent, CameraVisibilityStateComponentTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( AutomaticInstancingStateComponent, AutomaticInstancingStateComponentTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( VolumeTypeStateComponent, VolumeTypeStateComponentTypeId, int, 0);
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( VolumeGridStateComponent, VolumeGridStateComponentTypeId, std::string, "density");

} // namespace IECoreGL
