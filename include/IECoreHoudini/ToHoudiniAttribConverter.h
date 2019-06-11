//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINIATTRIBCONVERTER_H
#define IECOREHOUDINI_TOHOUDINIATTRIBCONVERTER_H

#include "IECoreHoudini/ToHoudiniConverter.h"
#include "IECoreHoudini/TypeIds.h"

#include "IECore/Data.h"

#include "GA/GA_AttributeRef.h"
#include "GU/GU_Detail.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( ToHoudiniAttribConverter );

/// The ToHoudiniAttribConverter class forms an abstract base class for all classes
/// able to perform some kind of conversion from IECore::Data to a Houdini GB_Attribute.
/// The resulting GB_Attribute will be transferred onto the provided GU_Detail.
class ToHoudiniAttribConverter : public ToHoudiniConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniAttribConverter, ToHoudiniAttribConverterTypeId, ToHoudiniConverter );

		/// Converts the IECore::Data into a GB_Attribute on the given GU_Detail and returns the
		/// associated GA_RWAttributeRef. It is assumed that the user has aquired the write lock
		/// for the given GU_Detail.
		GA_RWAttributeRef convert( std::string name, GU_Detail *geo ) const;
		GA_RWAttributeRef convert( std::string name, GU_Detail *geo, const GA_Range &range ) const;

		/// Creates a converter which will convert the given IECore::Data to a Houdini GB_Attribute.
		/// Returns 0 if no such converter can be found.
		static ToHoudiniAttribConverterPtr create( const IECore::Data *data );

	protected :

		ToHoudiniAttribConverter( const IECore::Data *data, const std::string &description );

		virtual ~ToHoudiniAttribConverter();

		/// Must be implemented by derived classes to create a GB_Attribute on the given GU_Detail and fill it with the IECore::Data
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const = 0;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const = 0;

		typedef ToHoudiniAttribConverterPtr (*CreatorFn)( const IECore::Data *data );

		static void registerConverter( IECore::TypeId fromType, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( IECore::TypeId fromType );
			private :
				static ToHoudiniAttribConverterPtr creator( const IECore::Data *data );
		};

	private :

		/// Struct for maintaining the registered derived classes
		struct Types
		{
			Types( IECore::TypeId from );
			IECore::TypeId fromType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		static TypesToFnsMap *typesToFns();

};

} // namespace IECoreHoudini

#include "ToHoudiniAttribConverter.inl"

#endif // IECOREHOUDINI_TOHOUDINIATTRIBCONVERTER_H
