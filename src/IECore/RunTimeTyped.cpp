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

#include "IECore/RunTimeTyped.h"

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

std::string RunTimeTyped::typeName() const
{
	return staticTypeName();
}

TypeId RunTimeTyped::staticTypeId()
{
	return RunTimeTypedTypeId;
}

std::string RunTimeTyped::staticTypeName()
{
	return "RunTimeTyped";
}

TypeId RunTimeTyped::baseTypeId()
{
	return InvalidTypeId;
}
		
std::string RunTimeTyped::baseTypeName()
{
	return "InvalidType";
}	

bool RunTimeTyped::isInstanceOf( TypeId typeId ) const
{
	return typeId==staticTypeId();
}

bool RunTimeTyped::isInstanceOf( const std::string &typeName ) const
{
	return typeName==staticTypeName();
}

bool RunTimeTyped::inheritsFrom( TypeId typeId )
{
	return false;
}

bool RunTimeTyped::inheritsFrom( const std::string &typeName )
{
	return false;
}

void RunTimeTyped::registerType( TypeId derivedTypeId, const std::string &derivedTypeName, TypeId baseTypeId )
{
	BaseTypeRegistryMap &baseRegistry = baseTypeRegistry();
#ifndef NDEBUG
	{
		BaseTypeRegistryMap::const_iterator it = baseRegistry.find( derivedTypeId );
		if ( it != baseRegistry.end() )
		{
			assert( it->second == baseTypeId );
		}	
	}
#endif	
	
	baseRegistry[ derivedTypeId ] = baseTypeId;
	
	DerivedTypesRegistryMap &derivedRegistry = derivedTypesRegistry();
	derivedRegistry[ baseTypeId ].insert( derivedTypeId );
	
	/// Put in id->name map
	TypeIdsToTypeNamesMap &idsToNames = typeIdsToTypeNames();
#ifndef NDEBUG
	{
		TypeIdsToTypeNamesMap::const_iterator it = idsToNames.find( derivedTypeId );
		if ( it != idsToNames.end() )
		{
			assert( it->second == derivedTypeName );
		}
	}	
#endif
	idsToNames[ derivedTypeId ] = derivedTypeName;
	
	/// Put in name->id map
	TypeNamesToTypeIdsMap &namesToIds = typeNamesToTypeIds();
#ifndef NDEBUG
	{
		TypeNamesToTypeIdsMap::const_iterator it = namesToIds.find( derivedTypeName );
		if ( it != namesToIds.end() )
		{
			assert( it->second == derivedTypeId );
		}	
	}
#endif
	namesToIds[ derivedTypeName ] = derivedTypeId;
	
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

TypeId RunTimeTyped::typeIdFromTypeName( const std::string &typeName )
{
	TypeNamesToTypeIdsMap &namesToIds = typeNamesToTypeIds();
	TypeNamesToTypeIdsMap::const_iterator it = namesToIds.find( typeName );
	if( it == namesToIds.end() )
	{
		return InvalidTypeId;
	}
	return it->second;
}

std::string RunTimeTyped::typeNameFromTypeId( TypeId typeId )
{
	TypeIdsToTypeNamesMap &idsToNames = typeIdsToTypeNames();
	TypeIdsToTypeNamesMap::const_iterator it = idsToNames.find( typeId );
	if( it == idsToNames.end() )
	{
		return "";
	}
	return it->second;
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
