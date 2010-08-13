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

#include "FromHoudiniNodeConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniNodeConverter );

FromHoudiniNodeConverter::FromHoudiniNodeConverter( const OP_Node *node, const std::string &description )
	: m_handle( node ), FromHoudiniConverter( description )
{
}

FromHoudiniNodeConverter::~FromHoudiniNodeConverter()
{
}

OP_Node *FromHoudiniNodeConverter::node() const
{
	return m_handle.node();
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

FromHoudiniNodeConverterPtr FromHoudiniNodeConverter::create( const OP_Node *node, IECore::TypeId resultType )
{
	const TypesToFnsMap *m = typesToFns();

	TypesToFnsMap::const_iterator it = m->find( Types( node->getOpTypeID(), resultType ) );
	if ( it != m->end() )
	{
		return it->second( node );
	}
	
	return 0;
}

void FromHoudiniNodeConverter::registerConverter( const OP_OpTypeId fromType, IECore::TypeId resultType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType, resultType ), creator ) );
	m->insert( TypesToFnsMap::value_type( Types( fromType, IECore::InvalidTypeId ), creator ) ); // for the create function which doesn't care about resultType
}

FromHoudiniNodeConverter::TypesToFnsMap *FromHoudiniNodeConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

FromHoudiniNodeConverter::Types::Types( OP_OpTypeId from, IECore::TypeId result )
	:	fromType( from ), resultType( result )
{
}

bool FromHoudiniNodeConverter::Types::operator < ( const Types &other ) const
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
