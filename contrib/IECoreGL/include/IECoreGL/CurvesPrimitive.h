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

#ifndef IECOREGL_CURVESPRIMITIVE_H
#define IECOREGL_CURVESPRIMITIVE_H

#include "IECoreGL/Primitive.h"
#include "IECoreGL/TypedStateComponent.h"

#include "IECore/VectorTypedData.h"
#include "IECore/CubicBasis.h"

namespace IECoreGL
{

class CurvesPrimitive : public Primitive
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::CurvesPrimitive, CurvesPrimitiveTypeId, Primitive );

		CurvesPrimitive( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr vertsPerCurve,
			IECore::ConstV3fVectorDataPtr points, float width=1.0f );
		virtual ~CurvesPrimitive();

		virtual Imath::Box3f bound() const;
		virtual size_t vertexAttributeSize() const;

		//! @name StateComponents
		/// The following StateComponent classes have an effect only on
		/// CurvesPrimitive objects.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		///
		/// Specifies that all curves should be rendered as linear regardless of their
		/// basis matrix.
		typedef TypedStateComponent<bool, CurvesPrimitiveIgnoreBasisTypeId> IgnoreBasis;
		IE_CORE_DECLAREPTR( IgnoreBasis );
		/// Specifies whether or not GL_LINE primitives should be used instead of
		/// polygons to represent curves.
		typedef TypedStateComponent<bool, CurvesPrimitiveUseGLLinesTypeId> UseGLLines;
		IE_CORE_DECLAREPTR( UseGLLines );
		/// Specifies the line width (in pixels) used whenever CurvesPrimitive objects
		/// are rendered using the GL_LINE primitives.
		typedef TypedStateComponent<float, CurvesPrimitiveGLLineWidthTypeId> GLLineWidth;
		IE_CORE_DECLAREPTR( GLLineWidth );
		//@}

	protected :

		virtual void render( ConstStatePtr state, IECore::TypeId style ) const;

	private :

		void renderLines( ConstStatePtr state, IECore::TypeId style ) const;
		void renderRibbons( ConstStatePtr state, IECore::TypeId style ) const;

		Imath::Box3f m_bound;
		IECore::CubicBasisf m_basis;
		bool m_periodic;
		IECore::IntVectorDataPtr m_vertsPerCurve;
		IECore::V3fVectorDataPtr m_points;
		float m_width;

};

IE_CORE_DECLAREPTR( CurvesPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_CURVESPRIMITIVE_H
