//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECOREALEMBIC_OBJECTALGO_H
#define IECOREALEMBIC_OBJECTALGO_H

#include "boost/noncopyable.hpp"

#include "Alembic/Abc/IObject.h"
#include "Alembic/Abc/ISampleSelector.h"
#include "Alembic/AbcCoreAbstract/TimeSampling.h"

#include "IECore/Object.h"

#include "IECoreAlembic/Export.h"

namespace IECoreAlembic
{

namespace ObjectAlgo
{

/// Converts the specified Alembic object into an equivalent Cortex object,
/// returning NULL if no converter is available.
IECOREALEMBIC_API IECore::ObjectPtr convert( const Alembic::Abc::IObject &object, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::TypeId resultType = IECore::InvalidTypeId );

/// Returns the TimeSampling for an object, and updates numSamples with the number of samples it contains.
IECOREALEMBIC_API Alembic::AbcCoreAbstract::TimeSamplingPtr timeSampling( const Alembic::Abc::IObject &object, size_t &numSamples );

/// Registers a converter from AlembicType to CortexType.
template<typename AlembicType, typename CortexType>
class ConverterDescription : public boost::noncopyable
{

	public :

		/// Type-safe conversion function
		typedef typename CortexType::Ptr (*Converter)( const AlembicType &object, const Alembic::Abc::ISampleSelector &sampleSelector );
		ConverterDescription( Converter converter );

};

} // namespace ObjectAlgo

} // namespace IECoreAlembic

#include "IECoreAlembic/ObjectAlgo.inl"

#endif // IECOREALEMBIC_OBJECTALGO_H
