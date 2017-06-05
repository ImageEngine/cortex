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

#include "IECore/SimpleTypedData.h"
#include "IECore/CompoundParameter.h"

#include "IECoreAlembic/ToAlembicConverter.h"

using namespace Alembic::Abc;
using namespace IECoreAlembic;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ToAlembicConverter );

ToAlembicConverter::ToAlembicConverter( const std::string &description, IECore::TypeId supportedType, Alembic::Abc::OObject transform )
	:	IECore::FromCoreConverter( description, supportedType ), m_supportedType( supportedType ), m_transform( transform )
{
	m_timeParameter = new FloatParameter( "time", "Time at which to write the converted object.", 0.0f );
	parameters()->addParameter( m_timeParameter );
}

ToAlembicConverter::~ToAlembicConverter()
{
}

FloatParameterPtr ToAlembicConverter::timeParameter()
{
	return m_timeParameter;
}

ConstFloatParameterPtr ToAlembicConverter::timeParameter() const
{
	return m_timeParameter;
}

IECore::TypeId ToAlembicConverter::supportedType()
{
	return m_supportedType;
}

void ToAlembicConverter::convert()
{
	double time = timeParameter()->getNumericValue();
	if( m_sampleTimes.size() && time <= m_sampleTimes.back() )
	{
		throw IECore::Exception( "ToAlembicConverter::convertAtTime: object sample times must be strictly increasing" );
	}

	if( srcParameter()->getValue()->typeId() != m_supportedType )
	{
		throw IECore::Exception( "ToAlembicConverter::convertAtTime: object is not of the supported type" );
	}

	// create underlying alembic object if it exists, and potentially perform validity checks on
	// the contents of srcParameter():
	ensureAlembicObject( m_transform );

	m_sampleTimes.push_back( time );
	TimeSamplingPtr tSamp( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), m_sampleTimes ) );
	updateTimeSampling( tSamp );

	writeAlembicObject();
}

ToAlembicConverterPtr ToAlembicConverter::create( IECore::TypeId typeId, Alembic::Abc::OObject object )
{
	const CreatorMap &m = creators();
	CreatorMap::const_iterator it = m.find( typeId );
	if( it != m.end() )
	{
		return it->second( object );
	}
	return 0;
}

ToAlembicConverter::CreatorMap &ToAlembicConverter::creators()
{
	static CreatorMap m;
	return m;
}
