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

#include "boost/python.hpp"

#include "IECore/Group.h"

#include "FromHoudiniGroupConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniGroupConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniGroupConverter> FromHoudiniGroupConverter::m_description( GroupTypeId );

FromHoudiniGroupConverter::FromHoudiniGroupConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::Group." )
{
}

FromHoudiniGroupConverter::FromHoudiniGroupConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::Group." )
{
}

FromHoudiniGroupConverter::~FromHoudiniGroupConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniGroupConverter::canConvert( const GU_Detail *geo )
{
	const GEO_PrimList &primitives = geo->primitives();
	
	// are there multiple primitives?
	size_t numPrims = primitives.entries();
	if ( numPrims < 2 )
	{
		return Admissible;
	}
	
	// are there mixed primitive types?
	unsigned firstPrimId = primitives( 0 )->getPrimitiveId();
	for ( size_t i=1; i < numPrims; i++ )
	{
		if ( primitives( i )->getPrimitiveId() != firstPrimId )
		{
			return Ideal;
		}
	}
	
	// are the primitives split into groups?
	const GB_GroupList &primGroups = geo->primitiveGroups();
	if ( !primGroups.length() || primGroups.head()->entries() == numPrims )
	{
		return Admissible;
	}
	
	return Ideal;
}

ObjectPtr FromHoudiniGroupConverter::doConversion( ConstCompoundObjectPtr operands ) const
{
	GU_DetailHandleAutoReadLock readHandle( handle() );
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return 0;
	}
	
	size_t numResultPrims = 0;
	size_t numOrigPrims = geo->primitives().entries();
	
	GroupPtr result = new Group();
	
	const GB_GroupList &primGroups = geo->primitiveGroups();
	for ( GB_Group *group=primGroups.head(); group; group = group->next() )
	{
		if ( group->getInternal() || group->isEmpty() )
		{
			continue;
		}
		
		VisibleRenderablePtr renderable = 0;
		numResultPrims += doGroupConversion( geo, (GB_PrimitiveGroup*)group, renderable );
		if( !renderable )
		{
			continue;
		}
		
		renderable->blindData()->writable()["name"] = new StringData( group->getName().toStdString() );
		result->addChild( renderable );
	}
	
	if ( numOrigPrims == numResultPrims )
	{
		return result;
	}
	
	GU_Detail ungroupedGeo( (GU_Detail*)geo );
	GB_PrimitiveGroup *ungrouped = ungroupedGeo.newPrimitiveGroup( "FromHoudiniGroupConverter__ungroupedPrimitives" );

	const GEO_PrimList &primitives = ungroupedGeo.primitives();
	size_t numPrims = primitives.entries();
	for ( size_t i=0; i < numPrims; i++ )
 	{
		GEO_Primitive *prim = (GEO_Primitive*)primitives( i );
		if ( !prim->memberOfAnyGroup() )
		{
			ungrouped->add( prim );
		}
	}

	if ( ungrouped->isEmpty() )
	{
		return result;
	}
	
	VisibleRenderablePtr renderable = 0;
	doGroupConversion( &ungroupedGeo, ungrouped, renderable );
	if ( renderable )
	{
		result->addChild( renderable );
	}

	return result;
}

size_t FromHoudiniGroupConverter::doGroupConversion( const GU_Detail *geo, GB_PrimitiveGroup *group, VisibleRenderablePtr &result ) const
{
	GU_Detail groupGeo( (GU_Detail*)geo, group );
	if ( !groupGeo.points().entries() )
	{
		return 0;
	}
	
	if ( groupGeo.primitives().entries() < 2 )
	{
		result = doPrimitiveConversion( &groupGeo );
		return groupGeo.primitives().entries();
	}
	
	PrimIdGroupMap groupMap;
	groupGeo.removeUnusedPrimGroups();
	size_t numNewGroups = regroup( &groupGeo, groupMap );
	
	if ( numNewGroups < 2 )
	{
		result = doPrimitiveConversion( &groupGeo );
		return groupGeo.primitives().entries();
	}

	GroupPtr groupResult = new Group();
	
	for ( PrimIdGroupMapIterator it = groupMap.begin(); it != groupMap.end(); it++ )
	{
		VisibleRenderablePtr renderable = 0;
		GU_Detail childGeo( &groupGeo, it->second );
		PrimitivePtr child = doPrimitiveConversion( &childGeo );
		if ( child )
		{
			groupResult->addChild( child );
		}
	}

	result = groupResult;
	return groupGeo.primitives().entries();
}

size_t FromHoudiniGroupConverter::regroup( GU_Detail *geo, PrimIdGroupMap &groupMap ) const
{
	PrimIdGroupMapIterator it;
	const GEO_PrimList &primitives = geo->primitives();
	size_t numPrims = primitives.entries();
	for ( size_t i=0; i < numPrims; i++ )
 	{
		GEO_Primitive *prim = (GEO_Primitive*)primitives( i );
		unsigned primType = prim->getPrimitiveId();
		it = groupMap.find( primType );
		if ( it == groupMap.end() )
		{
			PrimIdGroupPair pair( primType, geo->newPrimitiveGroup( ( boost::format( "FromHoudiniGroupConverter__typedPrimitives%d" ) % primType ).str().c_str() ) );
			it = groupMap.insert( pair ).first;
		}
		
		it->second->add( prim );
	}
	
	return groupMap.size();
}

PrimitivePtr FromHoudiniGroupConverter::doPrimitiveConversion( const GU_Detail *geo ) const
{
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );

	FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( handle );
	if ( !converter || converter->isInstanceOf( FromHoudiniGroupConverter::staticTypeId() ) )
	{
		return 0;
	}
	
	return IECore::runTimeCast<Primitive>( converter->convert() );
}
