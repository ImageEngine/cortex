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

#ifndef IECOREALEMBIC_OBJECTALGO_INL
#define IECOREALEMBIC_OBJECTALGO_INL

#include "boost/bind.hpp"
#include "boost/function.hpp"

namespace IECoreAlembic
{

namespace ObjectAlgo
{

namespace Detail
{

struct Registration
{
	typedef bool (*MatchFn)( const Alembic::AbcCoreAbstract::MetaData &, Alembic::Abc::SchemaInterpMatching );
	typedef boost::function<IECore::ObjectPtr ( const Alembic::Abc::IObject &object, const Alembic::Abc::ISampleSelector &sampleSelector )> Converter;

	IECore::TypeId resultType;
	MatchFn matcher;
	Converter converter;
};

typedef std::vector<Registration> RegistrationVector;
RegistrationVector &registrations();

template<typename AlembicType, typename CortexType, typename Converter>
IECore::ObjectPtr convert( const Alembic::Abc::IObject &object, const Alembic::Abc::ISampleSelector &sampleSelector, Converter converter )
{
	AlembicType typedObject( object, Alembic::Abc::kWrapExisting );
	return converter( typedObject, sampleSelector );
}

} // namespace Detail

/// Registers a converter from AlembicType to CortexType.
template<typename AlembicType, typename CortexType>
ConverterDescription<AlembicType, CortexType>::ConverterDescription( Converter converter )
{
	Detail::Registration r;
	r.resultType = CortexType::staticTypeId();
	r.matcher = AlembicType::matches;
	r.converter = boost::bind( Detail::convert<AlembicType, CortexType, Converter>, ::_1, ::_2, converter );
	Detail::registrations().push_back( r );
}

} // namespace ObjectAlgo

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_OBJECTALGO_INL
