//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECoreGL/TextPrimitive.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Font.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TextPrimitive::Type, TextPrimitiveTypeTypeId, TextPrimitive::RenderType, TextPrimitive::Mesh );

}

IE_CORE_DEFINERUNTIMETYPED( TextPrimitive );

TextPrimitive::TextPrimitive( const std::string &text, FontPtr font )
	:	m_font( font ), m_text( text )
{
	if( m_text.size() )
	{
		Box2f b;
		V2f advanceSum( 0 );
		for( unsigned i=0; i<m_text.size(); i++ )
		{
			Box2f b = m_font->coreFont()->bound( m_text[i] );
			b.min += advanceSum;
			b.max += advanceSum;
			if( i<m_text.size() - 1 )
			{
				V2f a = m_font->coreFont()->advance( m_text[i], m_text[i+1] );
				advanceSum += a;
			}
		}
		m_bound.min = V3f( b.min.x, b.min.y, 0 );
		m_bound.max = V3f( b.max.x, b.max.y, 0 );
	}

}

TextPrimitive::~TextPrimitive()
{
}

Imath::Box3f TextPrimitive::bound() const
{
	return m_bound;
}

void TextPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
}

void TextPrimitive::render( State *currentState ) const
{
	if( !m_text.size() )
	{
		return;
	}

	switch( currentState->get<Type>()->value() )
	{
		case Mesh :
			renderMeshes( currentState );
			break;
		case Sprite :
			renderSprites( currentState );
			break;
		default :
			break;
	}
}

void TextPrimitive::renderInstances( size_t numInstances ) const
{
	// should never get here, because we override the master render()
	// method above.
	assert( 0 );
}

void TextPrimitive::renderMeshes( State *state ) const
{
	m_font->renderMeshes( m_text, state );
}

void TextPrimitive::renderSprites( State *state ) const
{
	if( !state->get<Primitive::DrawSolid>()->value() )
	{
		return;
	}

	glPushAttrib( GL_TEXTURE_BIT | GL_ENABLE_BIT );
	glPushMatrix();

		/// \todo Can we integrate this with the Shader::Setup mechanism, and allow
		/// custom shaders to be allowed for sprite rendering as well?
		GLint oldProgram = 0;
		glGetIntegerv( GL_CURRENT_PROGRAM, &oldProgram );
		glUseProgram( 0 );

		glEnable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING ); /// \todo Perhaps we could support lighting even in this mode?
		glActiveTexture( GL_TEXTURE0 );
		m_font->texture()->bind();
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		m_font->renderSprites( m_text );

		glUseProgram( oldProgram );

	glPopMatrix();
	glPopAttrib();
}
