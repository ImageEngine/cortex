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

#include "GU/GU_PrimPoly.h"

#include "IECoreHoudini/ToHoudiniPolygonsConverter.h"
#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniPolygonsConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniPolygonsConverter> ToHoudiniPolygonsConverter::m_description( IECoreScene::MeshPrimitive::staticTypeId() );

ToHoudiniPolygonsConverter::ToHoudiniPolygonsConverter( const Object *object ) :
	ToHoudiniGeometryConverter( object, "Converts an IECoreScene::MeshPrimitive to a Houdini GU_Detail." )
{
}

ToHoudiniPolygonsConverter::~ToHoudiniPolygonsConverter()
{
}

bool ToHoudiniPolygonsConverter::doConversion( const Object *object, GU_Detail *geo ) const
{
	const MeshPrimitive *mesh = static_cast<const MeshPrimitive *>( object );
	if ( !mesh )
	{
		return false;
	}

	GA_Range newPoints = appendPoints( geo, mesh->variableSize( PrimitiveVariable::Vertex ) );
	if ( !newPoints.isValid() || newPoints.empty() )
	{
		return false;
	}

	GA_OffsetList pointOffsets;
	pointOffsets.harden( newPoints.getEntries() );
	pointOffsets.setEntries( newPoints.getEntries() );

	size_t i = 0;
	GA_Offset start, end;
	for( GA_Iterator it( newPoints ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset, ++i )
		{
			pointOffsets.set( i, offset );
		}
	}

	const std::vector<int> &vertexIds = mesh->vertexIds()->readable();
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();

	GA_OffsetList offsets;
	offsets.harden( verticesPerFace.size() );
	offsets.setEntries( verticesPerFace.size() );

	size_t vertCount = 0;
	size_t numPrims = geo->getNumPrimitives();
	for ( size_t f = 0, numFaces = verticesPerFace.size(); f < numFaces; ++f )
	{
		GU_PrimPoly *poly = GU_PrimPoly::build( geo, 0, GU_POLY_CLOSED, 0 );
		offsets.set( f, geo->primitiveOffset( numPrims + f ) );

		for( size_t v=0; v < (size_t)verticesPerFace[f]; ++v )
		{
			poly->appendVertex( pointOffsets.get( vertexIds[ vertCount + verticesPerFace[f] - 1 - v ] ) );
		}

		vertCount += verticesPerFace[f];
	}

	GA_Range newPrims( geo->getPrimitiveMap(), offsets );
	transferAttribs( geo, newPoints, newPrims );

	// add the interpolation type
	if ( newPrims.isValid() )
	{
		std::string interpolation = ( mesh->interpolation() == "catmullClark" ) ? "subdiv" : "poly";
		ToHoudiniStringVectorAttribConverter::convertString( "ieMeshInterpolation", interpolation, geo, newPrims );
	}

	return true;
}
