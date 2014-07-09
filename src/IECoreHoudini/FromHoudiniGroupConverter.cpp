//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2014, Image Engine Design Inc. All rights reserved.
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

#include "UT/UT_PtrArray.h"

#include "IECore/CompoundParameter.h"
#include "IECore/Group.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

#include "IECoreHoudini/DetailSplitter.h"
#include "IECoreHoudini/FromHoudiniGroupConverter.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"

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
	GA_ROAttributeRef attrRef = geo->findPrimitiveAttribute( "name" );
	if ( attrRef.isValid() && attrRef.isString() )
	{
		const GA_Attribute *nameAttr = attrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		if ( tuple->getTableEntries( nameAttr ) > 1 )
		{
			return Ideal;
		}
	}
	
	// are there multiple GU_CortexPrimitives holding VisibleRenderables?
	unsigned numCortex = 0;
	unsigned numVisibleRenderable = 0;
	for ( GA_Iterator it = geo->getPrimitiveRange().begin(); !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( prim->getTypeId() != GU_CortexPrimitive::typeId() )
		{
			continue;
		}
		
		numCortex++;
		if ( IECore::runTimeCast<const VisibleRenderable>( ((GU_CortexPrimitive *)prim)->getObject() ) )
		{
			numVisibleRenderable++;
		}
	}
	
	if ( numVisibleRenderable > 1 && numCortex == numVisibleRenderable )
	{
		return Ideal;
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
		const GA_ElementGroup *group = primGroups( i );
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
	GroupPtr result = new Group();
	
	if ( operands->member<const IntData>( "groupingMode" )->readable() == NameAttribute )
	{
		DetailSplitterPtr splitter = new DetailSplitter( handle() );
		std::vector<std::string> children;
		splitter->values( children );
		for ( std::vector<std::string>::iterator it = children.begin(); it != children.end(); ++it )
		{
			const std::string &name = *it;
			GU_DetailHandle childHandle = splitter->split( name );
			if ( childHandle.isNull() )
			{
				continue;
			}
			
			GU_DetailHandleAutoReadLock readHandle( childHandle );
			const GU_Detail *childGeo = readHandle.getGdp();
			ObjectPtr child = doDetailConversion( childGeo, operands.get() );
			if ( !child )
			{
				// this happens when mismatched primitives share the same name
				doUnnamedConversion( childGeo, result.get(), operands.get(), name );
			}
			else if ( VisibleRenderablePtr renderable = IECore::runTimeCast<VisibleRenderable>( child ) )
			{
				if ( name != "" )
				{
					renderable->blindData()->member<StringData>( "name", false, true )->writable() = name;
				}
				
				result->addChild( renderable );
			}
		}
	}
	else
	{
		GU_DetailHandleAutoReadLock readHandle( handle() );
		const GU_Detail *geo = readHandle.getGdp();
		if ( !geo )
		{
			return 0;
		}

		size_t numResultPrims = 0;
		size_t numOrigPrims = geo->getNumPrimitives();

		for ( GA_GroupTable::iterator<GA_ElementGroup> it=geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
		{
			GA_PrimitiveGroup *group = static_cast<GA_PrimitiveGroup*>( it.group() );
			if ( group->getInternal() || group->isEmpty() )
			{
				continue;
			}

			VisibleRenderablePtr renderable = 0;
			numResultPrims += doGroupConversion( geo, group, renderable, operands.get() );
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
		doGroupConversion( &ungroupedGeo, ungrouped, renderable, operands.get() );
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
		result = IECore::runTimeCast<VisibleRenderable>( doDetailConversion( &groupGeo, operands ) );
		return numPrims;
	}
	
	PrimIdGroupMap groupMap;
	groupGeo.destroyEmptyGroups( GA_ATTRIB_PRIMITIVE );
	size_t numNewGroups = regroup( &groupGeo, groupMap );
	
	if ( numNewGroups < 2 )
	{
		result = IECore::runTimeCast<VisibleRenderable>( doDetailConversion( &groupGeo, operands ) );
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

void FromHoudiniGroupConverter::doUnnamedConversion( const GU_Detail *geo, Group *result, const CompoundObject *operands, const std::string &name ) const
{
	GA_OffsetList unusedOffsets;
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	for ( GA_Iterator pIt=geo->getPrimitiveRange().begin(); !pIt.atEnd(); ++pIt )
	{
		if ( primitives.get( pIt.getOffset() )->getTypeId() == GU_CortexPrimitive::typeId() )
		{
			GA_OffsetList offsets;
			offsets.append( pIt.getOffset() );
			GU_Detail *newGeo = new GU_Detail();
			GA_Range thisPrim( geo->getPrimitiveMap(), offsets );
			newGeo->mergePrimitives( *geo, thisPrim );
			newGeo->incrementMetaCacheCount();
			ObjectPtr object = doDetailConversion( newGeo, operands );
			if ( VisibleRenderablePtr renderable = IECore::runTimeCast<VisibleRenderable>( object ) )
			{
				result->addChild( renderable );
			}
		}
		else
		{
			unusedOffsets.append( pIt.getOffset() );
		}
	}
	
	GU_Detail newGeo( (GU_Detail*)geo );
	GA_PrimitiveGroup *newGroup = static_cast<GA_PrimitiveGroup*>( newGeo.createInternalElementGroup( GA_ATTRIB_PRIMITIVE, "FromHoudiniGroupConverter__doUnnamedConversion" ) );
	GA_Range unusedRange( newGeo.getPrimitiveMap(), unusedOffsets );
	newGroup->toggleRange( unusedRange );
	
	VisibleRenderablePtr renderable = 0;
	doGroupConversion( &newGeo, newGroup, renderable, operands );
	if ( renderable )
	{
		if ( Group *group = IECore::runTimeCast<Group>( renderable.get() ) )
		{
			const Group::ChildContainer &children = group->children();
			for ( Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
			{
				if ( name != "" )
				{
					(*it)->blindData()->member<StringData>( "name", false, true )->writable() = name;
				}
				
				result->addChild( *it );
			}
		}
		else
		{
			if ( name != "" )
			{
				renderable->blindData()->member<StringData>( "name", false, true )->writable() = name;
			}
			
			result->addChild( renderable );
		}
	}
}

ObjectPtr FromHoudiniGroupConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
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
	
	return converter->convert();
}

void FromHoudiniGroupConverter::convertAndAddPrimitive( GU_Detail *geo, GA_PrimitiveGroup *group, GroupPtr &result, const CompoundObject *operands, const std::string &name ) const
{
	GU_Detail childGeo( geo, group );
	for ( GA_GroupTable::iterator<GA_ElementGroup> it=childGeo.primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
	{
		it.group()->clear();
	}
	childGeo.destroyAllEmptyGroups();
	
	PrimitivePtr child = IECore::runTimeCast<Primitive>( doDetailConversion( &childGeo, operands ) );
	if ( child )
	{
		if ( name != "" )
		{
			child->blindData()->member<StringData>( "name", false, true )->writable() = name;
		}
		
		result->addChild( child );
	}
}
