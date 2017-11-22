//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundObject.h"

#include "IECoreHoudini/FromHoudiniCompoundObjectConverter.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

#if UT_MAJOR_VERSION_INT >= 14

typedef GEO_CortexPrimitive CortexPrimitive;

#else

#include "IECoreHoudini/GU_CortexPrimitive.h"

typedef GU_CortexPrimitive CortexPrimitive;

#endif

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniCompoundObjectConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniCompoundObjectConverter> FromHoudiniCompoundObjectConverter::m_description( CompoundObjectTypeId );

FromHoudiniCompoundObjectConverter::FromHoudiniCompoundObjectConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::CompoundObject." )
{
}

FromHoudiniCompoundObjectConverter::FromHoudiniCompoundObjectConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::CompoundObject." )
{
}

FromHoudiniCompoundObjectConverter::~FromHoudiniCompoundObjectConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniCompoundObjectConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	// need multiple names
	GA_ROAttributeRef attrRef = geo->findPrimitiveAttribute( "name" );
	if ( attrRef.isValid() && attrRef.isString() )
	{
		const GA_Attribute *nameAttr = attrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		GA_StringTableStatistics stats;
		tuple->getStatistics( nameAttr, stats );
		if ( stats.getEntries() < 2 )
		{
			return Inapplicable;
		}
	}
	else
	{
		return Inapplicable;
	}

	// Need them all to be convertable as objects. Even then, if they're VisibleRenderable,
	// then the FromHoudiniGroupConverter would be preferable.
	bool nonRenderable = false;
	for ( GA_Iterator it = geo->getPrimitiveRange().begin(); !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );

		if ( prim->getTypeId() != CortexPrimitive::typeId() )
		{
			return Inapplicable;
		}

		if ( !IECore::runTimeCast<const VisibleRenderable>( ((CortexPrimitive *)prim)->getObject() ) )
		{
			nonRenderable = true;
		}
	}

	return ( nonRenderable ) ? Ideal : Suitable;
}

ObjectPtr FromHoudiniCompoundObjectConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	GA_ROAttributeRef attrRef = geo->findPrimitiveAttribute( "name" );
	if ( attrRef.isInvalid() || !attrRef.isString() )
	{
		throw std::runtime_error( "FromHoudiniCompoundObjectConverter: Can only convert named CortexObject primitives" );
	}

	CompoundObjectPtr result = new CompoundObject();

	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFStringTuple *attrAIF = attrRef.getAIFStringTuple();
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	for ( GA_Iterator it = geo->getPrimitiveRange().begin(); !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );

		if ( prim->getTypeId() != CortexPrimitive::typeId() )
		{
			throw std::runtime_error( "FromHoudiniCompoundObjectConverter: Geometry contains non-CortexObject primitives" );
		}

		std::string name = "";
		const char *tmp = attrAIF->getString( attr, it.getOffset() );
		if ( tmp )
		{
			name = tmp;
		}

		ConstObjectPtr object = ((CortexPrimitive *)prim)->getObject();

		result->members()[name] = ( object ) ? object->copy() : 0;
	}

	return result;
}
