//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TYPEDPARAMETER_H
#define IE_CORE_TYPEDPARAMETER_H

#include "IECore/Parameter.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// A template class for simple typed parameters. TypedData<T> is used to store the value.
template<typename T>
class TypedParameter : public Parameter
{
	public :
	
		typedef T ValueType;
		typedef TypedData<T> ObjectType;
		typedef boost::intrusive_ptr<ObjectType> ObjectTypePtr;
		typedef boost::intrusive_ptr<const ObjectType> ConstObjectTypePtr;
		typedef std::map<std::string, T> PresetsMap;
		typedef std::map<std::string, ObjectTypePtr> ObjectPresetsMap;
	
		/// Constructs a new ObjectType object to hold the default value.
		TypedParameter( const std::string &name, const std::string &description, const T &defaultValue = T(),
			const PresetsMap &presets = PresetsMap(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
		/// Takes a copy of defaultValue for use as the default value.
		TypedParameter( const std::string &name, const std::string &description, ObjectTypePtr defaultValue,
			const ObjectPresetsMap &presets = ObjectPresetsMap(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
		
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
			
		/// Implemented to return true only if value is of type TypedData<T>.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;
		
		/// Convenience function for getting the default value, which avoids all the hoop jumping
		/// needed to extract the value from the Object returned by Parameter::defaultValue().
		const ValueType &typedDefaultValue() const;
		
		/// Convenience function for value getting, which avoids all the hoop jumping
		/// needed to extract the value from the Object returned by Parameter::getValue().
		/// Throws an Exception if the value is not valid.
		const ValueType &getTypedValue() const;
		/// Convenience function for value setting, constructs a TypedData<T> from value
		/// and calls Parameter::setValue().
		void setTypedValue( const T &value );
		
};

typedef TypedParameter<bool> BoolParameter;
typedef TypedParameter<std::string> StringParameter;
typedef TypedParameter<Imath::V2i> V2iParameter;
typedef TypedParameter<Imath::V3i> V3iParameter;
typedef TypedParameter<Imath::V2f> V2fParameter;
typedef TypedParameter<Imath::V3f> V3fParameter;
typedef TypedParameter<Imath::V2d> V2dParameter;
typedef TypedParameter<Imath::V3d> V3dParameter;
typedef TypedParameter<Imath::Color3f> Color3fParameter;
typedef TypedParameter<Imath::Color4f> Color4fParameter;
typedef TypedParameter<Imath::Box2i> Box2iParameter;
typedef TypedParameter<Imath::Box3i> Box3iParameter;
typedef TypedParameter<Imath::Box2f> Box2fParameter;
typedef TypedParameter<Imath::Box3f> Box3fParameter;
typedef TypedParameter<Imath::Box2d> Box2dParameter;
typedef TypedParameter<Imath::Box3d> Box3dParameter;
typedef TypedParameter<Imath::M44f> M44fParameter;
typedef TypedParameter<Imath::M44d> M44dParameter;

typedef TypedParameter<std::vector<int> > IntVectorParameter;
typedef TypedParameter<std::vector<float> > FloatVectorParameter;
typedef TypedParameter<std::vector<double> > DoubleVectorParameter;
typedef TypedParameter<std::vector<std::string> > StringVectorParameter;
typedef TypedParameter<std::vector<Imath::V2f> > V2fVectorParameter;
typedef TypedParameter<std::vector<Imath::V3f> > V3fVectorParameter;
typedef TypedParameter<std::vector<Imath::V2d> > V2dVectorParameter;
typedef TypedParameter<std::vector<Imath::V3d> > V3dVectorParameter;
typedef TypedParameter<std::vector<Imath::Box3f> > Box3fVectorParameter;
typedef TypedParameter<std::vector<Imath::Box3d> > Box3dVectorParameter;
typedef TypedParameter<std::vector<Imath::M33f> > M33fVectorParameter;
typedef TypedParameter<std::vector<Imath::M44f> > M44fVectorParameter;
typedef TypedParameter<std::vector<Imath::M33d> > M33dVectorParameter;
typedef TypedParameter<std::vector<Imath::M44d> > M44dVectorParameter;
typedef TypedParameter<std::vector<Imath::Quatf> > QuatfVectorParameter;
typedef TypedParameter<std::vector<Imath::Quatd> > QuatdVectorParameter;
typedef TypedParameter<std::vector<Imath::Color3f> > Color3fVectorParameter;
typedef TypedParameter<std::vector<Imath::Color4f> > Color4fVectorParameter;

IE_CORE_DECLAREPTR( BoolParameter );
IE_CORE_DECLAREPTR( StringParameter );
IE_CORE_DECLAREPTR( V2iParameter );
IE_CORE_DECLAREPTR( V3iParameter );
IE_CORE_DECLAREPTR( V2fParameter );
IE_CORE_DECLAREPTR( V3fParameter );
IE_CORE_DECLAREPTR( V2dParameter );
IE_CORE_DECLAREPTR( V3dParameter );
IE_CORE_DECLAREPTR( Color3fParameter );
IE_CORE_DECLAREPTR( Color4fParameter );
IE_CORE_DECLAREPTR( Box2iParameter );
IE_CORE_DECLAREPTR( Box3iParameter );
IE_CORE_DECLAREPTR( Box2fParameter );
IE_CORE_DECLAREPTR( Box3fParameter );
IE_CORE_DECLAREPTR( Box2dParameter );
IE_CORE_DECLAREPTR( Box3dParameter );
IE_CORE_DECLAREPTR( M44fParameter );
IE_CORE_DECLAREPTR( M44dParameter );

IE_CORE_DECLAREPTR( IntVectorParameter );
IE_CORE_DECLAREPTR( FloatVectorParameter );
IE_CORE_DECLAREPTR( DoubleVectorParameter );
IE_CORE_DECLAREPTR( StringVectorParameter );
IE_CORE_DECLAREPTR( V2fVectorParameter );
IE_CORE_DECLAREPTR( V3fVectorParameter );
IE_CORE_DECLAREPTR( V2dVectorParameter );
IE_CORE_DECLAREPTR( V3dVectorParameter );
IE_CORE_DECLAREPTR( Box3fVectorParameter );
IE_CORE_DECLAREPTR( Box3dVectorParameter );
IE_CORE_DECLAREPTR( M33fVectorParameter );
IE_CORE_DECLAREPTR( M44fVectorParameter );
IE_CORE_DECLAREPTR( M33dVectorParameter );
IE_CORE_DECLAREPTR( M44dVectorParameter );
IE_CORE_DECLAREPTR( QuatfVectorParameter );
IE_CORE_DECLAREPTR( QuatdVectorParameter );
IE_CORE_DECLAREPTR( Color3fVectorParameter );
IE_CORE_DECLAREPTR( Color4fVectorParameter );

} // namespace IECore

#endif // IE_CORE_NUMERICPARAMETER_H
