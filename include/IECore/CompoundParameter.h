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

#ifndef IE_CORE_COMPOUNDPARAMETER_H
#define IE_CORE_COMPOUNDPARAMETER_H

#include "IECore/Parameter.h"
#include "IECore/CompoundObject.h"

namespace IECore
{

/// The CompoundParameter class implements a Parameter holding many
/// other parameter values, a little like a struct.
class CompoundParameter : public Parameter
{
	public :

		IE_CORE_DECLAREOBJECT( CompoundParameter, Parameter );

		/// A type to map from names to Parameters.
		typedef std::map<InternedString, ParameterPtr> ParameterMap;
		/// A type to hold a vector of Parameters.
		typedef std::vector<ParameterPtr> ParameterVector;

		/// Creates an empty CompoundParameter.
		CompoundParameter( const std::string &name = "", const std::string &description = "", ConstCompoundObjectPtr userData=0 );
		/// Create a CompoundParameter containing all the parameters in the range specified
		/// by the forward iterators membersBegin and membersEnd, which are expected to point
		/// to ParameterPtr objects.
		template<typename I>
		CompoundParameter( const std::string &name, const std::string &description, I membersBegin, I membersEnd, ConstCompoundObjectPtr userData=0 );

		//! @name Parameter method overrides.
		////////////////////////////////////////////////////////////////////////
		//@{
		/// Implemented to return a CompoundObject representing the default values
		/// of all the child objects.
		virtual ConstObjectPtr defaultValue() const;
		/// Implemented to update the presets with the intersection of the presets
		/// of all the child parameters. Please note that the map returned may differ between
		/// one call to presets() and the next.
		virtual const PresetsContainer &presets() const;
		/// Implemented to return true only if all children have presetsOnly() true.
		virtual bool presetsOnly() const;
		/// Values are only valid if they are a CompoundObject with a valid member
		/// for each child parameter, and no additional values.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;
		/// Sets the values of child parameters using the matching child objects of the passed CompoundObject.
		/// In the case of missing values (or if the value isn't even a CompoundParameter) sets the child parameter
		/// value to a NullObject instance to signify it's invalidity.
		virtual void setValue( ObjectPtr value );
		/// If the last set value was a CompoundObject (as it should have been) then updates it with the current
		/// child parameter values and returns it.
		virtual ObjectPtr getValue();
		/// As above.
		virtual ConstObjectPtr getValue() const;
		//@}

		//! @name Child Parameter access
		/// These functions provide access to the child parameters held
		/// by the CompoundParameter.
		/////////////////////////////////////////////////////////////////////////
		//@{
		/// Adds a Parameter to the end of the child parameters. Throws an Exception
		/// if a child of that name already exists.
		void addParameter( ParameterPtr parameter );
		/// Calls addParameter for all parmaters between the iterators begin and end.
		/// These iterators should point to ParameterPtrs.
		template<typename I>
		void addParameters( I begin, I end );
		/// Inserts parameter before other in the list of child parameters. Throws
		/// an Exception if an identically named parameter already exists, or if
		/// other is not a child parameter.
		void insertParameter( ParameterPtr parameter, ConstParameterPtr other );
		/// Removes the given parameter. Throws an Exception if parameter is not
		/// a child of this object.
		void removeParameter( ParameterPtr parameter );
		/// Removes the parameter with the given name. Throws an Exception if no
		/// such named parameter exists.
		void removeParameter( const std::string &name );
		/// Gives access to the parameters which are members of this compound in
		/// a form which maps from parameter names to parameters.
		/// The map itself cannot be modified (the parameters are fixed) but the values
		/// of the parameters contained may be set.
		const ParameterMap &parameters() const;
		/// Gives access to the parameters which are members of this compound in
		/// a form which maintains the order in which they were passed to the
		/// constructor.
		const ParameterVector &orderedParameters() const;
		/// Convenience function to find a parameter in parameters(). Returns 0 if
		/// the parameter doesn't exist, or does not match the type specified
		/// as the template argument.
		template<typename T>
		typename T::Ptr parameter( const std::string &name );
		template<typename T>
		typename T::ConstPtr parameter( const std::string &name ) const;
		/// Searches for child recursively underneath this parameter, filling path
		/// with the names of all its ancestors, plus the name of child
		/// itself. Returns true if child is found and false otherwise.
		bool parameterPath( ConstParameterPtr child, std::vector<std::string> &path ) const;
		/// Convenience function to find a parameter in parameters() and call setValue()
		/// on it. Throws an Exception if the named parameter doesn't exist.
		void setParameterValue( const std::string &name, ObjectPtr value );
		/// Convenience function to find a parameter in parameters() and call setValidatedValue()
		/// on it. Throws an Exception if the named parameter doesn't exist.
		void setValidatedParameterValue( const std::string &name, ObjectPtr value );
		/// Convenience function to find a parameter in parameters() and return the
		/// result of calling getValue() on it. Throws an Exception if the
		/// named parameter doesn't exist.
		ObjectPtr getParameterValue( const std::string &name );
		/// Convenience function to find a parameter in parameters() and return the
		/// result of calling getValidatedValue() on it. Throws an Exception if the
		/// named parameter doesn't exist.
		ObjectPtr getValidatedParameterValue( const std::string &name );
		//@}

	private :

		ParameterMap m_namesToParameters;
		ParameterVector m_parameters;

		static const unsigned int g_ioVersion;

};

IE_CORE_DECLAREPTR( CompoundParameter );

} // namespace IECore

#include "IECore/CompoundParameter.inl"

#endif // IE_CORE_COMPOUNDPARAMETER_H
