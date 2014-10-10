//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ParticleReader.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TestTypedData.h"

#include <algorithm>

using namespace std;
using namespace IECore;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ParticleReader );

ParticleReader::ParticleReader( const std::string &description )
		:	Reader( description, new ObjectParameter( "result", "The loaded object.", new NullObject, PointsPrimitive::staticTypeId() ) )
{
	m_percentageParameter = new FloatParameter(
		"percentage",
		"The percentage of particles to read.",
		100.0f,
		0.0f,
		100.0f
	);

	m_percentageSeedParameter = new IntParameter(
		"percentageSeed",
		"Used to control which particles are loaded when percentage is not 100. Different seeds give \
		different sets of particles.",
		0
	);

	m_attributesParameter = new StringVectorParameter(
		"attributes",
		"A list of attributes to load. If the list is empty then all attributes are loaded."
	);

	IntParameter::PresetsContainer realTypePresets;
	realTypePresets.push_back( IntParameter::Preset( "native", Native ) );
	realTypePresets.push_back( IntParameter::Preset( "float", Float ) );
	realTypePresets.push_back( IntParameter::Preset( "double", Double ) );
	m_realTypeParameter = new IntParameter(
		"realType",
		"The type of data to use to represent real values.",
		Float,
		Native,
		Double,
		realTypePresets,
		true
	);
	
	m_convertPrimVarNamesParameter = new BoolParameter(
		"convertPrimVarNames",
		"Convert the position primVar name to P.",
		true
	);

	parameters()->addParameter( m_percentageParameter );
	parameters()->addParameter( m_percentageSeedParameter );
	parameters()->addParameter( m_attributesParameter );
	parameters()->addParameter( m_realTypeParameter );
	parameters()->addParameter( m_convertPrimVarNamesParameter );
}

FloatParameter * ParticleReader::percentageParameter()
{
	return m_percentageParameter.get();
}

const FloatParameter * ParticleReader::percentageParameter() const
{
	return m_percentageParameter.get();
}

IntParameter * ParticleReader::percentageSeedParameter()
{
	return m_percentageSeedParameter.get();
}

const IntParameter * ParticleReader::percentageSeedParameter() const
{
	return m_percentageSeedParameter.get();
}

StringVectorParameter * ParticleReader::attributesParameter()
{
	return m_attributesParameter.get();
}

const StringVectorParameter * ParticleReader::attributesParameter() const
{
	return m_attributesParameter.get();
}

IntParameter * ParticleReader::realTypeParameter()
{
	return m_realTypeParameter.get();
}

const IntParameter * ParticleReader::realTypeParameter() const
{
	return m_realTypeParameter.get();
}

BoolParameter * ParticleReader::convertPrimVarNamesParameter()
{
	return m_convertPrimVarNamesParameter.get();
}

const BoolParameter * ParticleReader::convertPrimVarNamesParameter() const
{
	return m_convertPrimVarNamesParameter.get();
}

ObjectPtr ParticleReader::doOperation( const CompoundObject * operands )
{
	vector<string> attributes;
	particleAttributes( attributes );
	PointsPrimitivePtr result = new PointsPrimitive( numParticles() );
	// because of percentage filtering we don't really know the number of points until we've loaded an attribute.
	// we start off with numParticles() in case there aren't any varying attributes in the cache at all, but replace it
	// below as soon as we have a revised (percentage filtered) value.
	bool haveNumPoints = false;
	for( vector<string>::const_iterator it = attributes.begin(); it!=attributes.end(); it++ )
	{
		DataPtr d = readAttribute( *it );

		if ( testTypedData<TypeTraits::IsVectorTypedData>( d.get() ) )
		{
			size_t s = despatchTypedData< TypedDataSize, TypeTraits::IsVectorTypedData >( d.get() );
			if( !haveNumPoints )
			{
				result->setNumPoints( s );
				haveNumPoints = true;
			}
			if( s==result->getNumPoints() )
			{
				string primVarName = *it;
				if( convertPrimVarNames() && primVarName == positionPrimVarName() )
				{
					// Current attribute is the position. Use "P" instead.
					primVarName = "P";
				}
				result->variables.insert( PrimitiveVariableMap::value_type( primVarName, PrimitiveVariable( PrimitiveVariable::Vertex, d ) ) );
			}
			else
			{
				msg( Msg::Warning, "ParticleReader::doOperation", format( "Ignoring attribute \"%s\" due to insufficient elements (expected %d but found %d)." ) % *it % result->getNumPoints() % s );
			}
		}
		else if ( testTypedData<TypeTraits::IsSimpleTypedData>( d.get() ) )
		{
			result->variables.insert( PrimitiveVariableMap::value_type( *it, PrimitiveVariable( PrimitiveVariable::Constant, d ) ) );
		}
	}
	return result;
}

float ParticleReader::particlePercentage() const
{
	return m_percentageParameter->getNumericValue();
}

int ParticleReader::particlePercentageSeed() const
{
	return m_percentageSeedParameter->getNumericValue();
}

void ParticleReader::particleAttributes( std::vector<std::string> &names )
{
	vector<string> allNames;
	attributeNames( allNames );

	ConstStringVectorDataPtr d = m_attributesParameter->getTypedValidatedValue<StringVectorData>();
	if( !d->readable().size() )
	{
		names = allNames;
		return;
	}

	names.clear();
	for( vector<string>::const_iterator it = d->readable().begin(); it!=d->readable().end(); it++ )
	{
		if( find( allNames.begin(), allNames.end(), *it )!=allNames.end() )
		{
			names.push_back( *it );
		}
	}
}

ParticleReader::RealType ParticleReader::realType() const
{
	return RealType( m_realTypeParameter->getNumericValue() );
}

bool ParticleReader::convertPrimVarNames() const
{
	return m_convertPrimVarNamesParameter->getTypedValue();
}

