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

#ifndef IECOREALEMBIC_OBJECTWRITER_H
#define IECOREALEMBIC_OBJECTWRITER_H

#include "IECoreAlembic/Export.h"

#include "IECore/Object.h"

#include "Alembic/Abc/OObject.h"
#include "Alembic/AbcCoreAbstract/TimeSampling.h"

#include "boost/container/flat_map.hpp"
#include "boost/noncopyable.hpp"

#include <memory>

namespace IECoreAlembic
{

/// Base class for writing IECore::Objects into Alembic files.
/// Derived classes must have a constructor with the following
/// signature :
///
/// `Derived( Abc::OObject &parent, const std::string &name )`
///
/// This should create an appropriately typed child on the parent
/// and store it as member data. The virtual `writeSample()`
/// and `writeTimeSampling()` methods should then be implemented
/// to write to this object.
class ObjectWriter : private boost::noncopyable
{

	public :

		virtual ~ObjectWriter();

		virtual void writeSample( const IECore::Object *object ) = 0;
		virtual void writeTimeSampling( const Alembic::AbcCoreAbstract::TimeSamplingPtr &timeSampling ) = 0;

		/// Factory function. Creates an ObjectWriter for writing objects of the
		/// specified Cortex type. Returns null if no writer is available.
		static std::unique_ptr<ObjectWriter> create( IECore::TypeId cortexType, Alembic::Abc::OObject &parent, const std::string &name );

	protected :

		/// Derived classes should create a static instance of this to register
		/// themselves with the factory mechanism.
		template<typename WriterType>
		struct Description : private boost::noncopyable
		{
			Description( const IECore::TypeId objectType )
			{
				registerWriter(
					objectType,
					[]( Alembic::Abc::OObject &parent, const std::string &name ) {
						return std::unique_ptr<ObjectWriter>( new WriterType( parent, name ) );
					}
				);
			}
		};

	private :

		typedef std::function<std::unique_ptr<ObjectWriter>( Alembic::Abc::OObject &parent, const std::string &name )> Creator;

		typedef boost::container::flat_map<IECore::TypeId, Creator> Creators;
		static Creators &creators();

		static void registerWriter( IECore::TypeId objectType, Creator creator );

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_OBJECTWRITER_H
