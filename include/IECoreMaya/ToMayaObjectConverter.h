//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_TOMAYAOBJECTCONVERTER_H
#define IE_COREMAYA_TOMAYAOBJECTCONVERTER_H

#include "IECoreMaya/ToMayaConverter.h"

#include "IECore/Object.h"

#include "maya/MObjectHandle.h"

namespace IECoreMaya
{

class ToMayaObjectConverter;
IE_CORE_DECLAREPTR( ToMayaObjectConverter );

/// The ToMayaObjectConverter class forms a base class for
/// convertions to MObject datatype.
class ToMayaObjectConverter : public ToMayaConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaObjectConverter, ToMayaObjectConverterTypeId, ToMayaConverter );

		/// Converts the IECore::Object into the given MObject and returns True if successful and false otherwise.
		/// \todo Define the meaning of object. Some derived classes seem to modify it, others seem to create a
		/// new one and assign it. I think if a valid object is passed then it should be edited, and if a null object
		/// is passed then a new one should be created. We also probably need a ToMayaShapeConverter base class with
		/// some parentOrOwner type semantics.
		bool convert( MObject &object ) const;

		/// Creates a converter which will convert the given IECore::Object to a maya object
		/// of any relevant type. Returns 0 if no such converter can be found.
		static ToMayaObjectConverterPtr create( IECore::ConstObjectPtr object );
		/// Creates a converter which will convert the given IECore::Object to a MObject
		/// of the specified type. Returns 0 if no such converter can be found.
		static ToMayaObjectConverterPtr create( IECore::ConstObjectPtr object, MFn::Type resultType );

	protected :

		ToMayaObjectConverter( const std::string &name, const std::string &description, IECore::ConstObjectPtr object );

		/// Must be implemented by subclasses. Is guaranteed only to be called when object()
		/// returns a valid IECore::Object of a type specified when the converter was registered.
		/// \todo I don't think that guarantee is accurate at all.
		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const = 0;

		typedef ToMayaObjectConverterPtr (*CreatorFn)( IECore::ConstObjectPtr object );

		static void registerConverter( IECore::TypeId fromType, const MFn::Type resultType, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ToMayaObjectConverterDescription
		{
			public :
				/// \todo Constructors allowing more complex mappings of types (multiple fromTypes
				/// and resultTypes).
				ToMayaObjectConverterDescription( IECore::TypeId fromType, const MFn::Type resultType );
			private :
				static ToMayaObjectConverterPtr creator( IECore::ConstObjectPtr object );
		};

	private :

		struct Types
		{
			Types( IECore::TypeId from, MFn::Type result );
			IECore::TypeId fromType;
			MFn::Type resultType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		static TypesToFnsMap *typesToFns();

};

} // namespace IECoreMaya

#include "IECoreMaya/ToMayaObjectConverter.inl"

#endif // IE_COREMAYA_TOMAYACONVERTER_H
