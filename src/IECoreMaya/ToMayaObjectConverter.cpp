//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECoreMaya/ToMayaObjectConverter.h"

using namespace IECoreMaya;
using namespace IECore;

ToMayaObjectConverter::ToMayaObjectConverter(  const std::string &name, const std::string &description, ConstObjectPtr object )
	:	Converter( name, description ), m_object( object )
{
	assert( object );
}

ConstObjectPtr ToMayaObjectConverter::object() const
{
	return m_object;
}
		
bool ToMayaObjectConverter::convert( MObject &object ) const
{
	return doConvert( object );
}

/////////////////////////////////////////////////////////////////////////////////IECore::TypeId
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToMayaObjectConverterPtr ToMayaObjectConverter::create( ConstObjectPtr object )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object->typeId(), MFn::kInvalid ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

ToMayaObjectConverterPtr ToMayaObjectConverter::create( ConstObjectPtr object, MFn::Type resultType )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object->typeId(), resultType ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

void ToMayaObjectConverter::registerConverter( IECore::TypeId fromType, const MFn::Type resultType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType, resultType ), creator ) );
	m->insert( TypesToFnsMap::value_type( Types( fromType, MFn::kInvalid ), creator ) ); // for the create function which doesn't care about resultType
}

ToMayaObjectConverter::TypesToFnsMap *ToMayaObjectConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToMayaObjectConverter::Types::Types( IECore::TypeId from, MFn::Type result )
	:	fromType( from ), resultType( result )
{
}

bool ToMayaObjectConverter::Types::operator < ( const Types &other ) const
{
	if( fromType != other.fromType )
	{
		return fromType < other.fromType;
	}
	else
	{
		return resultType < other.resultType;
	}
}
