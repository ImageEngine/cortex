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

#include "boost/static_assert.hpp"

#include "IECore/TypedParameter.h"
#include "IECore/CompoundObject.h"

using namespace std;
using namespace IECore;
using namespace boost;
using namespace Imath;

/////////////////////////////////////////////////////////////////////////////////////
// constructor stuff
/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
static Parameter::PresetsMap convertPresets( const typename TypedParameter<T>::PresetsMap p )
{
	Parameter::PresetsMap result;
	for( typename TypedParameter<T>::PresetsMap::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.insert( typename Parameter::PresetsMap::value_type( it->first, new TypedData<T>( it->second ) ) );
	}
	return result;
}

template<typename T>
static Parameter::PresetsMap convertPresets( const typename TypedParameter<T>::ObjectPresetsMap p )
{
	Parameter::PresetsMap result;
	for( typename TypedParameter<T>::ObjectPresetsMap::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.insert( typename Parameter::PresetsMap::value_type( it->first, it->second ) );
	}
	return result;
}

template<typename T>
TypedParameter<T>::TypedParameter( const std::string &name, const std::string &description, const T &defaultValue,
	const PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new ObjectType( defaultValue ), convertPresets<T>( presets ), presetsOnly, userData )	
{
}

template<typename T>
TypedParameter<T>::TypedParameter( const std::string &name, const std::string &description, ObjectTypePtr defaultValue,
	const ObjectPresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, defaultValue, convertPresets<T>( presets ), presetsOnly, userData )
{
}

/////////////////////////////////////////////////////////////////////////////////////
// runtimetyped stuff
/////////////////////////////////////////////////////////////////////////////////////

template <class T> 
TypeId TypedParameter<T>::typeId() const
{
	return staticTypeId();
}

template <class T> 
TypeId TypedParameter<T>::staticTypeId()
{
	BOOST_STATIC_ASSERT( sizeof(T) == 0 ); // this function must be specialised for each type!
	return InvalidTypeId;
}

template <class T> 
std::string TypedParameter<T>::typeName() const
{
	return staticTypeName();
}

template <class T> 
std::string TypedParameter<T>::staticTypeName()
{
	BOOST_STATIC_ASSERT( sizeof(T) == 0 ); // this function must be specialised for each type!
	return "";
}

template<class T>
bool TypedParameter<T>::isInstanceOf( TypeId typeId ) const
{
	if( typeId==staticTypeId() )
	{
		return true;
	}
	return Parameter::isInstanceOf( typeId );
}

template<class T>
bool TypedParameter<T>::isInstanceOf( const std::string &typeName ) const
{
	if( typeName==staticTypeName() )
	{
		return true;
	}
	return Parameter::isInstanceOf( typeName );
}

template<class T>
bool TypedParameter<T>::inheritsFrom( TypeId typeId )
{
	return Parameter::staticTypeId()==typeId ? true : Parameter::inheritsFrom( typeId );
}

template<class T>
bool TypedParameter<T>::inheritsFrom( const std::string &typeName )
{
	return Parameter::staticTypeName()==typeName ? true : Parameter::inheritsFrom( typeName );
}

/////////////////////////////////////////////////////////////////////////////////////
// other stuff
/////////////////////////////////////////////////////////////////////////////////////					

template<typename T>
bool TypedParameter<T>::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if( !Parameter::valueValid( value, reason ) )
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

template<typename T>
const typename TypedParameter<T>::ValueType &TypedParameter<T>::typedDefaultValue() const
{
	return static_pointer_cast<const TypedData<T> >( defaultValue() )->readable();
}

template<typename T>
typename TypedParameter<T>::ValueType &TypedParameter<T>::getTypedValue()
{
	ConstObjectTypePtr tValue = runTimeCast<const ObjectType>( getValue() );
	if( !tValue )
	{
		throw Exception( string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"");
	}
	return static_pointer_cast<TypedData<T> >( getValue() )->writable();
}

template<typename T>
const typename TypedParameter<T>::ValueType &TypedParameter<T>::getTypedValue() const
{
	ConstObjectTypePtr tValue = runTimeCast<const ObjectType>( getValue() );
	if( !tValue )
	{
		throw Exception( string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"");
	}
	return static_pointer_cast<const TypedData<T> >( getValue() )->readable();
}
		
template<typename T>
void TypedParameter<T>::setTypedValue( const T &value )
{
	setValue( new TypedData<T>( value ) );
}

/////////////////////////////////////////////////////////////////////////////////////
// specialisation and template instantiation
/////////////////////////////////////////////////////////////////////////////////////	


#define IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( T, TNAME )						\
																					\
	template<>																		\
	TypeId TypedParameter<T>::staticTypeId()										\
	{																				\
		return TNAME ## TypeId;														\
	}																				\
																					\
	template<>																		\
	std::string TypedParameter<T>::staticTypeName()									\
	{																				\
		return # TNAME;																\
	}																				\
																					\
	template class TypedParameter<T>;

namespace IECore
{
/// \todo Split some of these off into separate files to speed up compilation times
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( bool, BoolParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( string, StringParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2i, V2iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3i, V3iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2f, V2fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3f, V3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V2d, V2dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( V3d, V3dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Color3f, Color3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Color4f, Color4fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2i, Box2iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3i, Box3iParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2f, Box2fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3f, Box3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box2d, Box2dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Box3d, Box3dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( M44f, M44fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( M44d, M44dParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Splineff, SplineffParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( Splinedd, SplineddParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( SplinefColor3f, SplinefColor3fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( SplinefColor4f, SplinefColor4fParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( CubeColorLookupf, CubeColorLookupfParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( CubeColorLookupd, CubeColorLookupdParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( boost::posix_time::ptime, DateTimeParameter )

IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<bool>, BoolVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<int>, IntVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<float>, FloatVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<double>, DoubleVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<string>, StringVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<V2f>, V2fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<V3f>, V3fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<V2d>, V2dVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<V3d>, V3dVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Box3f>, Box3fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Box3d>, Box3dVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<M33f>, M33fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<M44f>, M44fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<M33d>, M33dVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<M44d>, M44dVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Quatf>, QuatfVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Quatd>, QuatdVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Color3f>, Color3fVectorParameter )
IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( vector<Color4f>, Color4fVectorParameter )
}
