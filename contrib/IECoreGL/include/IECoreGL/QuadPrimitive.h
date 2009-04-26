//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_QUADPRIMITIVE_H
#define IECOREGL_QUADPRIMITIVE_H

#include "IECoreGL/Primitive.h"

namespace IECoreGL
{

class QuadPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::QuadPrimitive, QuadPrimitiveTypeId, Primitive );

		QuadPrimitive( float width = 1, float height = 1 );
		virtual ~QuadPrimitive();
		
		void setWidth( float width );
		float getWidth() const;
		
		void setHeight( float height );
		float getHeight() const;
		
		virtual Imath::Box3f bound() const;
		
	protected :
	
		virtual void render( ConstStatePtr state, IECore::TypeId style ) const;

	private :
	
		float m_width;
		float m_height;

		// So PointsPrimitive can use the protected render() method to
		// render particle quads.
		friend class PointsPrimitive;
};

IE_CORE_DECLAREPTR( QuadPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_QUADPRIMITIVE_H
