//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ParticleWriter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/TestTypedData.h"

using namespace std;
using namespace IECore;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ParticleWriter )

ParticleWriter::ParticleWriter( const std::string &name, const std::string &description )
	:	Writer( name, description, PointsPrimitiveTypeId )
{
	m_attributesParameter = new StringVectorParameter(
		"attributes",
		"A list of attributes to write. If the list is empty then all attributes are written."
	);
	parameters()->addParameter( m_attributesParameter );
}

bool ParticleWriter::canWrite( ConstObjectPtr object, const std::string &fileName )
{
	return runTimeCast<const PointsPrimitive>( object );
}

ConstPointsPrimitivePtr ParticleWriter::particleObject()
{
	/// \todo this cast is no longer ok - once we've adjusted the input object Parameter we can use the validation
	/// on there to do our work for us.
	return static_pointer_cast<const PointsPrimitive>( object() );
}

size_t ParticleWriter::particleCount()
{
	return particleObject()->getNumPoints();
}

void ParticleWriter::particleAttributes( std::vector<std::string> &names )
{
	// find all attributes of the particle object with appropriate amounts of data
	size_t numParticles = particleCount();
	vector<string> allNames;
	ConstPointsPrimitivePtr cd = particleObject();
	for( PrimitiveVariableMap::const_iterator it=cd->variables.begin(); it!=cd->variables.end(); it++ )
	{
		if ( testTypedData<TypeTraits::IsVectorTypedData>( it->second.data ) )
		{
			size_t s = despatchTypedData< TypedDataSize, TypeTraits::IsVectorTypedData >( it->second.data );
			if( s==numParticles )
			{
				allNames.push_back( it->first );
			}
			else
			{
				msg( Msg::Warning, "ParticleWriter::particleAttributes", format( "Ignoring attribute \"%s\" due to insufficient elements (expected %d but found %d)." ) % it->first % numParticles % s );
			}
		}
		else if ( testTypedData<TypeTraits::IsSimpleTypedData>( it->second.data ) )
		{
			// it's not data of a vector type but it could be of a simple type
			// in which case it's suitable for saving as a constant particle attribute

			despatchTypedData< TypedDataAddress, TypeTraits::IsSimpleTypedData >( it->second.data );
			allNames.push_back( it->first );
		}
	}

	ConstStringVectorDataPtr d = static_pointer_cast<const StringVectorData>( parameters()->parameter<StringVectorParameter>( "attributes" )->getValue() );
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
		else
		{
			msg( Msg::Warning, "ParticleWriter::particleAttributes", format( "Attribute \"%s\" requested via parameters but is not available." ) % *it );
		}
	}
}
