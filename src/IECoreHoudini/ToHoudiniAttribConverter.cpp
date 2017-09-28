//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/ToHoudiniAttribConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniAttribConverter );

ToHoudiniAttribConverter::ToHoudiniAttribConverter( const Data *data, const std::string &description )
	:	ToHoudiniConverter( description, DataTypeId )
{
	srcParameter()->setValue( (Data *)data );
}

ToHoudiniAttribConverter::~ToHoudiniAttribConverter()
{
}

GA_RWAttributeRef ToHoudiniAttribConverter::convert( std::string name, GU_Detail *geo ) const
{
	return doConversion( (const IECore::Data *)srcParameter()->getValidatedValue(), name, geo );
}

GA_RWAttributeRef ToHoudiniAttribConverter::convert( std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	return doConversion( (const IECore::Data *)srcParameter()->getValidatedValue(), name, geo, range );
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniAttribConverterPtr ToHoudiniAttribConverter::create( const Data *data )
{
	const TypesToFnsMap *m = typesToFns();

	TypesToFnsMap::const_iterator it = m->find( Types( data->typeId() ) );
	if( it!=m->end() )
	{
		return it->second( data );
	}

	return 0;
}

void ToHoudiniAttribConverter::registerConverter( IECore::TypeId fromType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType ), creator ) );
}

ToHoudiniAttribConverter::TypesToFnsMap *ToHoudiniAttribConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniAttribConverter::Types::Types( IECore::TypeId from ) : fromType( from )
{
}

bool ToHoudiniAttribConverter::Types::operator < ( const Types &other ) const
{
	return fromType < other.fromType;
}
