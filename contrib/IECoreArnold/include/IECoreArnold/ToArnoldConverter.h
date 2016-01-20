//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREARNOLD_TOARNOLDCONVERTER_H
#define IECOREARNOLD_TOARNOLDCONVERTER_H

#include "ai.h"

#include "IECoreArnold/TypeIds.h"
#include "IECoreArnold/Export.h"

#include "IECore/FromCoreConverter.h"
#include "IECore/CompoundData.h"

namespace IECoreArnold
{

IE_CORE_FORWARDDECLARE( ToArnoldConverter );

/// A base class for all classes which convert from an IECore datatype
/// to an Arnold node.
class IECOREARNOLD_API ToArnoldConverter : public IECore::FromCoreConverter
{

	public :

		/// This typedef describes the input object type for the converter.
		/// Derived classes /must/ override this typedef to more accurately
		/// describe their particular conversion in detail - it is used by
		/// the ConverterDescription to correctly register the converter
		/// with the factory mechanism.
		typedef IECore::Object InputType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToArnoldConverter, ToArnoldConverterTypeId, IECore::FromCoreConverter );

		/// Performs the conversion, returning an Arnold node representing the
		/// input object.
		AtNode *convert() const;

		/// \todo Do these belong somewhere else?
		static void setParameter( AtNode *node, const AtParamEntry *parameter, const IECore::Data *value );
		static void setParameter( AtNode *node, const char *name, const IECore::Data *value );
		static void setParameters( AtNode *node, const IECore::CompoundDataMap &values );

		/// \todo Even if setParameter belongs here, perhaps getParameter belongs on
		/// a FromArnoldConverter?
		static IECore::DataPtr getParameter( AtNode *node, const AtParamEntry *parameter );
		static IECore::DataPtr getParameter( AtNode *node, const AtUserParamEntry *parameter );
		static IECore::DataPtr getParameter( AtNode *node, const char *name );
		static void getParameters( AtNode *node, IECore::CompoundDataMap &values );

		/// Returns the Arnold parameter type (AI_TYPE_INT etc) suitable for
		/// storing Cortex data of the specified type, setting array to true
		/// or false depending on whether or not the Arnold type will be an
		/// array. Returns AI_TYPE_NONE if there is no suitable Arnold type.
		static int parameterType( IECore::TypeId dataType, bool &array );

		static AtArray *dataToArray( const IECore::Data *data );

		/// Creates a suitable ToArnoldConverter operating on the specified object.
		static ToArnoldConverterPtr create( IECore::ObjectPtr object );

	protected:

		ToArnoldConverter( const std::string &description, IECore::TypeId supportedType );

		virtual ~ToArnoldConverter();

		/// Must be implemented by derived classes to perform the conversion. It is guaranteed that the object
		/// is of the supported type and the parameter values have been validated and placed in operands.
		virtual AtNode *doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ConverterDescription
		{
			public :
				ConverterDescription();
			private :
				static ToArnoldConverterPtr creator( IECore::ObjectPtr object );
		};

	private :

		typedef ToArnoldConverterPtr (*CreatorFn)( IECore::ObjectPtr );
		typedef std::map<IECore::TypeId, CreatorFn> CreatorMap;
		static CreatorMap &creators();

		static void setParameterInternal( AtNode *node, const char *name, int parameterType, bool array, const IECore::Data *value );
		static IECore::DataPtr getParameterInternal( AtNode *node, const char *name, int parameterType );

};

} // namespace IECoreArnold

#include "IECoreArnold/ToArnoldConverter.inl"

#endif // IECOREARNOLD_TOARNOLDCONVERTER_H
