//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Font.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/TextureUnits.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TextPrimitive::Type, TextPrimitiveTypeTypeId, TextPrimitive::RenderType, TextPrimitive::Mesh );

}

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
				m_advances.push_back( a );
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

void TextPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	if( !m_text.size() )
	{
		return;
	}

	ConstTypePtr t = state->get<Type>();
	switch( t->value() )
	{
		case Mesh :
			renderMeshes( state, style );
			break;
		case Sprite :
			renderSprites( state, style );
			break;
		default :
			break;
	}
}

void TextPrimitive::renderMeshes( ConstStatePtr state, IECore::TypeId style ) const
{
	if( !m_meshes.size() )
	{
		for( unsigned i=0; i<m_text.size(); i++ )
		{
			ConstMeshPrimitivePtr mesh = m_font->mesh( m_text[i] );
			m_meshes.push_back( mesh );
		}
	}
	
	glPushMatrix();
	
		for( unsigned i=0; i<m_meshes.size(); i++ )
		{
			m_meshes[i]->render( state, style );
			if( i<m_advances.size() )
			{
				glTranslate( m_advances[i] );
			}
		}
	
	glPopMatrix();
}

void TextPrimitive::renderSprites( ConstStatePtr state, IECore::TypeId style ) const
{
	Box2f charBound = m_font->coreFont()->bound();
	glPushAttrib( GL_TEXTURE_BIT | GL_ENABLE_BIT );
	glPushMatrix();
	
		/// \todo We need a better way of dealing with shader push/pop
		/// How about some sort of ScopedProgram class which cleans up
		/// after itself on destruction? Maybe we should generalise that
		/// for all the bindables that don't work because there's no suitable
		/// glPush/Pop for them.
		GLint oldProgram = 0;
		if( GLEW_VERSION_2_0 )
		{
			glGetIntegerv( GL_CURRENT_PROGRAM, &oldProgram );
		}

		glUseProgram( 0 );
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING ); /// \todo Perhaps we could support lighting even in this mode?
		glActiveTexture( textureUnits()[0] );
		m_font->texture()->bind();
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		float sStep = 1.0f / 16.0f;
		float tStep = 1.0f / 8.0f;
		float eps = 0.001; // a small inset seems necessary to avoid getting the border of the adjacent letters
		for( unsigned i=0; i<m_text.size(); i++ )
		{
			char c = m_text[i];
			int tx = c % 16;
			int ty = 7 - (c / 16);
			
			glBegin( GL_QUADS );
			
				glTexCoord2f( tx * sStep + eps, ty * tStep + eps );
				glVertex2f( charBound.min.x, charBound.min.y );
				
				glTexCoord2f( (tx + 1) * sStep - eps, ty * tStep + eps );
				glVertex2f( charBound.max.x, charBound.min.y );
				
				glTexCoord2f( (tx + 1) * sStep - eps, (ty + 1) * tStep - eps );
				glVertex2f( charBound.max.x, charBound.max.y );
				
				glTexCoord2f( tx * sStep + eps, (ty + 1) * tStep - eps);
				glVertex2f( charBound.min.x, charBound.max.y );
			
			glEnd();
			if( i<m_advances.size() )
			{
				glTranslate( m_advances[i] );
			}
		}
		
		if( GLEW_VERSION_2_0 )
		{
			glUseProgram( oldProgram );
		}
	
	glPopMatrix();
	glPopAttrib();
}
