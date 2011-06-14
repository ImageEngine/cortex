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

#include "FromHoudiniPolygonsConverter.h"

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
	const GEO_PrimList &primitives = geo->primitives();
	
	size_t numPrims = primitives.entries();
	for ( size_t i=0; i < numPrims; i++ )
	{
		const GEO_Primitive *prim = primitives( i );
		if ( !( prim->getPrimitiveId() & GEOPRIMPOLY ) )
		{
			return Inapplicable;
		}
	}
	
	const GB_GroupList &primGroups = geo->primitiveGroups();
	if ( !primGroups.length() || primGroups.head()->entries() == numPrims )
	{
		return Ideal;
	}
	
	return Suitable;
}

PrimitivePtr FromHoudiniPolygonsConverter::doPrimitiveConversion( const GU_Detail *geo ) const
{
	const GEO_PrimList &primitives = geo->primitives();
	
	MeshPrimitivePtr result = new MeshPrimitive();
	
	size_t numVerts = 0;
	size_t numPrims = primitives.entries();
	for ( size_t i=0; i < numPrims; i++ )
	{
		const GEO_Primitive *prim = primitives( i );
		if ( !( prim->getPrimitiveId() & GEOPRIMPOLY ) )
		{
			throw runtime_error( "FromHoudiniPolygonsConverter: Geometry contains non-polygon primitives" );
		}
		
		numVerts += prim->getVertexCount();
	}
	
	// loop over primitives gathering mesh data
	std::vector<int> vertIds;
	std::vector<int> vertsPerFace;
	for ( size_t i=0; i < numPrims; i++ )
	{
		const GEO_Primitive *prim = primitives( i );
		size_t numPrimVerts = prim->getVertexCount();
		vertsPerFace.push_back( numPrimVerts );
		std::vector<int> ids( numPrimVerts );
		for ( size_t j=0; j < numPrimVerts; j++ )
		{
			const GEO_Vertex &vert = prim->getVertex( numPrimVerts - 1 - j );
			vertIds.push_back( vert.getPt()->getNum() );
		}
	}
	
	result->setTopology( new IntVectorData( vertsPerFace ), new IntVectorData( vertIds ) );
	
	if ( numVerts )
	{
		transferAttribs( geo, result );
	}
	
	return result;
}
