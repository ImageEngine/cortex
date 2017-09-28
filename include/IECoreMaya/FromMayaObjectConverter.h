//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_FROMMAYAOBJECTCONVERTER_H
#define IE_COREMAYA_FROMMAYAOBJECTCONVERTER_H

#include "IECoreMaya/FromMayaConverter.h"

#include "IECore/Object.h"
#include "IECore/SimpleTypedParameter.h"

#include "maya/MObjectHandle.h"

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaObjectConverter );

/// The FromMayaConverter class forms an abstract base class for
/// all classes able to perform some kind of conversion
/// from a Maya datatype to an IECore datatype.
class FromMayaObjectConverter : public FromMayaConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaObjectConverter, FromMayaObjectConverterTypeId, FromMayaConverter );

		/// The MObject which will be converted by the convert() function.
		/// Will return MObject::kNullObj if objectIsAlive() is false.
		const MObject &object() const;
		/// If you allow the MObject passed to the constructor or create()
		/// call to die then the Converter is unable to convert it and return
		/// 0 from convert(). This function tells you if the object is still
		/// alive or not, and therefore what return value you can expect.
		bool objectIsAlive() const;

		//! @name Parameters
		/// Parameters which affect the conversion process.
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		IECore::StringParameterPtr blindDataAttrPrefixParameter();
		IECore::ConstStringParameterPtr blindDataAttrPrefixParameter() const;
		IECore::BoolParameterPtr blindDataRemoveNamespaceParameter();
		IECore::ConstBoolParameterPtr blindDataRemoveNamespaceParameter() const;
		//@}

		//! @name Factory
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given object to an IECore::Object.
		/// If resultType is specified then only converters which create objects of that
		/// type will be returned - the default value allows any suitable converter to be
		/// created. If no matching converters exist then returns 0.
		static FromMayaObjectConverterPtr create( const MObject &object, IECore::TypeId resultType=IECore::InvalidTypeId );
		//@}

	protected :

		FromMayaObjectConverter( const std::string &description, const MObject &object );

		/// Implemented to call doConversion as defined below only if object() has a suitable
		/// value - otherwise returns 0.
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by subclasses. Is guaranteed only to be called when object()
		/// returns a valid MObject of a type specified when the converter was registered.
		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const = 0;

		typedef FromMayaObjectConverterPtr (*CreatorFn)( const MObject &object );

		static void registerConverter( const MFn::Type fromType, IECore::TypeId resultType, bool defaultConversion, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class FromMayaObjectConverterDescription
		{
			public :
				/// \param fromType The maya type which can be converted.
				/// \param resultType The cortex type which will result from the conversion.
				/// \param defaultConversion Should be true if this conversion is the "best" for a given fromType. If
				/// this is true then this is the converter that will be used when create() is called without specifying
				/// a resultType.
				FromMayaObjectConverterDescription( MFn::Type fromType, IECore::TypeId resultType, bool defaultConversion );
			private :
				static FromMayaObjectConverterPtr creator( const MObject &object );
		};

	private :

		void addBlindData( const MObject &object, IECore::ObjectPtr convertedObject ) const;

		MObjectHandle m_objectHandle;
		IECore::StringParameterPtr m_blindDataAttrPrefixParameter;
		IECore::BoolParameterPtr m_blindDataRemoveNamespaceParameter;

		typedef std::pair<MFn::Type, IECore::TypeId> Types;
		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		typedef std::map<MFn::Type, TypesToFnsMap::const_iterator> DefaultConvertersMap;

		static TypesToFnsMap &typesToFns();
		static DefaultConvertersMap &defaultConverters();

};

} // namespace IECoreMaya

#include "IECoreMaya/FromMayaObjectConverter.inl"

#endif // IE_COREMAYA_FROMMAYACONVERTER_H
