//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/StateComponent.h"

#include "OpenEXR/ImathColor.h"

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
		virtual IECore::TypeId typeId() const;
		virtual const char *typeName() const;
		virtual bool isInstanceOf( IECore::TypeId typeId ) const;
		virtual bool isInstanceOf( const char *typeName ) const;
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
		virtual void bind() const;
		virtual GLbitfield mask() const;
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
	const char *TYPE::typeName() const														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	const char *TYPE::staticTypeName()														\
	{																						\
		return # TYPE;																		\
	}																						\
																							\
	template<>																				\
	BASETYPE TYPE::defaultValue()															\
	{																						\
		return DEFAULTVALUE;																\
	}

#define IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( TYPE, TYPEID, BASETYPE, DEFAULTVALUE )	\
	IECOREGL_TYPEDSTATECOMPONENT_SPECIALISE( TYPE, BASETYPE, DEFAULTVALUE )								\
	template class TypedStateComponent<BASETYPE, TYPEID>;										\

/// \todo Now there are loads of these typedefs I think they should really be defined in the places
/// they're used - so PrimitiveBound would be a typedef member in Primitive etc. This would help people
/// see where StateComponents have effect, and also provide a better location for documenting them. Otherwise
/// this file has a massive influence over all sorts of disparate bits of the library.
/// The CurvesPrimitive specific StateComponents have already been implemented this way so use that for reference.
typedef TypedStateComponent<Imath::Color4f, ColorTypeId> Color;
template<>
void Color::bind() const;
template<>
GLbitfield Color::mask() const;

typedef TypedStateComponent<bool, PrimitiveBoundTypeId> PrimitiveBound;
typedef TypedStateComponent<bool, PrimitiveWireframeTypeId> PrimitiveWireframe;
typedef TypedStateComponent<float, PrimitiveWireframeWidthTypeId> PrimitiveWireframeWidth;
typedef TypedStateComponent<bool, PrimitiveSolidTypeId> PrimitiveSolid;
typedef TypedStateComponent<bool, PrimitiveOutlineTypeId> PrimitiveOutline;
typedef TypedStateComponent<float, PrimitiveOutlineWidthTypeId> PrimitiveOutlineWidth;
typedef TypedStateComponent<bool, PrimitivePointsTypeId> PrimitivePoints;
typedef TypedStateComponent<float, PrimitivePointWidthTypeId> PrimitivePointWidth;
/// Used to signify that the shading for a primitive may produce transparent values. The Renderer
/// maps the "gl:shade:transparent" attribute directly to this state. Note that this information
/// is provided as a separate state item rather than as a query on the Shader class as the values
/// of variables on Primitives may change the the transparency of a shader.
typedef TypedStateComponent<bool, TransparentShadingStateComponentTypeId> TransparentShadingStateComponent;
/// Used to trigger sorting of the components of a primitive when the TransparentShadingStateComponent has
/// a value of true.
typedef TypedStateComponent<bool, PrimitiveTransparencySortStateComponentTypeId> PrimitiveTransparencySortStateComponent;
/// Specifies the color to draw bounding boxes in
typedef TypedStateComponent<Imath::Color4f, BoundColorStateComponentTypeId> BoundColorStateComponent;
/// Specifies the color to draw wireframes in
typedef TypedStateComponent<Imath::Color4f, WireframeColorStateComponentTypeId> WireframeColorStateComponent;
/// Specifies the color to draw outlines in
typedef TypedStateComponent<Imath::Color4f, OutlineColorStateComponentTypeId> OutlineColorStateComponent;
/// Specifies the color to draw points in
typedef TypedStateComponent<Imath::Color4f, PointColorStateComponentTypeId> PointColorStateComponent;

enum UseGLPoints
{
	ForPointsOnly,
	ForPointsAndDisks,
	ForAll
};
/// Specifies an override for rendering PointsPrimitives with gl points.
typedef TypedStateComponent<UseGLPoints, PointsPrimitiveUseGLPointsTypeId> PointsPrimitiveUseGLPoints;
/// Specifies an attribute for defining the glPointSize of PointsPrimitives rendered as gl points.
typedef TypedStateComponent<float, PointsPrimitiveGLPointWidthTypeId> PointsPrimitiveGLPointWidth;

