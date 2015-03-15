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

/// This file contains the implementation of TypedObjectParameter. Rather than include it
/// in a public header it is #included in TypedObjectParameter.cpp and TypedPrimitiveParameter.cpp,
/// and the relevant template classes are explicitly instantiated there. This cuts down compilation
/// times.

#include "boost/static_assert.hpp"

#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"

namespace IECore
{

template<typename T>
TypedObjectParameter<T>::TypeDescription<TypedObjectParameter<T> > TypedObjectParameter<T>::g_typeDescription;

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

template<typename T>
bool TypedObjectParameter<T>::valueValid( const Object *value, std::string *reason ) const
{
	if( !ObjectParameter::valueValid( value, reason ) )
	{
		return false;
	}
	const ObjectType *tValue = runTimeCast<const ObjectType>( value );
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

#define IE_CORE_DEFINETYPEDOBJECTPARAMETERSPECIALISATION( T, TNAME )\
	\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TNAME##TypeId );\
	\
	template class IECORE_API TypedObjectParameter<T>;

}
