//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/ToNukeGeometryConverter.h"

#include "IECore/CompoundParameter.h"
#include "IECore/CompoundData.h"

#include <cassert>

using namespace IECoreNuke;
using namespace IECore;
using namespace DD::Image;

IE_CORE_DEFINERUNTIMETYPED( ToNukeGeometryConverter );

ToNukeGeometryConverter::ToNukeGeometryConverter( const std::string &description, IECore::TypeId fromType, ConstObjectPtr object )
	:	ToNukeConverter( description, fromType )
{
	srcParameter()->setValue( boost::const_pointer_cast<Object>( object ) );

	m_objIndexParameter = new IntParameter( "objIndex", "Index for the first object inserted on the GeometryList. Use -1 to simply add on the next index available", -1 );
	parameters()->addParameter( m_objIndexParameter );

}

void ToNukeGeometryConverter::convert( GeometryList &geoList ) const
{
	int objIndex = m_objIndexParameter->getNumericValue();
	if ( objIndex == -1 )
	{
		objIndex = (int)geoList.objects();
	}
	geoList.add_object(objIndex);

	ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<CompoundObject>();
	doConversion( srcParameter()->getValidatedValue(), geoList, objIndex, operands.get() );
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToNukeGeometryConverterPtr ToNukeGeometryConverter::create( ConstObjectPtr object )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object->typeId() ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

void ToNukeGeometryConverter::registerConverter( IECore::TypeId fromType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType ), creator ) );
}

ToNukeGeometryConverter::TypesToFnsMap *ToNukeGeometryConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToNukeGeometryConverter::Types::Types( IECore::TypeId from )
	:	fromType( from )
{
}

bool ToNukeGeometryConverter::Types::operator < ( const Types &other ) const
{
	return fromType < other.fromType;
}
