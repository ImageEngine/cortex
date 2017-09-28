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

#ifndef IECOREGL_CURVESPRIMITIVE_H
#define IECOREGL_CURVESPRIMITIVE_H

#include "IECore/VectorTypedData.h"
#include "IECore/CubicBasis.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/Primitive.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Buffer )

class IECOREGL_API CurvesPrimitive : public Primitive
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::CurvesPrimitive, CurvesPrimitiveTypeId, Primitive );

		CurvesPrimitive( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr vertsPerCurve, float width=1.0f );
		virtual ~CurvesPrimitive();

		virtual Imath::Box3f bound() const;
		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar );
		virtual const Shader::Setup *shaderSetup( const Shader *shader, State *state ) const;
		virtual void render( const State *currentState, IECore::TypeId style ) const;
		/// Just renders each segment as linear with GL_LINES.
		virtual void renderInstances( size_t numInstances = 1 ) const;

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

	private :

		void renderMode( const State *state, bool &linear, bool &ribbons ) const;

		static const std::string &cubicLinesGeometrySource();
		static const std::string &cubicRibbonsGeometrySource();
		static const std::string &linearRibbonsGeometrySource();

		void ensureVertIds() const;
		void ensureAdjacencyVertIds() const;
		void ensureLinearAdjacencyVertIds() const;

		IE_CORE_FORWARDDECLARE( MemberData );
		MemberDataPtr m_memberData;

};

IE_CORE_DECLAREPTR( CurvesPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_CURVESPRIMITIVE_H
