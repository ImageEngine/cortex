//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORENUKE_TONUKEGEOMETRYCONVERTER_H
#define IE_CORENUKE_TONUKEGEOMETRYCONVERTER_H

#include "IECoreNuke/ToNukeConverter.h"

#include "IECore/NumericParameter.h"
#include "IECore/Object.h"

#include "DDImage/GeometryList.h"

namespace IECoreNuke
{

IE_CORE_FORWARDDECLARE( ToNukeGeometryConverter );

/// The ToNukeGeometryConverter class allows conversion from an IECore Object to a Nuke geometry within the given GeometryList.
class IECORENUKE_API ToNukeGeometryConverter : public ToNukeConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToNukeGeometryConverter, ToNukeGeometryConverterTypeId, ToNukeConverter );

		/// Converts the srcParameter() value to a geometry within the given GeometryList.
		void convert( DD::Image::GeometryList &geoList ) const;

		/// Creates a converter which will convert the given IECore::Object to a Nuke geometry
		/// of any relevant type. Returns 0 if no such converter can be found.
		static ToNukeGeometryConverterPtr create( IECore::ConstObjectPtr src );

	protected :

		ToNukeGeometryConverter( const std::string &description, IECore::TypeId fromType, IECore::ConstObjectPtr object );

		/// Must be implemented by subclasses. Is guaranteed only to be called when object()
		/// returns a valid IECore::Object of a type specified when the converter was registered.
		virtual void doConversion( const IECore::Object *from, DD::Image::GeometryList &to, int objIndex, const IECore::CompoundObject *operands ) const = 0;

		typedef ToNukeGeometryConverterPtr (*CreatorFn)( IECore::ConstObjectPtr object );

		static void registerConverter( IECore::TypeId fromType, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ToNukeGeometryConverterDescription
		{
			public :
				ToNukeGeometryConverterDescription( IECore::TypeId fromType );
			private :
				static ToNukeGeometryConverterPtr creator( IECore::ConstObjectPtr object );
		};

	private :

		struct Types
		{
			Types( IECore::TypeId from );
			IECore::TypeId fromType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		static TypesToFnsMap *typesToFns();

		IECore::IntParameterPtr m_objIndexParameter;
};

} // namespace IECoreNuke

#include "IECoreNuke/ToNukeGeometryConverter.inl"

#endif // IE_CORENUKE_TONUKEGEOMETRYCONVERTER_H
