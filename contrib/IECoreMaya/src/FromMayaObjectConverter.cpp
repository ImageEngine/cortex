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

#include "IECoreMaya/FromMayaObjectConverter.h"

#include "IECore/CompoundObject.h"

using namespace IECoreMaya;
using namespace IECore;

FromMayaObjectConverter::FromMayaObjectConverter( const std::string &name, const std::string &description, const MObject &object )
	:	FromMayaConverter( name, description ), m_objectHandle( object )
{
}

const MObject &FromMayaObjectConverter::object() const
{
	if( objectIsAlive() )
	{
		return m_objectHandle.objectRef();
	}
	return MObject::kNullObj;
}
		
bool FromMayaObjectConverter::objectIsAlive() const
{
	return m_objectHandle.isAlive();
}

IECore::ObjectPtr FromMayaObjectConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	if( !objectIsAlive() )
	{
		return 0;
	}
	return doConversion( object(), operands );
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

FromMayaObjectConverterPtr FromMayaObjectConverter::create( const MObject &object )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object.apiType(), IECore::InvalidTypeId ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

FromMayaObjectConverterPtr FromMayaObjectConverter::create( const MObject &object, IECore::TypeId resultType )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object.apiType(), resultType ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

void FromMayaObjectConverter::registerConverter( const MFn::Type fromType, IECore::TypeId resultType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType, resultType ), creator ) );
	m->insert( TypesToFnsMap::value_type( Types( fromType, IECore::InvalidTypeId ), creator ) ); // for the create function which doesn't care about resultType
}

FromMayaObjectConverter::TypesToFnsMap *FromMayaObjectConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

FromMayaObjectConverter::Types::Types( MFn::Type from, IECore::TypeId result )
	:	fromType( from ), resultType( result )
{
}

bool FromMayaObjectConverter::Types::operator < ( const Types &other ) const
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
