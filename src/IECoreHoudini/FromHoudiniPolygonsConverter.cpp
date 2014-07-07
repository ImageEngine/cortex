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

#include "IECore/CompoundObject.h"

#include "IECoreHoudini/FromHoudiniPolygonsConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniPolygonsConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniPolygonsConverter> FromHoudiniPolygonsConverter::m_description( MeshPrimitiveTypeId );

FromHoudiniPolygonsConverter::FromHoudiniPolygonsConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::MeshPrimitive." )
{
}

FromHoudiniPolygonsConverter::FromHoudiniPolygonsConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::MeshPrimitive." )
{
}

FromHoudiniPolygonsConverter::~FromHoudiniPolygonsConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniPolygonsConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	
	for ( GA_Iterator it=geo->getPrimitiveRange().begin(); !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( prim->getTypeId() != GEO_PRIMPOLY )
		{
			return Inapplicable;
		}
	}
	
	// is there a single named shape?
	GA_ROAttributeRef attrRef = geo->findPrimitiveAttribute( "name" );
	if ( attrRef.isValid() && attrRef.isString() )
	{
		const GA_Attribute *nameAttr = attrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		if ( tuple->getTableEntries( nameAttr ) < 2 )
		{
			return Ideal;
		}
	}
	
	return Suitable;
}

ObjectPtr FromHoudiniPolygonsConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	
	MeshPrimitivePtr result = new MeshPrimitive();
	
	GA_Iterator firstPrim = geo->getPrimitiveRange().begin();
	for ( GA_Iterator it=firstPrim; !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( prim->getTypeId() != GEO_PRIMPOLY )
		{
			throw std::runtime_error( "FromHoudiniPolygonsConverter: Geometry contains non-polygon primitives" );
		}
	}
	
	// loop over primitives gathering mesh data
	std::vector<int> vertIds;
	std::vector<int> vertsPerFace;
	for ( GA_Iterator it=firstPrim; !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		size_t numPrimVerts = prim->getVertexCount();
		vertsPerFace.push_back( numPrimVerts );
		std::vector<int> ids( numPrimVerts );
		for ( size_t j=0; j < numPrimVerts; j++ )
		{
			vertIds.push_back( geo->pointIndex( prim->getPointOffset( numPrimVerts - 1 - j ) ) );
		}
	}
	
	// try to get the interpolation type from the geo
	CompoundObjectPtr modifiedOperands = 0;
	std::string interpolation = "linear";
	const GA_ROAttributeRef attrRef = geo->findStringTuple( GA_ATTRIB_PRIMITIVE, GA_SCOPE_PUBLIC, "ieMeshInterpolation" );
	if ( attrRef.isValid() )
	{
		modifiedOperands = operands->copy();
		std::string &attributeFilter = modifiedOperands->member<StringData>( "attributeFilter" )->writable();
		attributeFilter += " ^ieMeshInterpolation";
		
		GA_Range primRange = geo->getPrimitiveRange();
		for ( GA_Iterator it=primRange.begin(); !it.atEnd(); ++it )
		{
			const char *value = attrRef.getString( it.getOffset() );
			if ( value )
			{
				if ( !strcmp( value, "subdiv" ) )
				{
					interpolation = "catmullClark";
					// subdivision meshes should not have normals. we assume this occurred because the geo contained
					// both subdiv and linear meshes, inadvertantly extending the normals attribute to both.
					attributeFilter += " ^N";
					break;
				}
				else if ( !strcmp( value, "poly" ) )
				{
					interpolation = "linear";
					break;
				}
			}
		}
	}
	
	result->setTopology( new IntVectorData( vertsPerFace ), new IntVectorData( vertIds ), interpolation );
	
	if ( geo->getNumVertices() )
	{
		transferAttribs( geo, result.get(), modifiedOperands ? modifiedOperands.get() : operands );
	}
	
	return result;
}
