//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_BOXPRIMITIVE_H
#define IECOREGL_BOXPRIMITIVE_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Primitive.h"

namespace IECoreGL
{

class IECOREGL_API BoxPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::BoxPrimitive, BoxPrimitiveTypeId, Primitive );

		BoxPrimitive( const Imath::Box3f &box );
		virtual ~BoxPrimitive();

		void setBox( const Imath::Box3f &box );
		const Imath::Box3f getBox() const;

		virtual Imath::Box3f bound() const;
		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar );

		/// Utility function which draws a box using GL_LINES.
		static void renderWireframe( const Imath::Box3f &box );
		/// Utility function which draws a box using GL_QUADS.
		static void renderSolid( const Imath::Box3f &box );

	protected :

		virtual void render( const State * state, IECore::TypeId style ) const;

	private :

		Imath::Box3f m_box;

};

IE_CORE_DECLAREPTR( BoxPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_BOXPRIMITIVE_H
