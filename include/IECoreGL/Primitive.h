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

#include "IECore/VectorTypedData.h"
#include "IECoreScene/PrimitiveVariable.h"

#include "IECoreGL/Export.h"
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
class IECOREGL_API Primitive : public Renderable
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Primitive, PrimitiveTypeId, Renderable );

		Primitive();
		~Primitive() override;

		/// Adds a primitive variable to this primitive. Derived classes should implement any filtering
		/// or conversions that are necessary and then call addVertexAttribute() or addUniformAttribute().
		/// The default implementation calls addUniformAttribute() for uniform primitive variables and
		/// addVertexAttribute() for all others.
		virtual void addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar ) = 0;

		/// Returns the bounding box for the primitive.
		Imath::Box3f bound() const override = 0;

		/// High level rendering function which renders in the styles represented by
		/// currentState, allowing representations such as wireframe over shaded etc to
		/// be achieved with a single call. The currentState must be complete and
		/// already have been bound. Finer grained control over rendering can be achieved
		/// by using the shaderSetup() and renderInstances() methods - in fact those methods
		/// are used to implement this one.
		void render( State *currentState ) const override;

		//! @name Lower level rendering methods
		/// These methods are used to implement the higher level render() method - they
		/// may also be called directly to implement custom rendering.
		//////////////////////////////////////////////////////////////////////////////
		/// This method returns a Shader::Setup binding the primitive to a shader for
		/// rendering in a particular state. It may be used in conjunction with renderInstances()
		/// to provide finer grained control over rendering. All vertex attributes are
		/// mapped to shader parameters prefixed with "vertex" so for instance "P" will be
		/// mapped to "vertexP". Uniform attributes are mapped directly to shader uniforms with
		/// no prefix. This naming convention corresponds to the inputs defined by the default
		/// source defined in the Shader class, and should be adopted when writing custom shaders.
		///
		/// Most classes will not need to override this method - reasons for overriding would be
		/// to substitute in custom geometry or vertex shaders and/or to bind in attributes
		/// not already specified with addUniformAttribute() or addVertexAttribute().
		///
		/// \todo We need to rethink this mechanism. The problem is that we've ended up using
		/// this method for two things - firstly to get a ShaderSetup where all the primitive
		/// variables are bound (good), and secondly we've abused it to actually change the
		/// shader in PointsPrimitive and CurvePrimitive. Asking for a setup for one shader and getting
		/// back a setup for another doesn't make a great deal of sense. There are several
		/// competing sources of source code for shaders :
		///
		/// - The user-provided source coming through Renderer::shader().
		/// - The vertex and geometry shaders that PointsPrimitive and CurvesPrimitive need
		///   to insert.
		/// - The constant fragment shader that Primitive needs to insert to do wireframe
		///   shading etc.
		/// - The ID fragment shader needed for the Selector.
		///
		/// We should redesign our API so that we first resolve these requirements to generate
		/// a shader, and then use shaderSetup() just to apply primitive variables to it.
		virtual const Shader::Setup *shaderSetup( const Shader *shader, State *state ) const;
		/// Adds the primitive variables held by this Primitive to the specified Shader::Setup.
		/// Vertex attributes will be prefixed as specified, and for each vertex attribute
		/// a boolean uniform parameter called "${prefix}${attributeName}Active" will also be
		/// added so the shader can determine whether or not the values for that input are useful.
		void addPrimitiveVariablesToShaderSetup( Shader::Setup *shaderSetup, const std::string &vertexPrefix = "vertex", GLuint vertexDivisor = 0 ) const;
		/// Renders the primitive using the specified state and with a particular style.
		/// The style is specified using the TypeId of the StateComponent representing that style
		/// (e.g. PrimitiveWireframeTypeId is passed for wireframe rendering).
		///
		/// The default implementation calls renderInstances() but derived classes may override it
		/// to modify their drawing based on the state. A Shader::Setup
		/// created for this primitive must be bound before calling this method.
		virtual void render( const State *currentState, IECore::TypeId style ) const;
		/// Renders a number of instances of the primitive by issuing a single call to
		/// glDrawElementsInstanced() or glDrawArraysInstanced(). A Shader::Setup created for this
		/// primitive must be bound before calling this method.
		virtual void renderInstances( size_t numInstances = 1 ) const = 0;
		///@}

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
		typedef TypedStateComponent<bool, PrimitiveSelectableTypeId> Selectable;
		IE_CORE_DECLAREPTR( Selectable );
		/// Used to trigger sorting of the components of a primitive when the TransparentShadingStateComponent has a value of true.
		typedef TypedStateComponent<bool, PrimitiveTransparencySortStateComponentTypeId> TransparencySort;
		IE_CORE_DECLAREPTR( TransparencySort );
		//@}

	protected :

		/// Called by derived classes to register a uniform attribute. There are no type or length checks on this call.
		void addUniformAttribute( const std::string &name, IECore::ConstDataPtr data );
		/// Called by derived classes to register a vertex attribute. There are no type or length checks on this call.
		void addVertexAttribute( const std::string &name, IECore::ConstDataPtr data );

		/// Convenience function for use in render() implementations. Returns
		/// true if TransparentShadingStateComponent is true and
		/// PrimitiveTransparencySortStateComponent is true.
		bool depthSortRequested( const State *state ) const;

	private :

		typedef std::vector<Shader::SetupPtr> ShaderSetupVector;
		mutable ShaderSetupVector m_shaderSetups;

		mutable Shader::SetupPtr m_boundSetup;
		const Shader::Setup *boundSetup() const;

		typedef std::map<std::string, IECore::ConstDataPtr> AttributeMap;
		AttributeMap m_vertexAttributes;
		AttributeMap m_uniformAttributes;

};

IE_CORE_DECLAREPTR( Primitive );

} // namespace IECoreGL

#endif // IECOREGL_PRIMITIVE_H
