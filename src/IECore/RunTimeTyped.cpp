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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/RunTimeTyped.h"
#include "IECore/MessageHandler.h"

using namespace IECore;

RunTimeTyped::RunTimeTyped()
{
}

RunTimeTyped::~RunTimeTyped()
{
}

TypeId RunTimeTyped::typeId() const
{
	return staticTypeId();
}

const char *RunTimeTyped::typeName() const
{
	return staticTypeName();
}

TypeId RunTimeTyped::staticTypeId()
{
	return RunTimeTypedTypeId;
}

const char *RunTimeTyped::staticTypeName()
{
	return "RunTimeTyped";
}

TypeId RunTimeTyped::baseTypeId()
{
	return InvalidTypeId;
}

const char *RunTimeTyped::baseTypeName()
{
	return "InvalidType";
}

bool RunTimeTyped::isInstanceOf( TypeId typeId ) const
{
	return typeId==staticTypeId();
}

bool RunTimeTyped::isInstanceOf( const char *typeName ) const
{
	assert( typeName );
	return !strcmp( typeName, staticTypeName() );
}

bool RunTimeTyped::inheritsFrom( TypeId typeId )
{
	return false;
}

bool RunTimeTyped::inheritsFrom( const char *typeName )
{
	assert( typeName );
	return false;
}

void RunTimeTyped::registerType( TypeId derivedTypeId, const char *derivedTypeName, TypeId baseTypeId )
{
	assert( derivedTypeName );

	{
		BaseTypeRegistryMap &baseRegistry = baseTypeRegistry();
		BaseTypeRegistryMap::iterator lb = baseRegistry.lower_bound( derivedTypeId );
		if ( lb != baseRegistry.end() && derivedTypeId == lb->first )
		{
			if ( baseTypeId != lb->second )
			{
				msg( Msg::Warning, "RunTimeTyped", boost::format( "Duplicate registration of base type id for '%s' - %d and %d") % derivedTypeName % lb->second % baseTypeId  );
			}
		}
		else
		{
			/// Use the lower-bound as a hint for the position, yielding constant insert time
			baseRegistry.insert( lb, BaseTypeRegistryMap::value_type( derivedTypeId, baseTypeId ) );
		}
	}

	/// Inserted derived type id into set of base classes derived type ids
	DerivedTypesRegistryMap &derivedRegistry = derivedTypesRegistry();
	derivedRegistry[ baseTypeId ].insert( derivedTypeId );

	/// Put in id->name map
	{
		TypeIdsToTypeNamesMap &idsToNames = typeIdsToTypeNames();
		TypeIdsToTypeNamesMap::iterator lb = idsToNames.lower_bound( derivedTypeId );
		if ( lb != idsToNames.end() && derivedTypeId == lb->first )
		{
			if ( std::string( derivedTypeName ) != lb->second )
			{
				msg( Msg::Warning, "RunTimeTyped", boost::format( "Duplicate registration of type name for type id %d - '%s' and '%s'" ) % derivedTypeId % lb->second % derivedTypeName );
			}
		}
		else
		{
			/// Use the lower-bound as a hint for the position, yielding constant insert time
			idsToNames.insert( lb, TypeIdsToTypeNamesMap::value_type( derivedTypeId, derivedTypeName ) );
			assert( !strcmp( typeNameFromTypeId( derivedTypeId ), derivedTypeName ) );
		}
	}

	/// Put in name->id map
	{
		TypeNamesToTypeIdsMap &namesToIds = typeNamesToTypeIds();
		TypeNamesToTypeIdsMap::iterator lb = namesToIds.lower_bound( derivedTypeName );
		if ( lb != namesToIds.end() && derivedTypeName == lb->first )
		{
			if ( derivedTypeId != lb->second )
			{
				msg( Msg::Warning, "RunTimeTyped", boost::format( "Duplicate registration of type id for type name '%s' - %d and %d") % derivedTypeName % lb->second % derivedTypeId );
			}
		}
		else
		{
			/// Use the lower-bound as a hint for the position, yielding constant insert time
			namesToIds.insert( lb, TypeNamesToTypeIdsMap::value_type( derivedTypeName, derivedTypeId ) );
			assert( typeIdFromTypeName( derivedTypeName ) == derivedTypeId );
		}
	}
}

RunTimeTyped::BaseTypeRegistryMap &RunTimeTyped::baseTypeRegistry()
{
	static BaseTypeRegistryMap *registry = new BaseTypeRegistryMap();

	assert( registry );
	return *registry;
}

