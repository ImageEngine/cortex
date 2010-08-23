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

FromHoudiniNodeConverter::Description<FromHoudiniPolygonsConverter> FromHoudiniPolygonsConverter::m_description( SOP_OPTYPE_ID, MeshPrimitiveTypeId );

FromHoudiniPolygonsConverter::FromHoudiniPolygonsConverter( const SOP_Node *sop ) :
	FromHoudiniSopConverter( sop, "Converts Houdini SOP geometry to a IECore::MeshPrimitive." )
{
}

FromHoudiniPolygonsConverter::~FromHoudiniPolygonsConverter()
{
}

PrimitivePtr FromHoudiniPolygonsConverter::doPrimitiveConversion( const GU_Detail *geo, IECore::ConstCompoundObjectPtr operands ) const
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
			throw runtime_error( ( boost::format( "FromHoudiniPolygonsConverter: SOP \"%d\" contains non-polygon primitives" ) % sop()->getName() ).str() );
		}
		
		numVerts += prim->getVertexCount();
	}
	
	if ( !numVerts )
	{
		throw runtime_error( ( boost::format( "FromHoudiniPolygonsConverter: SOP \"%d\" does not contain polygon vertices" ) % sop()->getName() ).str() );
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
	
	transferAttribs( geo, result );
	
	return result;
}
