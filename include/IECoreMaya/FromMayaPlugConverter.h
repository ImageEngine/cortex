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

#ifndef IE_COREMAYA_FROMMAYAPLUGCONVERTER_H
#define IE_COREMAYA_FROMMAYAPLUGCONVERTER_H

#include "IECoreMaya/FromMayaConverter.h"

#include "IECore/Object.h"

#include "maya/MPlug.h"
#include "maya/MFnNumericData.h"

#include <map>

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaPlugConverter );

/// The FromMayaPlugConverter class allows conversion from MPlug values to
/// an IECore Object.
/// \todo Remove the doConversion() implementation and protect the constructor - for
/// the next major version of the library.
class FromMayaPlugConverter : public FromMayaConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaPlugConverter, FromMayaPlugConverterTypeId, FromMayaConverter );

		/// Constructs a new converter which will do a slightly arbitrary
		/// conversion of the data in plug, with no control over the sort
		/// of object being returned.
		/// \deprecated You should use the create() method to create an
		/// appropriate subclass. This gives the capability of requesting the
		/// result type following conversion.
		FromMayaPlugConverter( const MPlug &plug );
		
		/// The MPlug which will be converted by the convert() function.
		const MPlug &plug() const;
		
		/// Creates a converter to convert the plug value to the "most appropriate" IECore
		/// type. May return 0 if no converter can be found. Note that this returns a generic
		/// FromMayaConverterPtr rather than a FromMayaPlugConverterPtr so that a
		/// FromMayaObjectConverterPtr may be returned for the case of a plug holding an MObject.
		static FromMayaConverterPtr create( const MPlug &plug );
		/// Creates a converter to convert the plug value to a specific IECore type. Maya return
		/// 0 if no such converter can be found.
		static FromMayaConverterPtr create( const MPlug &plug, IECore::TypeId resultType );
		
	protected :

		/// Should be implemented by all derived classes to convert the value in plug().
		/// \todo Implement this in a manner similar to FromMayaObjectConverter (forward on
		/// to a pure virtual function).
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;

		/// Creating a static instance of one of these (templated on your converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( MFnData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter );
				Description( MFnNumericData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter );
			private :
				static FromMayaPlugConverterPtr creator( const MPlug &object );
		};
		
	private :

		typedef FromMayaPlugConverterPtr (*CreatorFn)( const MPlug &plug );
		static void registerConverter( MFnNumericData::Type fromType, IECore::TypeId resultType, CreatorFn creator );
		static void registerConverter( MFnData::Type fromType, IECore::TypeId resultType, CreatorFn creator );
		
		typedef std::pair<MFnNumericData::Type, IECore::TypeId> NumericTypePair;
		typedef std::map<NumericTypePair, CreatorFn> NumericTypesToFnsMap;
		static NumericTypesToFnsMap *numericTypesToFns();
		
		typedef std::pair<MFnData::Type, IECore::TypeId> TypedTypePair;
		typedef std::map<TypedTypePair, CreatorFn> TypedTypesToFnsMap;
		static TypedTypesToFnsMap *typedTypesToFns();

		MPlug m_plug;

};

} // namespace IECoreMaya

#include "IECoreMaya/FromMayaPlugConverter.inl"

#endif // IE_COREMAYA_FROMMAYAPLUGCONVERTER_H
