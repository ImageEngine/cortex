//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshPrimitive.h"

#include "IECoreGL/Font.h"
#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/ShaderStateComponent.h"

using namespace IECoreGL;
using namespace std;
using namespace boost;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( Font );

Font::Font( IECore::FontPtr font )
	:	m_font( font ), m_texture( 0 )
{
	m_meshes.resize( 128 );
}

Font::~Font()
{
}

IECore::Font *Font::coreFont()
{
	return m_font.get();
}

const MeshPrimitive *Font::mesh( char c ) const
{
	if( m_meshes[c] )
	{
		return m_meshes[c].get();
	}

	ToGLMeshConverter converter( m_font->mesh( c ) );
	ConstMeshPrimitivePtr mesh = boost::static_pointer_cast<const MeshPrimitive>( converter.convert() );
	m_meshes[c] = mesh;

	return mesh.get();
}

const AlphaTexture *Font::texture() const
{
	if( m_texture )
	{
		return m_texture.get();
	}

	IECore::ConstImagePrimitivePtr image = m_font->image();

	IECore::ConstFloatVectorDataPtr y = image->getChannel<float>( "Y" );
	Imath::V2i s = image->getDataWindow().size() + Imath::V2i( 1 );
	m_texture = new AlphaTexture( s.x, s.y, y );
	return m_texture.get();
}

void Font::renderSprites( const std::string &text ) const
{
	Box2f charBound = m_font->bound();
	V2f origin( 0 );

	float sStep = 1.0f / 16.0f;
	float tStep = 1.0f / 8.0f;
	float eps = 0.001; // a small inset seems necessary to avoid getting the border of the adjacent letters
	for( unsigned int i=0; i<text.size(); i++ )
	{
		char c = text[i];
		int tx = c % 16;
		int ty = 7 - (c / 16);

		glBegin( GL_QUADS );

			glTexCoord2f( tx * sStep + eps, ty * tStep + eps );
			glVertex2f( origin.x + charBound.min.x, origin.y + charBound.min.y );

			glTexCoord2f( (tx + 1) * sStep - eps, ty * tStep + eps );
			glVertex2f( origin.x + charBound.max.x, origin.y + charBound.min.y );

			glTexCoord2f( (tx + 1) * sStep - eps, (ty + 1) * tStep - eps );
			glVertex2f( origin.x + charBound.max.x, origin.y + charBound.max.y );

			glTexCoord2f( tx * sStep + eps, (ty + 1) * tStep - eps);
			glVertex2f( origin.x + charBound.min.x, origin.y + charBound.max.y );

		glEnd();
		if( i < text.size() - 1 )
		{
			origin += m_font->advance( c, text[i+1] );
		}
	}
}

void Font::renderMeshes( const std::string &text, State *state ) const
{
	glPushMatrix();

		for( unsigned i=0; i<text.size(); i++ )
		{
			const Primitive *m = mesh( text[i] );
			m->render( state );
			if( i < text.size() - 1 )
			{
				glTranslate( m_font->advance( text[i], text[i+1] ) );
			}
		}

	glPopMatrix();
}
