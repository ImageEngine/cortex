//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/Group.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

#include "IECoreHoudini/FromHoudiniGroupConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniGroupConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniGroupConverter> FromHoudiniGroupConverter::m_description( GroupTypeId );

FromHoudiniGroupConverter::FromHoudiniGroupConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::Group." )
{
	constructCommon();
}

FromHoudiniGroupConverter::FromHoudiniGroupConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::Group." )
{
	constructCommon();
}

FromHoudiniGroupConverter::~FromHoudiniGroupConverter()
{
}

void FromHoudiniGroupConverter::constructCommon()
{
	IntParameter::PresetsContainer groupingModePresets;
	groupingModePresets.push_back( IntParameter::Preset( "PrimitiveGroup", PrimitiveGroup ) );
	groupingModePresets.push_back( IntParameter::Preset( "NameAttribute", NameAttribute ) );
	
	IntParameterPtr groupingMode = new IntParameter(
		"groupingMode",
		"The mode used to separate Primitives during conversion",
		NameAttribute,
		PrimitiveGroup,
		NameAttribute,
		groupingModePresets,
		true
	);
	
	parameters()->addParameter( groupingMode );
}

FromHoudiniGeometryConverter::Convertability FromHoudiniGroupConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	
	// are there multiple primitives?
	unsigned numPrims = geo->getNumPrimitives();
	if ( numPrims < 2 )
	{
		return Admissible;
	}
	
	// are there mixed primitive types?
	GA_Iterator firstPrim = geo->getPrimitiveRange().begin();
	GA_PrimitiveTypeId firstPrimId = primitives.get( firstPrim.getOffset() )->getTypeId();
	for ( GA_Iterator it=firstPrim; !it.atEnd(); ++it )
	{
		if ( primitives.get( it.getOffset() )->getTypeId() != firstPrimId )
		{
			return Ideal;
		}
	}
	
	// are there multiple named shapes?
	const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
	if ( attrHandle.isAttributeValid() )
	{
		const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
		if ( geo->getUniqueValueCount( attrRef ) > 1 )
		{
			return Ideal;
		}
	}
	
	// are the primitives split into groups?
	UT_PtrArray<const GA_ElementGroup*> primGroups;
	geo->getElementGroupList( GA_ATTRIB_PRIMITIVE, primGroups );
	if ( primGroups.isEmpty() )
	{
		return Admissible;
	}
	
	bool externalGroups = false;
	for ( unsigned i=0; i < primGroups.entries(); ++i )
	{
		const GA_ElementGroup *group = primGroups[i];
		if ( group->getInternal() )
		{
			continue;
		}
		
		if ( group->entries() == numPrims )
		{
			return Admissible;
		}
		
		externalGroups = true;
	}
	
	if ( externalGroups )
	{
		return Ideal;
	}
	
	return Admissible;
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
	size_t numOrigPrims = geo->getNumPrimitives();
	
	GroupPtr result = new Group();
	
	if ( operands->member<const IntData>( "groupingMode" )->readable() == NameAttribute )
	{
		GA_ROAttributeRef attributeRef = geo->findPrimitiveAttribute( "name" );
		if ( attributeRef.isInvalid() || !attributeRef.isString() )
		{
			GU_Detail ungroupedGeo( (GU_Detail*)geo );
			GA_PrimitiveGroup *ungrouped = static_cast<GA_PrimitiveGroup*>( ungroupedGeo.createInternalElementGroup( GA_ATTRIB_PRIMITIVE, "FromHoudiniGroupConverter__ungroupedPrimitives" ) );
			ungrouped->toggleRange( ungroupedGeo.getPrimitiveRange() );
			
			VisibleRenderablePtr renderable = 0;
			doGroupConversion( &ungroupedGeo, ungrouped, renderable, operands );
			if ( renderable )
			{
				Group *group = runTimeCast<Group>( renderable );
				if ( group )
				{
					const Group::ChildContainer &children = group->children();
					for ( Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
					{
						result->addChild( *it );
					}
				}
				else
				{
					result->addChild( renderable );
				}
			}
			
			return result;
		}
		
		GU_Detail groupGeo( (GU_Detail*)geo );
		
		AttributePrimIdGroupMap groupMap;
		regroup( &groupGeo, groupMap, attributeRef );
		
		for ( AttributePrimIdGroupMapIterator it=groupMap.begin(); it != groupMap.end(); ++it )
		{
			convertAndAddPrimitive( &groupGeo, it->second, result, operands, it->first.first );
		}
	}
	else
	{
		for ( GA_GroupTable::iterator<GA_ElementGroup> it=geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
		{
			GA_PrimitiveGroup *group = static_cast<GA_PrimitiveGroup*>( it.group() );
			if ( group->getInternal() || group->isEmpty() )
			{
				continue;
			}

			VisibleRenderablePtr renderable = 0;
			numResultPrims += doGroupConversion( geo, group, renderable, operands );
			if( !renderable )
			{
				continue;
			}
			
			renderable->blindData()->member<StringData>( "name", false, true )->writable() = group->getName().toStdString();
			result->addChild( renderable );
		}

		if ( numOrigPrims == numResultPrims )
		{
			return result;
		}

		GU_Detail ungroupedGeo( (GU_Detail*)geo );
		GA_PrimitiveGroup *ungrouped = static_cast<GA_PrimitiveGroup*>( ungroupedGeo.createInternalElementGroup( GA_ATTRIB_PRIMITIVE, "FromHoudiniGroupConverter__ungroupedPrimitives" ) );
		for ( GA_GroupTable::iterator<GA_ElementGroup> it=geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
		{
			*ungrouped |= *static_cast<GA_PrimitiveGroup*>( it.group() );
		}
		ungrouped->toggleRange( ungroupedGeo.getPrimitiveRange() );

		if ( ungrouped->isEmpty() )
		{
			return result;
		}

		VisibleRenderablePtr renderable = 0;
		doGroupConversion( &ungroupedGeo, ungrouped, renderable, operands );
		if ( renderable )
		{
			result->addChild( renderable );
		}
	}
	
	return result;
}

size_t FromHoudiniGroupConverter::doGroupConversion( const GU_Detail *geo, GA_PrimitiveGroup *group, VisibleRenderablePtr &result, const CompoundObject *operands ) const
{
	GU_Detail groupGeo( (GU_Detail*)geo, group );
	if ( !groupGeo.getNumPoints() )
	{
		return 0;
	}
	
	size_t numPrims = groupGeo.getNumPrimitives();
	if ( numPrims < 2 )
	{
		result = doPrimitiveConversion( &groupGeo, operands );
		return numPrims;
	}
	
	PrimIdGroupMap groupMap;
	groupGeo.destroyEmptyGroups( GA_ATTRIB_PRIMITIVE );
	size_t numNewGroups = regroup( &groupGeo, groupMap );
	
	if ( numNewGroups < 2 )
	{
		result = doPrimitiveConversion( &groupGeo, operands );
		return numPrims;
	}

	GroupPtr groupResult = new Group();
	for ( PrimIdGroupMapIterator it = groupMap.begin(); it != groupMap.end(); it++ )
	{
		convertAndAddPrimitive( &groupGeo, it->second, groupResult, operands );
	}

	result = groupResult;
	return numPrims;
}

size_t FromHoudiniGroupConverter::regroup( GU_Detail *geo, PrimIdGroupMap &groupMap ) const
{
	PrimIdGroupMapIterator it;
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	for ( GA_Iterator pIt=geo->getPrimitiveRange().begin(); !pIt.atEnd(); ++pIt )
	{
		GA_Primitive *prim = primitives.get( pIt.getOffset() );
		unsigned primType = prim->getTypeId().get();
		it = groupMap.find( primType );
		if ( it == groupMap.end() )
		{
			PrimIdGroupPair pair( primType, static_cast<GA_PrimitiveGroup*>( geo->createInternalElementGroup( GA_ATTRIB_PRIMITIVE, ( boost::format( "FromHoudiniGroupConverter__typedPrimitives%d" ) % primType ).str().c_str() ) ) );
			it = groupMap.insert( pair ).first;
		}
		
		it->second->add( prim );
	}
	
	return groupMap.size();
}

size_t FromHoudiniGroupConverter::regroup( GU_Detail *geo, AttributePrimIdGroupMap &groupMap, GA_ROAttributeRef attrRef ) const
{
	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFStringTuple *attrAIF = attrRef.getAIFStringTuple();
	
	AttributePrimIdGroupMapIterator it;
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	for ( GA_Iterator pIt=geo->getPrimitiveRange().begin(); !pIt.atEnd(); ++pIt )
	{
		GA_Primitive *prim = primitives.get( pIt.getOffset() );
		unsigned primType = prim->getTypeId().get();
		
		std::string value = "";
		const char *tmp = attrAIF->getString( attr, pIt.getOffset() );
		if ( tmp )
		{
			value = tmp;
		}
		
		AttributePrimIdPair key( value, primType );
		it = groupMap.find( key );
		if ( it == groupMap.end() )
		{
			AttributePrimIdGroupPair pair( key, static_cast<GA_PrimitiveGroup*>( geo->createInternalElementGroup( GA_ATTRIB_PRIMITIVE, ( boost::format( "FromHoudiniGroupConverter__typedPrimitives%d%s" ) % primType % value ).str().c_str() ) ) );
			it = groupMap.insert( pair ).first;
		}
		
		it->second->add( prim );
	}
	
	return groupMap.size();
}

PrimitivePtr FromHoudiniGroupConverter::doPrimitiveConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );

	FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( handle );
	if ( !converter || converter->isInstanceOf( FromHoudiniGroupConverter::staticTypeId() ) )
	{
		/// \todo: if we're in PrimitiveGroup mode, but names exist, we return 0 when we should be returning a Group
		return 0;
	}
	
	// transfer the common parameter values
	CompoundParameter *parameters = converter->parameters();
	const CompoundParameter::ParameterMap &parameterMap = parameters->parameters();
	const CompoundObject::ObjectMap &values = operands->members();
	for ( CompoundObject::ObjectMap::const_iterator it = values.begin(); it != values.end(); ++it )
	{
		CompoundParameter::ParameterMap::const_iterator pIt = parameterMap.find( it->first );
		if ( pIt != parameterMap.end() && pIt->second->defaultValue()->typeId() == it->second->typeId() )
		{
			parameters->setParameterValue( it->first, it->second );
		}
	}
	
	return IECore::runTimeCast<Primitive>( converter->convert() );
}

void FromHoudiniGroupConverter::convertAndAddPrimitive( GU_Detail *geo, GA_PrimitiveGroup *group, GroupPtr &result, const CompoundObject *operands, const std::string &name ) const
{
	GU_Detail childGeo( geo, group );
	for ( GA_GroupTable::iterator<GA_ElementGroup> it=childGeo.primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
	{
		it.group()->clear();
	}
	childGeo.destroyAllEmptyGroups();
	
	PrimitivePtr child = doPrimitiveConversion( &childGeo, operands );
	if ( child )
	{
		if ( name != "" )
		{
			child->blindData()->member<StringData>( "name", false, true )->writable() = name;
		}
		
		result->addChild( child );
	}
}
