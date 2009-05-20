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

#ifndef IE_CORE_TYPEDPARAMETER_INL
#define IE_CORE_TYPEDPARAMETER_INL

#include "boost/static_assert.hpp"

#include "IECore/TypedParameter.h"
#include "IECore/CompoundObject.h"

namespace IECore
{

/////////////////////////////////////////////////////////////////////////////////////
// constructor stuff
/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
static Parameter::PresetsContainer convertPresets( const typename TypedParameter<T>::PresetsContainer &p )
{
	Parameter::PresetsContainer result;
	for( typename TypedParameter<T>::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.push_back( typename Parameter::PresetsContainer::value_type( it->first, new (typename TypedParameter<T>::ObjectType)( it->second ) ) );
	}
	return result;
}

template<typename T>
static Parameter::PresetsContainer convertPresets( const typename TypedParameter<T>::ObjectPresetsContainer &p )
{
	Parameter::PresetsContainer result;
	for( typename TypedParameter<T>::ObjectPresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.push_back( typename Parameter::PresetsContainer::value_type( it->first, it->second ) );
	}
	return result;
}

template<typename T>
TypedParameter<T>::TypedParameter()
{
}

template<typename T>
TypedParameter<T>::TypedParameter( const std::string &name, const std::string &description, const T &defaultValue,
	const PresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new ObjectType( defaultValue ), convertPresets<T>( presets ), presetsOnly, userData )
{
}

template<typename T>
TypedParameter<T>::TypedParameter( const std::string &name, const std::string &description, ObjectTypePtr defaultValue,
	const ObjectPresetsContainer &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, defaultValue, convertPresets<T>( presets ), presetsOnly, userData )
{
}

/////////////////////////////////////////////////////////////////////////////////////
// runtimetyped stuff
/////////////////////////////////////////////////////////////////////////////////////

template <class T>
const Object::TypeDescription<TypedParameter<T> > TypedParameter<T>::g_typeDescription;

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
const char *TypedParameter<T>::typeName() const
{
	return staticTypeName();
}

template <class T>
const char *TypedParameter<T>::staticTypeName()
{
	BOOST_STATIC_ASSERT( sizeof(T) == 0 ); // this function must be specialised for each type!
	return "";
}

template <class T>
TypeId TypedParameter<T>::baseTypeId()
{
	return Parameter::staticTypeId();
}

template <class T>
const char *TypedParameter<T>::baseTypeName()
{
	return Parameter::staticTypeName();
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
bool TypedParameter<T>::isInstanceOf( const char *typeName ) const
{
	if( !strcmp( typeName, staticTypeName() ) )
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
bool TypedParameter<T>::inheritsFrom( const char *typeName )
{
	return !strcmp( Parameter::staticTypeName(), typeName ) ? true : Parameter::inheritsFrom( typeName );
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
			*reason = std::string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"";
		}
		return false;
	}
	return true;
}

template<typename T>
const typename TypedParameter<T>::ValueType &TypedParameter<T>::typedDefaultValue() const
{
	return boost::static_pointer_cast<const ObjectType>( defaultValue() )->readable();
}

template<typename T>
typename TypedParameter<T>::ValueType &TypedParameter<T>::getTypedValue()
{
	ConstObjectTypePtr tValue = runTimeCast<const ObjectType>( getValue() );
	if( !tValue )
	{
		throw Exception( std::string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"");
	}
	return boost::static_pointer_cast<ObjectType>( getValue() )->writable();
}

template<typename T>
const typename TypedParameter<T>::ValueType &TypedParameter<T>::getTypedValue() const
{
	ConstObjectTypePtr tValue = runTimeCast<const ObjectType>( getValue() );
	if( !tValue )
	{
		throw Exception( std::string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"");
	}
	return boost::static_pointer_cast<const ObjectType>( getValue() )->readable();
}

template<typename T>
void TypedParameter<T>::setTypedValue( const T &value )
{
	setValue( new ObjectType( value ) );
}

/////////////////////////////////////////////////////////////////////////////////////
// object stuff
/////////////////////////////////////////////////////////////////////////////////////

template <class T>
typename TypedParameter<T>::Ptr TypedParameter<T>::copy() const
{
	return boost::static_pointer_cast<TypedParameter<T> >( copy() );
}

template<class T>
void TypedParameter<T>::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Parameter::copyFrom( other, context );
}

template<class T>
void TypedParameter<T>::save( SaveContext *context ) const
{
	Parameter::save( context );
}

template<class T>
void TypedParameter<T>::load( LoadContextPtr context )
{
	Parameter::load( context );
}

template<class T>
bool TypedParameter<T>::isEqualTo( ConstObjectPtr other ) const
{
	return Parameter::isEqualTo( other );
}

template<class T>
void TypedParameter<T>::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Parameter::memoryUsage( a );
}

/////////////////////////////////////////////////////////////////////////////////////
// specialisation and template instantiation
/////////////////////////////////////////////////////////////////////////////////////


#define IE_CORE_DEFINETYPEDPARAMETERSPECIALISATION( T, TNAME ) \
	\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TNAME##TypeId );\
	\
	template class TypedParameter<T>;

}

#endif // IE_CORE_TYPEDPARAMETER_INL
