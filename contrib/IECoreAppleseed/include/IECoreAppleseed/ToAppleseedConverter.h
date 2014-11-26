//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_TOAPPLESEEDCONVERTER_H
#define IECOREAPPLESEED_TOAPPLESEEDCONVERTER_H

#include "renderer/api/entity.h"

#include "IECore/FromCoreConverter.h"
#include "IECore/CompoundData.h"

#include "IECoreAppleseed/TypeIds.h"

namespace IECoreAppleseed
{

IE_CORE_FORWARDDECLARE( ToAppleseedConverter );

/// A base class for all classes which convert from an IECore datatype
/// to an appleseed entity.
class ToAppleseedConverter : public IECore::FromCoreConverter
{

	public :

		/// This typedef describes the input object type for the converter.
		/// Derived classes /must/ override this typedef to more accurately
		/// describe their particular conversion in detail - it is used by
		/// the ConverterDescription to correctly register the converter
		/// with the factory mechanism.
		typedef IECore::Object InputType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToAppleseedConverter, ToAppleseedConverterTypeId, IECore::FromCoreConverter );

		/// Performs the conversion, returning an Appleseed entity representing the
		/// input object.
		renderer::Entity *convert() const;

		/// Creates a suitable ToAppleseedConverter operating on the specified object.
		static ToAppleseedConverterPtr create( IECore::ObjectPtr object );

	protected:

		ToAppleseedConverter( const std::string &description, IECore::TypeId supportedType );

		virtual ~ToAppleseedConverter();

		/// Must be implemented by derived classes to perform the conversion. It is guaranteed that the object
		/// is of the supported type and the parameter values have been validated and placed in operands.
		virtual renderer::Entity *doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ConverterDescription
		{
			public :
				ConverterDescription();
			private :
				static ToAppleseedConverterPtr creator( IECore::ObjectPtr object );
		};

	private :

		typedef ToAppleseedConverterPtr (*CreatorFn)( IECore::ObjectPtr );
		typedef std::map<IECore::TypeId, CreatorFn> CreatorMap;
		static CreatorMap &creators();

};

} // namespace IECoreAppleseed

#include "IECoreAppleseed/ToAppleseedConverter.inl"

#endif // IECOREAPPLESEED_TOAPPLESEEDCONVERTER_H
