//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECOREALEMBIC_FROMALEMBICCONVERTER_H
#define IECOREALEMBIC_FROMALEMBICCONVERTER_H

#include "Alembic/Abc/IObject.h"
#include "Alembic/Abc/ISampleSelector.h"

#include "IECore/ToCoreConverter.h"
#include "IECore/NumericParameter.h"

#include "IECoreAlembic/TypeIds.h"
#include "IECoreAlembic/Export.h"

namespace IECoreAlembic
{

IE_CORE_FORWARDDECLARE( FromAlembicConverter )

class IECOREALEMBIC_API FromAlembicConverter : public IECore::ToCoreConverter
{

	public :

		/// These typedefs describe the input and output types of the
		/// conversion supported by this class. Derived classes /must/
		/// override these typedefs to more accurately describe their
		/// particular conversion in detail - they will be used by
		/// the ConverterDescription to register the converter with
		/// the factory mechanism.
		typedef Alembic::Abc::IObject InputType;
		typedef IECore::Object ResultType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromAlembicConverter, FromAlembicConverterTypeId, ToCoreConverter );

		IECore::IntParameter *sampleIndexParameter();
		const IECore::IntParameter *sampleIndexParameter() const;

		//! @name Factory
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given Alembic object to an IECore::Object.
		/// If resultType is specified then only converters which create objects of that
		/// type will be returned - the default value allows any suitable converter to be
		/// created. If no matching converters exist then returns 0.
		static FromAlembicConverterPtr create( Alembic::Abc::IObject object, IECore::TypeId resultType=IECore::ObjectTypeId );
		//@}

	protected :

		FromAlembicConverter( const std::string &description, Alembic::Abc::IObject iObject );

		/// Implemented to call doAlembicConversion. Derived classes should implement doAlembicConversion()
		/// instead.
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;		
		virtual IECore::ObjectPtr doAlembicConversion( const Alembic::Abc::IObject &iObject, const Alembic::Abc::ISampleSelector &sampleSelector, const IECore::CompoundObject *operands ) const = 0;		
		
		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ConverterDescription
		{
			public :			
				ConverterDescription();
			private :
				static FromAlembicConverterPtr creator( Alembic::Abc::IObject &iObject );
		};

	private :

		/// Definition of a function which returns true if a converter is compatible
		/// with a given alembic object.
		typedef bool (*MatchFn)( const Alembic::AbcCoreAbstract::MetaData &, Alembic::Abc::SchemaInterpMatching );
		/// Definition of a function which returns a converter given an alembic object.
		typedef FromAlembicConverterPtr (*CreatorFn)( Alembic::Abc::IObject &object );

		struct Registration
		{
			IECore::TypeId resultType;
			MatchFn matcher;
			CreatorFn creator;
		};

		typedef std::vector<Registration> RegistrationVector;
		static RegistrationVector &registrations();

		Alembic::Abc::IObject m_iObject;
		
};

} // namespace IECoreAlembic

#include "IECoreAlembic/FromAlembicConverter.inl"

#endif // IECOREALEMBIC_FROMALEMBICCONVERTER_H
