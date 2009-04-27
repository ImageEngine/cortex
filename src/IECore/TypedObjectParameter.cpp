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

#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"

#include "IECore/Renderable.h"
#include "IECore/StateRenderable.h"
#include "IECore/AttributeState.h"
#include "IECore/Shader.h"
#include "IECore/Transform.h"
#include "IECore/MatrixMotionTransform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/Group.h"
#include "IECore/MotionPrimitive.h"
#include "IECore/Primitive.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectVector.h"

using namespace IECore;
using namespace std;

template<typename T>
TypedObjectParameter<T>::TypedObjectParameter( const std::string &name, const std::string &description, typename T::Ptr defaultValue, const ObjectPresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	: ObjectParameter(  name, description, defaultValue, T::staticTypeId(), makePresets( presets) , presetsOnly, userData )
{
}

template<typename T>
Parameter::PresetsContainer TypedObjectParameter<T>::makePresets( const ObjectPresetsContainer &presets )
{
	Parameter::PresetsContainer result; result.reserve( presets.size() );
	
	result.insert( result.end(), presets.begin(), presets.end() );
	
	return result;
}

template <class T> 
const RunTimeTyped::TypeDescription<TypedObjectParameter<T> > TypedObjectParameter<T>::g_typeDescription;

template <typename T> 
TypeId TypedObjectParameter<T>::typeId() const
{
	return staticTypeId();
}

template <typename T> 
TypeId TypedObjectParameter<T>::staticTypeId()
{
	assert( 0 ); // this function must be specialised for each type!
	return InvalidTypeId;
}

template <typename T> 
const char *TypedObjectParameter<T>::typeName() const
{
	return staticTypeName();
}

template <typename T> 
const char *TypedObjectParameter<T>::staticTypeName()
{
	assert( 0 ); // this function must be specialised for each type!
	return "";
}

template <class T> 
TypeId TypedObjectParameter<T>::baseTypeId()
{
	return ObjectParameter::staticTypeId();
}

template <class T> 
const char *TypedObjectParameter<T>::baseTypeName()
{
	return ObjectParameter::staticTypeName();
}

template<typename T>
bool TypedObjectParameter<T>::isInstanceOf( TypeId typeId ) const
{
	if( typeId==staticTypeId() )
	{
		return true;
	}
	return ObjectParameter::isInstanceOf( typeId );
}

template<typename T>
bool TypedObjectParameter<T>::isInstanceOf( const char *typeName ) const
{
	if( !strcmp( typeName, staticTypeName() ) )
	{
		return true;
	}
	return ObjectParameter::isInstanceOf( typeName );
}

template<typename T>
bool TypedObjectParameter<T>::inheritsFrom( TypeId typeId )
{
	return ObjectParameter::staticTypeId()==typeId ? true : ObjectParameter::inheritsFrom( typeId );
}

template<typename T>
bool TypedObjectParameter<T>::inheritsFrom( const char *typeName )
{
	return !strcmp( ObjectParameter::staticTypeName(), typeName ) ? true : ObjectParameter::inheritsFrom( typeName );
}


template<typename T>
bool TypedObjectParameter<T>::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if( !ObjectParameter::valueValid( value, reason ) )
	{
		return false;
	}
	ConstObjectTypePtr tValue = runTimeCast<const ObjectType>( value );
	if( !tValue )
	{
		if( reason )
		{
			*reason = string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"";
		}
		return false;
	}
	return true;
}

#define IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( T, TNAME )\
	\
	template<>\
	TypeId TypedObjectParameter<T>::staticTypeId()\
	{\
		return TNAME ## TypeId;\
	}\
	\
	template<>\
	const char *TypedObjectParameter<T>::staticTypeName()\
	{\
		return # TNAME;\
	}\
	template class TypedObjectParameter<T>;
	
namespace IECore
{
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Renderable, RenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( StateRenderable, StateRenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( AttributeState, AttributeStateParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Shader, ShaderParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Transform, TransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MatrixMotionTransform, MatrixMotionTransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MatrixTransform, MatrixTransformParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( VisibleRenderable, VisibleRenderableParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Group, GroupParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MotionPrimitive, MotionPrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( Primitive, PrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( ImagePrimitive, ImagePrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( MeshPrimitive, MeshPrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( CurvesPrimitive, CurvesPrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( PointsPrimitive, PointsPrimitiveParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( CompoundObject, CompoundObjectParameter );
IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( ObjectVector, ObjectVectorParameter );
}	
