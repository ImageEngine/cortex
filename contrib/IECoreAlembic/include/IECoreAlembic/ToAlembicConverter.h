//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREALEMBIC_TOALEMBICCONVERTER_H
#define IECOREALEMBIC_TOALEMBICCONVERTER_H

#include "Alembic/Abc/OObject.h"

#include "IECore/FromCoreConverter.h"
#include "IECore/NumericParameter.h"

#include "IECoreAlembic/TypeIds.h"
#include "IECoreAlembic/Export.h"

namespace IECoreAlembic
{

IE_CORE_FORWARDDECLARE( ToAlembicConverter )

class IECOREALEMBIC_API ToAlembicConverter : public IECore::FromCoreConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToAlembicConverter, ToAlembicConverterTypeId, FromCoreConverter );

		//! @name Factory
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter for the given IECore object type, associated with the
		/// given alembic transform.
		static ToAlembicConverterPtr create( IECore::TypeId supportedType, Alembic::Abc::OObject transform );
		//@}

		/// Writes the object in srcParameter() as a sample at the time specified by timeParameter().
		/// Will be written as a child of the transform this converter is associated with. The object
		/// must have the same type id as the supportedType:
		void convert();

		/// supported type of this converter:
		IECore::TypeId supportedType();

		/// The parameter holding the time at which the object should be converted.
		IECore::FloatParameterPtr timeParameter();
		IECore::ConstFloatParameterPtr timeParameter() const;

	protected :

		ToAlembicConverter( const std::string &description, IECore::TypeId supportedType, Alembic::Abc::OObject transform );

		virtual ~ToAlembicConverter();

		/// Derived classes will contain the actual alembic object - eg OPolyMesh, OCamera etc.
		/// This method must be implemented to create the object as a child of transform
		/// if it doesn't exist, and potentially perform validity checks on the contents of
		/// srcParameter().
		virtual void ensureAlembicObject( Alembic::Abc::OObject &transform ) = 0;

		/// Must be implemented by derived classes to perform the conversion. It is guaranteed that the object
		/// in srcParameter() is of the supported type.
		virtual void writeAlembicObject() = 0;

		/// Must be implemented by derived classes to update time sampling on the object.
		virtual void updateTimeSampling( Alembic::Abc::TimeSamplingPtr timeSampling ) = 0;

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class ConverterDescription
		{
			public :			
				ConverterDescription();
			private :
				static ToAlembicConverterPtr creator( Alembic::Abc::OObject object );
		};

	private :

		typedef ToAlembicConverterPtr (*CreatorFn)( Alembic::Abc::OObject object );
		typedef std::map<IECore::TypeId, CreatorFn> CreatorMap;
		static CreatorMap &creators();

		IECore::TypeId m_supportedType;
		std::vector<Alembic::Abc::chrono_t> m_sampleTimes;
		Alembic::Abc::OObject m_transform;
		IECore::FloatParameterPtr m_timeParameter;

};

} // namespace IECoreAlembic

#include "IECoreAlembic/ToAlembicConverter.inl"

#endif // IECOREALEMBIC_TOALEMBICCONVERTER_H
