//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_PRIMITIVE_H
#define IECOREGL_PRIMITIVE_H

#include "OpenEXR/ImathBox.h"

#include "IECore/PrimitiveVariable.h"
#include "IECore/VectorTypedData.h"

#include "IECoreGL/GL.h"
#include "IECoreGL/Renderable.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/Shader.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );

/// The Primitive class represents geometric objects that can
/// be rendered in OpenGL. Primitives may be rendered in a variety
/// of styles defined by State objects, or just rendered as raw geometry
/// in the current OpenGL state.
class Primitive : public Renderable
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Primitive, PrimitiveTypeId, Renderable );

		Primitive();
		virtual ~Primitive();

		/// Renders the Primitive in the current
		/// OpenGL context. The Primitive will draw itself
		/// using the style represented by state, allowing
		/// representations such as wireframe over shaded etc.
		/// It temporarily changes the shader parameters that 
		/// match the primitive's uniform variables.
		/// An exception is thrown if state->isComplete() is not
		/// true.
		///
		/// The default implementation for this function calls
		/// the protected render() method several times
		/// in different OpenGL states, once for each style
		/// present in state.
		virtual void render( State *currentState ) const;

		/// Adds a primitive variable on this primitive.
		/// Derived classes should implement any customized filtering and/or conversions or call the base class implementation.
		/// Default implementation sets Constant variables as uniform shader parameters and all the
		/// others as vertex shader parameters.
		/// \todo MAYBE JUST HAVE ADDVERTEXATTRIBUTE AND ADDUNIFORMATTRIBUTE AND LET THE CONVERTERS DO IT?
		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar ) = 0;

		virtual Imath::Box3f bound() const = 0;

		//! @name StateComponents
		/// The following StateComponent classes have an effect only on
		/// Primitive objects.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		typedef TypedStateComponent<bool, PrimitiveBoundTypeId> DrawBound;
		IE_CORE_DECLAREPTR( DrawBound );
		typedef TypedStateComponent<bool, PrimitiveWireframeTypeId> DrawWireframe;
		IE_CORE_DECLAREPTR( DrawWireframe );
		typedef TypedStateComponent<float, PrimitiveWireframeWidthTypeId> WireframeWidth;
		IE_CORE_DECLAREPTR( WireframeWidth );
		typedef TypedStateComponent<bool, PrimitiveSolidTypeId> DrawSolid;
		IE_CORE_DECLAREPTR( DrawSolid );
		typedef TypedStateComponent<bool, PrimitiveOutlineTypeId> DrawOutline;
		IE_CORE_DECLAREPTR( DrawOutline );
		typedef TypedStateComponent<float, PrimitiveOutlineWidthTypeId> OutlineWidth;
		IE_CORE_DECLAREPTR( OutlineWidth );
		typedef TypedStateComponent<bool, PrimitivePointsTypeId> DrawPoints;
		IE_CORE_DECLAREPTR( DrawPoints );
		typedef TypedStateComponent<float, PrimitivePointWidthTypeId> PointWidth;
		IE_CORE_DECLAREPTR( PointWidth );
		/// Used to trigger sorting of the components of a primitive when the TransparentShadingStateComponent has a value of true.
		typedef TypedStateComponent<bool, PrimitiveTransparencySortStateComponentTypeId> TransparencySort;
		IE_CORE_DECLAREPTR( TransparencySort );
		//@}

	protected :

		/// Must be implemented by subclasses. This function is called several
		/// times by the standard render() call, once for each style of rendering
		/// requested in state (wireframe, solid etc). The TypeId of the StateComponent
		/// representing that style is passed so that the drawing can be optimised
		/// for the particular style (e.g. PrimitiveWireframeTypeId is passed for
		/// wireframe rendering).
		virtual void render( const State *state, IECore::TypeId style ) const = 0;

		/// Called by derived classes to register a vertex attribute. There are no type or length checks on this call.
		void addVertexAttribute( const std::string &name, IECore::ConstDataPtr data );

		/// Called by derived classes to register a uniform attribute. There are no type or length checks on this call.
		void addUniformAttribute( const std::string &name, IECore::ConstDataPtr data );

		/// Convenience function for use in render() implementations. Returns
		/// true if TransparentShadingStateComponent is true and
		/// PrimitiveTransparencySortStateComponent is true.
		bool depthSortRequested( const State *state ) const;

	private :

		typedef std::vector<Shader::SetupPtr> ShaderSetupVector;
		mutable ShaderSetupVector m_shaderSetups;
		const Shader::Setup *shaderSetup( Shader *shader ) const;
		
		typedef std::map<std::string, IECore::ConstDataPtr> AttributeMap;
		AttributeMap m_vertexAttributes;
		AttributeMap m_uniformAttributes;

};

IE_CORE_DECLAREPTR( Primitive );

} // namespace IECoreGL

#endif // IECOREGL_PRIMITIVE_H
