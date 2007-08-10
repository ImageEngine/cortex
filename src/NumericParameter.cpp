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

#include "IECore/NumericParameter.h"
#include "IECore/CompoundObject.h"

using namespace std;
using namespace IECore;
using namespace boost;

/////////////////////////////////////////////////////////////////////////////////////
// constructor stuff
/////////////////////////////////////////////////////////////////////////////////////

template<typename T>
static Parameter::PresetsMap convertPresets( const typename NumericParameter<T>::PresetsMap p )
{
	Parameter::PresetsMap result;
	for( typename NumericParameter<T>::PresetsMap::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.insert( typename Parameter::PresetsMap::value_type( it->first, new TypedData<T>( it->second ) ) );
	}
	return result;
}

template<typename T>
NumericParameter<T>::NumericParameter( const std::string &name, const std::string &description, T defaultValue,
	T minValue, T maxValue, const PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new ObjectType( defaultValue ), convertPresets<T>( presets ), presetsOnly, userData ), m_min( minValue ), m_max( maxValue )	
{
}

/////////////////////////////////////////////////////////////////////////////////////
// runtimetyped stuff
/////////////////////////////////////////////////////////////////////////////////////

template <class T> 
TypeId NumericParameter<T>::typeId() const
{
	return staticTypeId();
}

template <class T> 
TypeId NumericParameter<T>::staticTypeId()
{
	assert( 0 ); // this function must be specialised for each type!
	return InvalidTypeId;
}

template <class T> 
std::string NumericParameter<T>::typeName() const
{
	return staticTypeName();
}

template <class T> 
std::string NumericParameter<T>::staticTypeName()
{
	assert( 0 ); // this function must be specialised for each type!
	return "";
}

template<class T>
bool NumericParameter<T>::isInstanceOf( TypeId typeId ) const
{
	if( typeId==staticTypeId() )
	{
		return true;
	}
	return Parameter::isInstanceOf( typeId );
}

template<class T>
bool NumericParameter<T>::isInstanceOf( const std::string &typeName ) const
{
	if( typeName==staticTypeName() )
	{
		return true;
	}
	return Parameter::isInstanceOf( typeName );
}

template<class T>
bool NumericParameter<T>::inheritsFrom( TypeId typeId )
{
	return Parameter::staticTypeId()==typeId ? true : Parameter::inheritsFrom( typeId );
}

template<class T>
bool NumericParameter<T>::inheritsFrom( const std::string &typeName )
{
	return Parameter::staticTypeName()==typeName ? true : Parameter::inheritsFrom( typeName );
}

/////////////////////////////////////////////////////////////////////////////////////
// other stuff
/////////////////////////////////////////////////////////////////////////////////////					

template<typename T>
bool NumericParameter<T>::hasMinValue() const
{
	return m_min != Imath::limits<T>::min();
}

template<typename T>
T NumericParameter<T>::minValue() const
{
	return m_min;
}

template<typename T>
bool NumericParameter<T>::hasMaxValue() const
{
	return m_max != Imath::limits<T>::max();
}

template<typename T>
T NumericParameter<T>::maxValue() const
{
	return m_max;
}

template<typename T>
T NumericParameter<T>::numericDefaultValue() const
{
	intrusive_ptr<const TypedData<T> > v = static_pointer_cast<const TypedData<T> >( defaultValue() );
	return v->readable();
}

template<typename T>
T NumericParameter<T>::getNumericValue() const
{
	intrusive_ptr<const TypedData<T> > v = runTimeCast<const TypedData<T> >( getValidatedValue() );
	if (!v)
	{
		throw Exception( string( "Value is not an instance of \"" ) + ObjectType::staticTypeName() + "\"" );
	}
	return v->readable();
}

template<typename T>
void NumericParameter<T>::setNumericValue( T value )
{
	setValue( new TypedData<T>( value ) );
}	
								
template<typename T>
bool NumericParameter<T>::valueValid( ConstObjectPtr value, std::string *reason ) const
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
	if( tValue->readable() < m_min || tValue->readable() > m_max )
	{
		if( reason )
		{
			*reason = "Value is not in range.";
		}
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// specialisation and template instantiation
/////////////////////////////////////////////////////////////////////////////////////	


#define IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( T, TNAME )					\
																					\
	template<>																		\
	TypeId NumericParameter<T>::staticTypeId()										\
	{																				\
		return TNAME ## TypeId;														\
	}																				\
																					\
	template<>																		\
	std::string NumericParameter<T>::staticTypeName()								\
	{																				\
		return # TNAME;																\
	}																				\
																					\
	template class NumericParameter<T>;												\

namespace IECore
{	
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( int, IntParameter )
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( float, FloatParameter )
IE_CORE_DEFINENUMERICPARAMETERSPECIALISATION( double, DoubleParameter )
}
