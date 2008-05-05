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

#include "IECoreGL/TextPrimitive.h"
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Font.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace Imath;

TextPrimitive::TextPrimitive( const std::string &text, FontPtr font )
{
	if( text.size() )
	{
		V3f advanceSum( 0 );
		for( unsigned i=0; i<text.size(); i++ )
		{
			ConstMeshPrimitivePtr mesh = font->mesh( text[i] );
			m_meshes.push_back( mesh );
			Box3f b = mesh->bound();
			b.min += advanceSum;
			b.max += advanceSum;
			m_bound.extendBy( b );
			if( i<text.size() - 1 )
			{
				V2f a = font->coreFont()->advance( text[i], text[i+1] );
				m_advances.push_back( a );
				advanceSum += V3f( a.x, a.y, 0 );
			}
		}
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