RunTimeTyped::DerivedTypesRegistryMap &RunTimeTyped::derivedTypesRegistry()
{
	static DerivedTypesRegistryMap *registry = new DerivedTypesRegistryMap();

	assert( registry );
	return *registry;
}

TypeId RunTimeTyped::baseTypeId( TypeId typeId )
{
	BaseTypeRegistryMap &baseRegistry = baseTypeRegistry();
	BaseTypeRegistryMap::const_iterator it = baseRegistry.find( typeId );

	if ( it == baseRegistry.end() )
	{
		return InvalidTypeId;
	}
	else
	{
		return it->second;
	}
}

const std::vector<TypeId> &RunTimeTyped::baseTypeIds( TypeId typeId )
{
	BaseTypesRegistryMap &baseTypes = completeBaseTypesRegistry();

	BaseTypesRegistryMap::iterator it = baseTypes.find( typeId );
	if ( it != baseTypes.end() )
	{
		return it->second;
	}

	baseTypes.insert( BaseTypesRegistryMap::value_type( typeId, std::vector<TypeId>() ) );
	it = baseTypes.find( typeId );
	assert( it != baseTypes.end() );

	TypeId baseType = baseTypeId( typeId );
	while ( baseType != InvalidTypeId )
	{
		it->second.push_back( baseType );
		baseType = baseTypeId( baseType );
	}

	return it->second;
}

const std::set<TypeId> &RunTimeTyped::derivedTypeIds( TypeId typeId )
{
	DerivedTypesRegistryMap &derivedTypes = completeDerivedTypesRegistry();
	DerivedTypesRegistryMap::iterator it = derivedTypes.find( typeId );

	if ( it == derivedTypes.end() )
	{
		derivedTypes.insert( DerivedTypesRegistryMap::value_type( typeId, std::set<TypeId>() ) );
		it = derivedTypes.find( typeId );
		assert( it != derivedTypes.end() );

		// Walk over the hierarchy of derived types
		derivedTypeIdsWalk( typeId, it->second );
	}

	return it->second;
}

void RunTimeTyped::derivedTypeIdsWalk( TypeId typeId, std::set<TypeId> &typeIds )
{
	DerivedTypesRegistryMap &derivedRegistry = derivedTypesRegistry();
	DerivedTypesRegistryMap::const_iterator it = derivedRegistry.find( typeId );
	if ( it == derivedRegistry.end() )
	{
		/// Termination condition: No derived types
		return;
	}

	for ( std::set<TypeId>::const_iterator typesIt = it->second.begin(); typesIt != it->second.end(); ++ typesIt )
	{
		typeIds.insert( *typesIt );

		// Recurse down into derived types
		derivedTypeIdsWalk( *typesIt, typeIds );
	}
}

RunTimeTyped::BaseTypesRegistryMap &RunTimeTyped::completeBaseTypesRegistry()
{
	static BaseTypesRegistryMap *baseTypes = new BaseTypesRegistryMap();
	assert( baseTypes );
	return *baseTypes;
}

RunTimeTyped::DerivedTypesRegistryMap &RunTimeTyped::completeDerivedTypesRegistry()
{
	static DerivedTypesRegistryMap *derivedTypes = new DerivedTypesRegistryMap();
	assert( derivedTypes );
	return *derivedTypes;
}

TypeId RunTimeTyped::typeIdFromTypeName( const char *typeName )
{
	assert( typeName );

	TypeNamesToTypeIdsMap &namesToIds = typeNamesToTypeIds();
	const std::string key( typeName );
	TypeNamesToTypeIdsMap::const_iterator it = namesToIds.find( key );
	if( it == namesToIds.end() )
	{
		return InvalidTypeId;
	}
	return it->second;
}

const char *RunTimeTyped::typeNameFromTypeId( TypeId typeId )
{
	TypeIdsToTypeNamesMap &idsToNames = typeIdsToTypeNames();
	TypeIdsToTypeNamesMap::const_iterator it = idsToNames.find( typeId );
	if( it == idsToNames.end() )
	{
		return "";
	}
	return it->second.c_str();
}

RunTimeTyped::TypeIdsToTypeNamesMap &RunTimeTyped::typeIdsToTypeNames()
{
	static TypeIdsToTypeNamesMap *m = new TypeIdsToTypeNamesMap();
	assert( m );
	return *m;
}

RunTimeTyped::TypeNamesToTypeIdsMap &RunTimeTyped::typeNamesToTypeIds()
{
	static TypeNamesToTypeIdsMap *m = new TypeNamesToTypeIdsMap();
	assert( m );
	return *m;
}
