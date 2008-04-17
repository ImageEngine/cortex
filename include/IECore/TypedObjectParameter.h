//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TYPEDOBJECTPARAMETER_H
#define IE_CORE_TYPEDOBJECTPARAMETER_H

#include <string>

#include "IECore/ObjectParameter.h"



namespace IECore
{

class Renderable;
class StateRenderable;
class AttributeState;
class Shader;
class Transform;
class MatrixMotionTransform;
class MatrixTransform;
class VisibleRenderable;
class Group;
class MotionPrimitive;
class Primitive;
class ImagePrimitive;
class MeshPrimitive;
class PointsPrimitive;

/// The TypedObjectParameter class implements an ObjectParameter which rigidly only
/// accepts one type of Object
template<typename T>
class TypedObjectParameter : public ObjectParameter
{
	public :
		
		IE_CORE_DECLAREMEMBERPTR( TypedObjectParameter<T> );
		typedef T ObjectType;
		typedef typename T::Ptr ObjectTypePtr;
		typedef typename T::ConstPtr ConstObjectTypePtr;
		typedef std::map<std::string, ObjectTypePtr> ObjectPresetsMap;
		
		TypedObjectParameter( const std::string &name, const std::string &description, typename T::Ptr defaultValue, const ObjectPresetsMap &presets = ObjectPresetsMap(), bool presetsOnly = false,ConstCompoundObjectPtr userData=0 );		
		
		//! @name RunTimeTyped functions
		////////////////////////////////////
		//@{
		virtual TypeId typeId() const;
		virtual std::string typeName() const;
		virtual bool isInstanceOf( TypeId typeId ) const;
		virtual bool isInstanceOf( const std::string &typeName ) const;
		static TypeId staticTypeId();
		static std::string staticTypeName();
		static bool inheritsFrom( TypeId typeId );
		static bool inheritsFrom( const std::string &typeName );
		//@}
		
		/// Implemented to return true only if value is of type T.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;
		
	protected:
	
		static PresetsMap makePresets( const ObjectPresetsMap &presets );

};

typedef TypedObjectParameter<MeshPrimitive> MeshPrimitiveParameter;
typedef TypedObjectParameter<Renderable> RenderableParameter;
typedef TypedObjectParameter<StateRenderable> StateRenderableParameter;
typedef TypedObjectParameter<AttributeState> AttributeStateParameter;
typedef TypedObjectParameter<Shader> ShaderParameter;
typedef TypedObjectParameter<Transform> TransformParameter;
typedef TypedObjectParameter<MatrixMotionTransform> MatrixMotionTransformParameter;
typedef TypedObjectParameter<MatrixTransform> MatrixTransformParameter;
typedef TypedObjectParameter<VisibleRenderable> VisibleRenderableParameter;
typedef TypedObjectParameter<Group> GroupParameter;
typedef TypedObjectParameter<MotionPrimitive> MotionPrimitiveParameter;
typedef TypedObjectParameter<Primitive> PrimitiveParameter;
typedef TypedObjectParameter<ImagePrimitive> ImagePrimitiveParameter;
typedef TypedObjectParameter<MeshPrimitive> MeshPrimitiveParameter;
typedef TypedObjectParameter<PointsPrimitive> PointsPrimitiveParameter;


IE_CORE_DECLAREPTR( MeshPrimitiveParameter );
IE_CORE_DECLAREPTR( RenderableParameter );
IE_CORE_DECLAREPTR( StateRenderableParameter );
IE_CORE_DECLAREPTR( AttributeStateParameter );
IE_CORE_DECLAREPTR( ShaderParameter );
IE_CORE_DECLAREPTR( TransformParameter );
IE_CORE_DECLAREPTR( MatrixMotionTransformParameter );
IE_CORE_DECLAREPTR( MatrixTransformParameter );
IE_CORE_DECLAREPTR( VisibleRenderableParameter );
IE_CORE_DECLAREPTR( GroupParameter );
IE_CORE_DECLAREPTR( MotionPrimitiveParameter );
IE_CORE_DECLAREPTR( PrimitiveParameter );
IE_CORE_DECLAREPTR( ImagePrimitiveParameter );
IE_CORE_DECLAREPTR( MeshPrimitiveParameter );
IE_CORE_DECLAREPTR( PointsPrimitiveParameter );

} // namespace IECore

#endif // IE_CORE_TYPEDOBJECTPARAMETER_H
