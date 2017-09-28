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

#ifndef IECOREGL_FONT_H
#define IECOREGL_FONT_H

#include "IECore/Font.h"

#include "IECoreImage/Font.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/TypeIds.h"
#include "IECoreGL/AlphaTexture.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( MeshPrimitive );
IE_CORE_FORWARDDECLARE( State );

class IECOREGL_API Font : public IECore::RunTimeTyped
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Font, FontTypeId, IECore::RunTimeTyped );

		Font( IECore::FontPtr font );
		virtual ~Font();

		IECore::Font *coreFont();

		const MeshPrimitive *mesh( char c ) const;
		const AlphaTexture *texture() const;

		/// Emits a series of quads with appropriate texture coordinates,
		/// such that if you have bound texture() you can render text.
		void renderSprites( const std::string &text ) const;
		/// Renders text as a series of meshes with the specified state.
		void renderMeshes( const std::string &text, State *state ) const;

	private :

		IECore::FontPtr m_font;
		IECoreImage::FontPtr m_imageFont;

		typedef std::vector<ConstMeshPrimitivePtr> MeshVector;
		mutable MeshVector m_meshes;

		mutable ConstAlphaTexturePtr m_texture;

};

IE_CORE_DECLAREPTR( Font );

} // namespace IECoreGL

#endif // IECOREGL_FONT_H
