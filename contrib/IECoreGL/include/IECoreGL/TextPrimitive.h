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

#ifndef IECOREGL_TEXTPRIMITIVE_H
#define IECOREGL_TEXTPRIMITIVE_H

#include "IECoreGL/Primitive.h"
#include "IECoreGL/TypedStateComponent.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Font )
IE_CORE_FORWARDDECLARE( MeshPrimitive )

class TextPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( TextPrimitive, TextPrimitiveTypeId, Primitive );

		TextPrimitive( const std::string &text, FontPtr font );
		virtual ~TextPrimitive();
		
		virtual Imath::Box3f bound() const;
		
		//! @name StateComponents
		/// The following StateComponent classes have an effect only on
		/// TextPrimitive objects.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		/// An enum to specifiy the type of primitives used to render text.
		enum RenderType
		{
			Invalid,
			Mesh,
			Sprite
		};
		typedef TypedStateComponent<RenderType, TextPrimitiveTypeTypeId> Type;
		IE_CORE_DECLAREPTR( Type );
		//@}
		
	protected :
		
		virtual void render( ConstStatePtr state, IECore::TypeId style ) const;
	
	private :
	
		FontPtr m_font;
		std::string m_text;
		Imath::Box3f m_bound;
		typedef std::vector<Imath::V2f> AdvanceVector;
		AdvanceVector m_advances;
	
		/// Mesh rendering bits
		void renderMeshes( ConstStatePtr state, IECore::TypeId style ) const;
		typedef std::vector<ConstMeshPrimitivePtr> MeshVector;
		mutable MeshVector m_meshes;
		
		/// Sprite rendering bits
		void renderSprites( ConstStatePtr state, IECore::TypeId style ) const;
	
};

IE_CORE_DECLAREPTR( TextPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_TEXTPRIMITIVE_H
