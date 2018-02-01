//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#ifndef IECOREGL_TYPEDSTATECOMPONENT_H
#define IECOREGL_TYPEDSTATECOMPONENT_H

#include "IECoreGL/Export.h"
#include "IECoreGL/StateComponent.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreGL
{

template<typename T, unsigned int TId>
class TypedStateComponent : public StateComponent
{
	public :

		typedef T ValueType;

		typedef boost::intrusive_ptr<TypedStateComponent> Ptr;
		typedef boost::intrusive_ptr<const TypedStateComponent> ConstPtr;

		TypedStateComponent();
		TypedStateComponent( const T &value );

		//! @name RunTimeTyped interface
		////////////////////////////////////////////////////
		//@{
		IECore::TypeId typeId() const override;
		const char *typeName() const override;
		bool isInstanceOf( IECore::TypeId typeId ) const override;
		bool isInstanceOf( const char *typeName ) const override;
		static IECore::TypeId staticTypeId();
		static const char *staticTypeName();
		static bool inheritsFrom( IECore::TypeId typeId );
		static bool inheritsFrom( const char *typeName );
		static IECore::TypeId baseTypeId();
		static const char *baseTypeName();
		typedef StateComponent BaseClass;
		//@}

		//! @name Bindable interface
		////////////////////////////////////////////////////
		//@{
		void bind() const override;
		//@}

		const T &value() const;

		static T defaultValue();

	private :

		T m_value;

		static Description<TypedStateComponent<T, TId> > g_description;

};

/// Use this macro to specialise the necessary parts of a TypedStateComponent instantiation.
#define IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( TYPE, BASETYPE, DEFAULTVALUE )				\
	template<>																				\
	IECOREGL_API const char *TYPE::typeName() const														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	IECOREGL_API const char *TYPE::staticTypeName()														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	IECOREGL_API BASETYPE TYPE::defaultValue()															\
	{																						\
		return DEFAULTVALUE;																\
	}

#define IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TYPE, TYPEID, BASETYPE, DEFAULTVALUE )	\
	IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( TYPE, BASETYPE, DEFAULTVALUE )								\
	template class TypedStateComponent<BASETYPE, TYPEID>;										\

typedef TypedStateComponent<Imath::Color4f, ColorTypeId> Color;
template<>
IECOREGL_API void Color::bind() const;

/// \todo Consider also moving the following state components to Primitive header since the renderer maps then to "gl:primitive:*"
/// Used to signify that the shading for a primitive may produce transparent values. The Renderer
/// maps the "gl:shade:transparent" attribute directly to this state. Note that this information
/// is provided as a separate state item rather than as a query on the Shader class as the values
/// of variables on Primitives may change the transparency of a shader.
typedef TypedStateComponent<bool, TransparentShadingStateComponentTypeId> TransparentShadingStateComponent;
/// Specifies the color to draw bounding boxes in
typedef TypedStateComponent<Imath::Color4f, BoundColorStateComponentTypeId> BoundColorStateComponent;
/// Specifies the color to draw wireframes in
typedef TypedStateComponent<Imath::Color4f, WireframeColorStateComponentTypeId> WireframeColorStateComponent;
/// Specifies the color to draw outlines in
typedef TypedStateComponent<Imath::Color4f, OutlineColorStateComponentTypeId> OutlineColorStateComponent;
/// Specifies the color to draw points in
typedef TypedStateComponent<Imath::Color4f, PointColorStateComponentTypeId> PointColorStateComponent;

enum GLPointsUsage
{
	ForPointsOnly,
	ForPointsAndDisks,
	ForAll
};

struct IECOREGL_API BlendFactors
{
	BlendFactors( GLenum src, GLenum dst );
	BlendFactors( const BlendFactors &other );
	GLenum src;
	GLenum dst;
};

struct IECOREGL_API AlphaFunc
{
	AlphaFunc( GLenum m, GLfloat value );
	AlphaFunc( const AlphaFunc &other );
	GLenum mode;
	GLfloat value;
};

typedef TypedStateComponent<BlendFactors, BlendFuncStateComponentTypeId> BlendFuncStateComponent;
template<>
IECOREGL_API void BlendFuncStateComponent::bind() const;

typedef TypedStateComponent<Imath::Color4f, BlendColorStateComponentTypeId> BlendColorStateComponent;
template<>
IECOREGL_API void BlendColorStateComponent::bind() const;

typedef TypedStateComponent<GLenum, BlendEquationStateComponentTypeId> BlendEquationStateComponent;
template<>
IECOREGL_API void BlendEquationStateComponent::bind() const;

typedef TypedStateComponent<bool, AlphaTestStateComponentTypeId> AlphaTestStateComponent;
template<>
IECOREGL_API void AlphaTestStateComponent::bind() const;

typedef TypedStateComponent<AlphaFunc, AlphaFuncStateComponentTypeId> AlphaFuncStateComponent;
template<>
IECOREGL_API void AlphaFuncStateComponent::bind() const;

/// Used to specify enable state of GL_CULL_FACE
typedef TypedStateComponent<bool, DoubleSidedStateComponentTypeId> DoubleSidedStateComponent;
template<>
IECOREGL_API void DoubleSidedStateComponent::bind() const;

/// Used to implement the "rightHandedOrientation" Renderer attribute. Implemented by calling
/// glFrontFace( GL_CCW ) when true and glFrontFace( GL_CW ) when false.
typedef TypedStateComponent<bool, RightHandedOrientationStateComponentTypeId> RightHandedOrientationStateComponent;
template<>
IECOREGL_API void RightHandedOrientationStateComponent::bind() const;

/// Used to specify enable state of GL_LINE_SMOOTH
typedef TypedStateComponent<bool, LineSmoothingStateComponentTypeId> LineSmoothingStateComponent;
template<>
IECOREGL_API void LineSmoothingStateComponent::bind() const;

/// Used to specify enable state of GL_POINT_SMOOTH
typedef TypedStateComponent<bool, PointSmoothingStateComponentTypeId> PointSmoothingStateComponent;
template<>
IECOREGL_API void PointSmoothingStateComponent::bind() const;

/// Used to specify enable state of GL_POLYGON_SMOOTH
typedef TypedStateComponent<bool, PolygonSmoothingStateComponentTypeId> PolygonSmoothingStateComponent;
template<>
IECOREGL_API void PolygonSmoothingStateComponent::bind() const;

/// Used to specify enable state of GL_DEPTH_TEST
typedef TypedStateComponent<bool, DepthTestStateComponentTypeId> DepthTestStateComponent;
template<>
IECOREGL_API void DepthTestStateComponent::bind() const;

/// Used to set glDepthMask to GL_TRUE or GL_FALSE
typedef TypedStateComponent<bool, DepthMaskStateComponentTypeId> DepthMaskStateComponent;
template<>
IECOREGL_API void DepthMaskStateComponent::bind() const;

// \todo: implement CurrentSpace, ShaderSpace, CameraSpace, ScreenSpace, RasterSpace and NDCSpace like in Renderman interface.
enum RendererSpace
{
	ObjectSpace,
	WorldSpace
};
// Used to specify which space the culling bounding box is defined. Culling is applied to primitives and procedurals.
typedef TypedStateComponent<RendererSpace, CullingSpaceStateComponentTypeId> CullingSpaceStateComponent;

// Defines the bounding box for culling. The space it is defined on is given by CullingSpaceStateComponent.
typedef TypedStateComponent<Imath::Box3f, CullingBoxStateComponentTypeId> CullingBoxStateComponent;

/// Defines whether or not procedurals will be executed in parallel threads.
typedef TypedStateComponent<bool, ProceduralThreadingStateComponentTypeId> ProceduralThreadingStateComponent;

/// Defines camera visibility.
typedef TypedStateComponent<bool, CameraVisibilityStateComponentTypeId> CameraVisibilityStateComponent;

/// Defines whether or not the renderer will create instances automatically when identical
/// primitives are encountered.
typedef TypedStateComponent<bool, AutomaticInstancingStateComponentTypeId> AutomaticInstancingStateComponent;

IE_CORE_DECLAREPTR( Color );
IE_CORE_DECLAREPTR( BlendColorStateComponent );
IE_CORE_DECLAREPTR( BlendFuncStateComponent );
IE_CORE_DECLAREPTR( BlendEquationStateComponent );
IE_CORE_DECLAREPTR( BoundColorStateComponent );
IE_CORE_DECLAREPTR( WireframeColorStateComponent );
IE_CORE_DECLAREPTR( OutlineColorStateComponent );
IE_CORE_DECLAREPTR( PointColorStateComponent );
IE_CORE_DECLAREPTR( DoubleSidedStateComponent );
IE_CORE_DECLAREPTR( LineSmoothingStateComponent );
IE_CORE_DECLAREPTR( PointSmoothingStateComponent );
IE_CORE_DECLAREPTR( DepthTestStateComponent );
IE_CORE_DECLAREPTR( DepthMaskStateComponent );
IE_CORE_DECLAREPTR( CullingSpaceStateComponent );
IE_CORE_DECLAREPTR( CullingBoxStateComponent );
IE_CORE_DECLAREPTR( ProceduralThreadingStateComponent );
IE_CORE_DECLAREPTR( CameraVisibilityStateComponent );
IE_CORE_DECLAREPTR( AutomaticInstancingStateComponent );

} // namespace IECoreGL

#include "IECoreGL/TypedStateComponent.inl"

#endif // IECOREGL_TYPEDSTATECOMPONENT_H
