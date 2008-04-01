//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
#include "IECore/TypedDataDespatch.h"

#include <algorithm>

using namespace std;
using namespace IECore;
using namespace boost;

ParticleReader::ParticleReader( const std::string name, const std::string description ) 
		:	Reader( name, description, new ObjectParameter( "result", "The loaded object.", new NullObject, PointsPrimitive::staticTypeId() ) )
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

	IntParameter::PresetsMap realTypePresets;
	realTypePresets["native"] = Native;
	realTypePresets["float"] = Float;
	realTypePresets["double"] = Double;
	m_realTypeParameter = new IntParameter(
		"realType",
		"The type of data to use to represent real values.",
		Native,
		Native,
		Double,
		realTypePresets,
		true
	);
	
	parameters()->addParameter( m_percentageParameter );
	parameters()->addParameter( m_percentageSeedParameter );
	parameters()->addParameter( m_attributesParameter );
	parameters()->addParameter( m_realTypeParameter );
}

FloatParameterPtr ParticleReader::percentageParameter()
{
	return m_percentageParameter;
}

ConstFloatParameterPtr ParticleReader::percentageParameter() const
{
	return m_percentageParameter;
}

IntParameterPtr ParticleReader::percentageSeedParameter()
{
	return m_percentageSeedParameter;
}

ConstIntParameterPtr ParticleReader::percentageSeedParameter() const
{
	return m_percentageSeedParameter;
}

StringVectorParameterPtr ParticleReader::attributesParameter()
{
	return m_attributesParameter;
}

ConstStringVectorParameterPtr ParticleReader::attributesParameter() const
{
	return m_attributesParameter;
}

IntParameterPtr ParticleReader::realTypeParameter()
{
	return m_realTypeParameter;
}

ConstIntParameterPtr ParticleReader::realTypeParameter() const
{
	return m_realTypeParameter;
}

ObjectPtr ParticleReader::doOperation( ConstCompoundObjectPtr operands )
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
		
		/// \todo Remove use of exception handling as means of flow control
		try
		{
			// throws if it's not vector data
			size_t s = despatchTypedData< TypedDataSize, TypeTraits::IsVectorTypedData >( d );
			if( !haveNumPoints )
			{
				result->setNumPoints( s );
				haveNumPoints = true;
			}
			if( s==result->getNumPoints() )
			{			
				result->variables.insert( PrimitiveVariableMap::value_type( *it, PrimitiveVariable( PrimitiveVariable::Vertex, d ) ) );
			}
			else
			{			
				msg( Msg::Warning, "ParticleReader::doOperation", format( "Ignoring attribute \"%s\" due to insufficient elements (expected %d but found %d)." ) % *it % result->getNumPoints() % s );
			}
		}
		catch( ... )
		{
			// not vector data, maybe it's some sort of constant data
			try
			{
				// throws if not simple data
				despatchTypedData< TypedDataAddress, TypeTraits::IsSimpleTypedData >( d );
				result->variables.insert( PrimitiveVariableMap::value_type( *it, PrimitiveVariable( PrimitiveVariable::Constant, d ) ) );
			}
			catch( ... )
			{
				msg( Msg::Warning, "ParticleReader::doOperation", format( "Ignoring attribute \"%s\" due to unsupported type \"%s\"." ) % *it % d->typeName() );
			}
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
