//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

IE_CORE_FORWARDDECLARE( Renderable )
IE_CORE_FORWARDDECLARE( StateRenderable )
IE_CORE_FORWARDDECLARE( AttributeState )
IE_CORE_FORWARDDECLARE( Shader )
IE_CORE_FORWARDDECLARE( Transform )
IE_CORE_FORWARDDECLARE( MatrixMotionTransform )
IE_CORE_FORWARDDECLARE( MatrixTransform )
IE_CORE_FORWARDDECLARE( VisibleRenderable )
IE_CORE_FORWARDDECLARE( Group )
IE_CORE_FORWARDDECLARE( ObjectVector )
IE_CORE_FORWARDDECLARE( SmoothSkinningData )

/// The TypedObjectParameter class implements an ObjectParameter which rigidly only
/// accepts one type of Object
template<typename T>
class TypedObjectParameter : public ObjectParameter
{
	public :

		typedef T ObjectType;
		typedef typename T::Ptr ObjectTypePtr;
		typedef typename T::ConstPtr ConstObjectTypePtr;
		typedef std::pair<std::string, ObjectTypePtr> ObjectPreset;
		typedef std::vector<ObjectPreset> ObjectPresetsContainer;

		TypedObjectParameter( const std::string &name, const std::string &description, typename T::Ptr defaultValue, const ObjectPresetsContainer &presets = ObjectPresetsContainer(), bool presetsOnly = false,ConstCompoundObjectPtr userData=nullptr );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( TypedObjectParameter<T>, ObjectParameter );

		/// Implemented to return true only if value is of type T.
		bool valueValid( const Object *value, std::string *reason = nullptr ) const override;

	protected:

		static PresetsContainer makePresets( const ObjectPresetsContainer &presets );

	private :

		static TypeDescription<TypedObjectParameter<T> > g_typeDescription;

};

typedef TypedObjectParameter<Renderable> RenderableParameter;
typedef TypedObjectParameter<StateRenderable> StateRenderableParameter;
typedef TypedObjectParameter<AttributeState> AttributeStateParameter;
typedef TypedObjectParameter<Shader> ShaderParameter;
typedef TypedObjectParameter<Transform> TransformParameter;
typedef TypedObjectParameter<MatrixMotionTransform> MatrixMotionTransformParameter;
typedef TypedObjectParameter<MatrixTransform> MatrixTransformParameter;
typedef TypedObjectParameter<VisibleRenderable> VisibleRenderableParameter;
typedef TypedObjectParameter<Group> GroupParameter;
typedef TypedObjectParameter<CompoundObject> CompoundObjectParameter;
typedef TypedObjectParameter<ObjectVector> ObjectVectorParameter;
typedef TypedObjectParameter<SmoothSkinningData> SmoothSkinningDataParameter;

IE_CORE_DECLAREPTR( RenderableParameter );
IE_CORE_DECLAREPTR( StateRenderableParameter );
IE_CORE_DECLAREPTR( AttributeStateParameter );
IE_CORE_DECLAREPTR( ShaderParameter );
IE_CORE_DECLAREPTR( TransformParameter );
IE_CORE_DECLAREPTR( MatrixMotionTransformParameter );
IE_CORE_DECLAREPTR( MatrixTransformParameter );
IE_CORE_DECLAREPTR( VisibleRenderableParameter );
IE_CORE_DECLAREPTR( GroupParameter );
IE_CORE_DECLAREPTR( CompoundObjectParameter );
IE_CORE_DECLAREPTR( ObjectVectorParameter );
IE_CORE_DECLAREPTR( SmoothSkinningDataParameter );

} // namespace IECore

#endif // IE_CORE_TYPEDOBJECTPARAMETER_H
