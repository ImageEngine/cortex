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

#ifndef IE_CORE_NUMERICPARAMETER_H
#define IE_CORE_NUMERICPARAMETER_H

#include "IECore/Parameter.h"
#include "IECore/TypedData.h"
#include "OpenEXR/ImathLimits.h"

namespace IECore
{

/// A template class for numeric parameters, with value validation
/// on a min/max range. TypedData<T> is used to store the value.
template<typename T>
class NumericParameter : public Parameter
{
	public :
	
		typedef T ValueType;
		typedef TypedData<T> ObjectType;
		IE_CORE_DECLAREMEMBERPTR( NumericParameter<T> );
		IE_CORE_DECLAREPTR( ObjectType );
		typedef std::pair<std::string, T> Preset;
		typedef std::vector<Preset> PresetsContainer;
		typedef Parameter BaseClass;
	
		NumericParameter( const std::string &name, const std::string &description, T defaultValue = T(),
			T minValue = Imath::limits<T>::min(), T maxValue = Imath::limits<T>::max(),
			const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
			
		NumericParameter( const std::string &name, const std::string &description, T defaultValue,
			const PresetsContainer &presets, ConstCompoundObjectPtr userData = 0 );	
		
		//! @name RunTimeTyped functions
		////////////////////////////////////
		//@{
		virtual TypeId typeId() const;
		virtual const char *typeName() const;
		virtual bool isInstanceOf( TypeId typeId ) const;
		virtual bool isInstanceOf( const char *typeName ) const;
		static TypeId staticTypeId();
		static const char *staticTypeName();
		static bool inheritsFrom( TypeId typeId );
		static bool inheritsFrom( const char *typeName );
		static TypeId baseTypeId();
		static const char *baseTypeName();
		//@}
		
		//! @name Object functions
		////////////////////////////////////
		//@{
		typename NumericParameter<T>::Ptr copy() const;
		virtual bool isEqualTo( ConstObjectPtr other ) const;
		//@}
		
		bool hasMinValue() const;
		T minValue() const;
		
		bool hasMaxValue() const;
		T maxValue() const;
		
		/// Convenience function for getting the default value, which avoids all
		/// the hoop jumping needed to extract the value from the Object returned
		/// by Parameter::defaultValue()
		T numericDefaultValue() const;
		
		/// Convenience function for value getting, which avoids all the hoop
		/// jumping needed to extract the value from the Object returned
		/// by Parameter::getValue(). Throws an exception if the value is not
		/// valid.
		T getNumericValue() const;
		/// Convenience function for value setting - constructs a TypedData<T>
		/// from value and calls Parameter::setValue()
		void setNumericValue( T value );
		
		/// Implemented to return true only if value is of type TypedData<T> and if
		/// min <= value->readable() <= max.		
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;

	protected :
	
		// constructor for use during load/copy
		NumericParameter();

		virtual void copyFrom( ConstObjectPtr other, CopyContext *context );
		virtual void save( SaveContext *context ) const;
		virtual void load( LoadContextPtr context );
		virtual void memoryUsage( Object::MemoryAccumulator &accumulator ) const;

	private :	

		T m_min;
		T m_max;
	
		static TypeDescription<NumericParameter<T> > g_typeDescription;
		friend class TypeDescription<NumericParameter<T> >;

		static const unsigned int g_ioVersion;
};

typedef NumericParameter<int> IntParameter;
typedef NumericParameter<float> FloatParameter;
typedef NumericParameter<double> DoubleParameter;

IE_CORE_DECLAREPTR( IntParameter );
IE_CORE_DECLAREPTR( FloatParameter );
IE_CORE_DECLAREPTR( DoubleParameter );

} // namespace IECore

#endif // IE_CORE_NUMERICPARAMETER_H
