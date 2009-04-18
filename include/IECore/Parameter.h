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

#ifndef IE_CORE_PARAMETER_H
#define IE_CORE_PARAMETER_H

#include "IECore/RunTimeTyped.h"
#include "IECore/Interned.h"

#include <map>
#include <vector>
#include <string>

namespace IECore
{

IE_CORE_FORWARDDECLARE( Object );
IE_CORE_FORWARDDECLARE( CompoundObject );

/// The Parameter base class represents a means of describing data to be passed
/// to some process.
class Parameter : public RunTimeTyped
{
	public :
	
		IE_CORE_DECLARERUNTIMETYPED( Parameter, RunTimeTyped );
	
		/// A type which associates a value for the Parameter with
		/// a name.
		typedef std::pair<std::string, ObjectPtr> Preset;
		/// A type to store a bunch of preset values for the Parameter.
		typedef std::vector<Preset> PresetsContainer;
	
		/// Creates a new Parameter. If presetsOnly is true then the parameter acts somewhat
		/// like an enum and only accepts values if they are present in the presets map.
		Parameter( const std::string &name, const std::string &description, ObjectPtr defaultValue,
			const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0 );

		virtual ~Parameter();

		//! @name Basic Parameter information
		////////////////////////////////////////////////////////////
		//@{
		/// Returns the name of this parameter.
		const std::string &name() const;
		inline const InternedString &internedName() const;
		/// Returns the description for this parameter.
		const std::string &description() const;
		/// Returns the default value for this parameter.
		virtual ConstObjectPtr defaultValue() const;
		/// Returns the presets for this parameter.
		virtual const PresetsContainer &presets() const;
		/// Returns true if this parameter only accepts
		/// parameters present as presets.
		virtual bool presetsOnly() const;
		/// Returns the userdata. This can be modified freely.
		CompoundObjectPtr userData();
		/// Read only version of the above.
		ConstCompoundObjectPtr userData() const;
		//@}
		
		//! @name Validation
		/// The Parameter class defines the concept of whether or not
		/// a value is valid. This is used to provide guarantees about
		/// the values passed to Parameterised processes. For various
		/// efficiency reasons it's possible to set an invalid value,
		/// but member functions are provided to guarantee validity
		/// when it's needed.
		//////////////////////////////////////////////////////////////
		//@{
		/// Determines whether or not a given value is suitable for
		/// use in this Parameter.
		/// Should be implemented in subclasses to perform
		/// validation on the supplied value, returning false
		/// if it's unsuitable. When returning false, reason
		/// should be set to some descriptive string.
		/// All implementations should call the
		/// base class valueValid(), returning false immediately if
		/// it returns false. The default implementation returns false only if
		/// presetsOnly() is true and value is not present in
		/// the presets() map, or if value is a NullObject instance.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;
		/// Calls valueValid( getValue(), reason )
		bool valueValid( std::string *reason = 0 ) const;
		/// Throws an Exception if valueValid( getValue() ) is false, otherwise
		/// does nothing.
		void validate() const;		
		/// Throws an Exception if valueValid( value ) is false, otherwise
		/// does nothing.
		void validate( ConstObjectPtr value ) const;
		//@}
		
		//! @name Value setting
		/// These functions set the Parameter value, with or without
		/// validation.
		//////////////////////////////////////////////////////////////
		//@{
		/// Sets the value held by this parameter. Note that this
		/// function will not prevent you from setting a value
		/// for which valueValid( value ) is false.
		virtual void setValue( ObjectPtr value );
		/// If valueValid( value ) is true, calls setValue( value ), otherwise
		/// throws an Exception.
		void setValidatedValue( ObjectPtr value );
		/// Sets the value held by this parameter to one of the named
		/// presets. Throws an exception if presetName is not a defined
		/// preset.
		void setValue( const std::string &presetName );
		//@}
		
		//! @name Value getting
		/// These functions provide access to the Parameter value, with or without
		/// validation.
		//////////////////////////////////////////////////////////////
		//@{
		/// Returns the value held by this parameter. Note that this
		/// value may not be valid.
		virtual ObjectPtr getValue();
		/// Returns the value held by this parameter. Note that this
		/// value may not be valid.
		virtual ConstObjectPtr getValue() const;
		/// Returns the value held by this parameter if it is valid, throwing
		/// an Exception if it is not.
		ObjectPtr getValidatedValue();
		/// Returns the value held by this parameter if it is valid, throwing
		/// an Exception if it is not.
		ConstObjectPtr getValidatedValue() const;
		/// Convenience function returning runTimeCast<T>( getValue() ).
		template<typename T>
		typename T::Ptr getTypedValue();
		/// Convenience function returning runTimeCast<T>( getValue() ).
		template<typename T>
		typename T::ConstPtr getTypedValue() const;
		/// Convenience function returning runTimeCast<T>( getValidatedValue() ).
		/// Note that if a value is valid but not of the requested type an Exception is not thrown.
		template<typename T>
		typename T::Ptr getTypedValidatedValue();
		/// Convenience function returning runTimeCast<T>( getValidatedValue() ).
		/// Note that if a value is valid but not of the requested type an Exception is not thrown.
		template<typename T>
		typename T::ConstPtr getTypedValidatedValue() const;
		/// If the current value is one of the presets, then returns its
		/// name, otherwise returns the empty string.
		std::string getCurrentPresetName() const;
		//@}
	
	private :	

		InternedString m_name;
		InternedString m_description;

		ObjectPtr m_value;
		ConstObjectPtr m_defaultValue;
		
		PresetsContainer m_presets;
		bool m_presetsOnly;

		mutable CompoundObjectPtr m_userData;
};

IE_CORE_DECLAREPTR( Parameter );

} // namespace IECore

#include "IECore/Parameter.inl"

#endif // IE_CORE_PARAMETER_H
