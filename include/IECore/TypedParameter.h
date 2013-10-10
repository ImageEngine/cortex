//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
#include "IECore/TypedData.h"
#include "IECore/TypedParameterInternals.h"

namespace IECore
{

/// A template class for simple typed parameters. TypedParameterTraits<T> is used to
/// determine the appropriate ObjectType which is used to store the value.
template<typename T>
class TypedParameter : public Parameter
{
	public :

		typedef T ValueType;
		typedef typename TypedParameterTraits<T>::ObjectType ObjectType;

		IE_CORE_DECLAREPTR( ObjectType );

		typedef std::pair<std::string, T> Preset;
		typedef std::pair<std::string, ObjectTypePtr> ObjectPreset;
		typedef std::vector<Preset> PresetsContainer;
		typedef std::vector<ObjectPreset> ObjectPresetsContainer;

		/// Constructs a new ObjectType object to hold the default value.
		TypedParameter( const std::string &name, const std::string &description, const T &defaultValue = T(),
			const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
		/// Takes a copy of defaultValue for use as the default value.
		TypedParameter( const std::string &name, const std::string &description, ObjectTypePtr defaultValue,
			const ObjectPresetsContainer &presets = ObjectPresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( TypedParameter<T>, Parameter );

		/// Implemented to return true only if value is of type TypedData<T>.
		virtual bool valueValid( const Object *value, std::string *reason = 0 ) const;

		/// Convenience function for getting the default value, which avoids all the hoop jumping
		/// needed to extract the value from the Object returned by Parameter::defaultValue().
		const ValueType &typedDefaultValue() const;

		/// Convenience function for value getting, which avoids all the hoop jumping
		/// needed to extract the value from the Object returned by Parameter::getValue().
		/// Throws an Exception if the value is not valid.
		/// \threading Multiple concurrent threads may read the value provided that
		/// no other thread is modifying it.
		ValueType &getTypedValue();
		const ValueType &getTypedValue() const;
		/// Convenience function for value setting, constructs a TypedData<T> from value
		/// and calls Parameter::setValue().
		void setTypedValue( const T &value );

	private :

		static const TypeDescription<TypedParameter<T> > g_typeDescription;

};

} // namespace IECore

#endif // IE_CORE_TYPEDPARAMETER_H