struct BlendFactors
{
	BlendFactors( GLenum src, GLenum dst );
	BlendFactors( const BlendFactors &other );
	GLenum src;
	GLenum dst;
};

typedef TypedStateComponent<BlendFactors, BlendFuncStateComponentTypeId> BlendFuncStateComponent;
template<>
void BlendFuncStateComponent::bind() const;
template<>
GLbitfield BlendFuncStateComponent::mask() const;

typedef TypedStateComponent<Imath::Color4f, BlendColorStateComponentTypeId> BlendColorStateComponent;
template<>
void BlendColorStateComponent::bind() const;
template<>
GLbitfield BlendColorStateComponent::mask() const;

typedef TypedStateComponent<GLenum, BlendEquationStateComponentTypeId> BlendEquationStateComponent;
template<>
void BlendEquationStateComponent::bind() const;
template<>
GLbitfield BlendEquationStateComponent::mask() const;

/// Used to specify enable state of GL_CULL_FACE
typedef TypedStateComponent<bool, DoubleSidedStateComponentTypeId> DoubleSidedStateComponent;
template<>
void DoubleSidedStateComponent::bind() const;
template<>
GLbitfield DoubleSidedStateComponent::mask() const;

/// Used to implement the "rightHandedOrientation" Renderer attribute. Implemented by calling
/// glFrontFace( GL_CCW ) when true and glFrontFace( GL_CW ) when false.
typedef TypedStateComponent<bool, RightHandedOrientationStateComponentTypeId> RightHandedOrientationStateComponent;
template<>
void RightHandedOrientationStateComponent::bind() const;
template<>
GLbitfield RightHandedOrientationStateComponent::mask() const;

/// Used to specify enable state of GL_LINE_SMOOTH
typedef TypedStateComponent<bool, LineSmoothingStateComponentTypeId> LineSmoothingStateComponent;
template<>
void LineSmoothingStateComponent::bind() const;
template<>
GLbitfield LineSmoothingStateComponent::mask() const;

/// Used to specify enable state of GL_POINT_SMOOTH
typedef TypedStateComponent<bool, PointSmoothingStateComponentTypeId> PointSmoothingStateComponent;
template<>
void PointSmoothingStateComponent::bind() const;
template<>
GLbitfield PointSmoothingStateComponent::mask() const;

/// Used to specify enable state of GL_POLYGON_SMOOTH
typedef TypedStateComponent<bool, PolygonSmoothingStateComponentTypeId> PolygonSmoothingStateComponent;
template<>
void PolygonSmoothingStateComponent::bind() const;
template<>
GLbitfield PolygonSmoothingStateComponent::mask() const;

IE_CORE_DECLAREPTR( Color );
IE_CORE_DECLAREPTR( PrimitiveBound );
IE_CORE_DECLAREPTR( PrimitiveWireframe );
IE_CORE_DECLAREPTR( PrimitiveWireframeWidth );
IE_CORE_DECLAREPTR( PrimitiveSolid );
IE_CORE_DECLAREPTR( PrimitiveOutline );
IE_CORE_DECLAREPTR( PrimitiveOutlineWidth );
IE_CORE_DECLAREPTR( PrimitivePoints );
IE_CORE_DECLAREPTR( PrimitivePointWidth );
IE_CORE_DECLAREPTR( BlendColorStateComponent );
IE_CORE_DECLAREPTR( BlendFuncStateComponent );
IE_CORE_DECLAREPTR( BlendEquationStateComponent );
IE_CORE_DECLAREPTR( BoundColorStateComponent );
IE_CORE_DECLAREPTR( WireframeColorStateComponent );
IE_CORE_DECLAREPTR( OutlineColorStateComponent );
IE_CORE_DECLAREPTR( PointColorStateComponent );
IE_CORE_DECLAREPTR( PointsPrimitiveUseGLPoints );
IE_CORE_DECLAREPTR( PointsPrimitiveGLPointWidth );
IE_CORE_DECLAREPTR( DoubleSidedStateComponent );
IE_CORE_DECLAREPTR( LineSmoothingStateComponent );
IE_CORE_DECLAREPTR( PointSmoothingStateComponent );

} // namespace IECoreGL

#include "IECoreGL/TypedStateComponent.inl"

#endif // IECOREGL_TYPEDSTATECOMPONENT_H
