//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREALEMBIC_OBJECTREADER_H
#define IECOREALEMBIC_OBJECTREADER_H

#include "IECoreAlembic/Export.h"

#include "IECore/Canceller.h"
#include "IECore/Object.h"

#include "Alembic/Abc/IObject.h"
#include "Alembic/Abc/ISampleSelector.h"
#include "Alembic/Abc/ITypedScalarProperty.h"
#include "Alembic/AbcCoreAbstract/TimeSampling.h"

#include "boost/noncopyable.hpp"

#include <memory>
#include <vector>

namespace IECoreAlembic
{

/// Base class for reading IECore::Objects from Alembic files.
class ObjectReader : private boost::noncopyable
{

	public :

		virtual ~ObjectReader();

		virtual const Alembic::Abc::IObject &object() const = 0;
		virtual Alembic::Abc::IBox3dProperty readBoundProperty() const = 0;
		virtual size_t readNumSamples() const = 0;
		virtual Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const = 0;
		virtual IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector, const IECore::Canceller *canceller = nullptr ) const = 0;

		/// Factory function. Creates an ObjectReader for reading the specified
		/// IObject and converting it to the specified cortex type. Returns null
		/// if no reader is available.
		static std::unique_ptr<ObjectReader> create( const Alembic::Abc::IObject &object, IECore::TypeId cortexType = IECore::InvalidTypeId );

	protected :

		/// Derived classes should create a static instance of this to register
		/// themselves with the factory mechanism.
		template<typename ReaderType, typename AlembicType>
		struct Description : private boost::noncopyable
		{
			Description( const IECore::TypeId resultType )
			{
				registerReader(
					AlembicType::matches,
					resultType,
					[]( const Alembic::Abc::IObject &object ) {
						return std::unique_ptr<ObjectReader>(
							new ReaderType( AlembicType( object, Alembic::Abc::kWrapExisting ) )
						);
					}
				);
			}
		};

	private :

		typedef bool (*MatchFn)( const Alembic::AbcCoreAbstract::MetaData &, Alembic::Abc::SchemaInterpMatching );
		typedef std::function<std::unique_ptr<ObjectReader>( const Alembic::Abc::IObject &object )> Creator;

		static void registerReader( MatchFn matchFn, IECore::TypeId resultType, Creator creator );

		struct Registration;
		typedef std::vector<Registration> Registrations;
		static Registrations &registrations();

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_OBJECTREADER_H
