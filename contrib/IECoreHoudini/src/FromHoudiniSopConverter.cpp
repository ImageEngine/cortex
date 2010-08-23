//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "IECore/CompoundObject.h"

#include "CoreHoudini.h"
#include "FromHoudiniSopConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniSopConverter );

// ctor
FromHoudiniSopConverter::FromHoudiniSopConverter( const SOP_Node *sop, const std::string &description ) :
	FromHoudiniNodeConverter( sop, description )
{
}

// dtor
FromHoudiniSopConverter::~FromHoudiniSopConverter()
{
}

SOP_Node *FromHoudiniSopConverter::sop() const
{
	return CAST_SOPNODE( node() );
}

ObjectPtr FromHoudiniSopConverter::doConversion( ConstCompoundObjectPtr operands ) const
{
	// find global time
	float time = CoreHoudini::currTime();

	// create the work context
	OP_Context context;
	context.setTime( time );

	// get the sop
	SOP_Node *sop = this->sop();
	if( !sop )
	{
		return 0;
	}
	
	// get the geometry
	const GU_Detail *geo = sop->getCookedGeo( context );
	if ( !geo )
	{
		return 0;
	}
	
	return doPrimitiveConversion( geo, operands );
}

void FromHoudiniSopConverter::transferAttribs(
	const GU_Detail *geo, IECore::Primitive *result,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{
	// add position
	unsigned i = 0;
	const GEO_PointList &points = geo->points();
	size_t numPoints = points.entries();
	std::vector<Imath::V3f> pData( numPoints );
	for ( const GEO_Point *point = points.head(); point !=0 ; point = points.next( point ), i++ )
	{
		const UT_Vector4 &pos = point->getPos();
		pData[i] = Imath::V3f( pos[0], pos[1], pos[2] );
	}

	result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( pData ) );
	
	// add detail attribs	
	if ( result->variableSize( detailInterpolation ) == 1 )
	{
		transferDetailAttribs( geo, result, detailInterpolation );
	}
	
	// add point attribs
	if ( result->variableSize( pointInterpolation ) == numPoints )
	{
		transferPointAttribs( geo, result, pointInterpolation, points );
	}
	
	// add primitive attribs
	const GEO_PrimList &primitives = geo->primitives();
	size_t numPrims = primitives.entries();
	if ( result->variableSize( primitiveInterpolation ) == numPrims )
	{
		transferPrimitiveAttribs( geo, result, primitiveInterpolation, primitives );
	}
	
	// add vertex attribs
	size_t numVerts = 0;
	for ( size_t i=0; i < numPrims; i++ )
	{
		numVerts += primitives[i]->getVertexCount();
	}
	if ( result->variableSize( vertexInterpolation ) == numVerts )
	{
		size_t vertCount = 0;
		VertexList vertices( numVerts );
		for ( size_t i=0; i < numPrims; i++ )
		{
			const GEO_Primitive *prim = primitives[i];
			size_t numPrimVerts = prim->getVertexCount();
			for ( size_t v=0; v < numPrimVerts; v++, vertCount++ )
			{
				if ( prim->getPrimitiveId() & GEOPRIMPOLY )
				{
					vertices[vertCount] = &prim->getVertex( numPrimVerts - 1 - v );
				}
				else
				{
					vertices[vertCount] = &prim->getVertex( v );
				}
			}
		}
		
		transferVertexAttribs( geo, result, vertexInterpolation, vertices );
	}
}

void FromHoudiniSopConverter::transferDetailAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
{
	const GB_AttributeTable &attribs = geo->attribs();
	
	for( UT_LinkNode *current=attribs.head(); current != 0; current = attribs.next( current ) )
	{
		GB_Attribute *attr = dynamic_cast<GB_Attribute*>( current );
		if ( !attr )
		{
			continue;
		}
		
		GB_AttributeRef attrRef = geo->findAttrib( attr );
		if ( GBisAttributeRefInvalid( attrRef ) )
		{
			continue;
		}
		
		DataPtr dataPtr = 0;
		
		switch ( attr->getType() )
		{
			case GB_ATTRIB_FLOAT :
			{
				unsigned dimensions = attr->getSize() / sizeof( float );
				switch ( dimensions )
				{
					case 1:
						dataPtr = extractData<FloatData>( attribs, attrRef );
						break;
					case 2:
						dataPtr = extractData<V2fData>( attribs, attrRef );
						break;
					case 3:
						dataPtr = extractData<V3fData>( attribs, attrRef );
						break;
					default:
						break;
				}
				break;
			}
			case GB_ATTRIB_INT :
 			{
				unsigned dimensions = attr->getSize() / sizeof( float );
				switch ( dimensions )
				{
					case 1:
						dataPtr = extractData<IntData>( attribs, attrRef );
						break;
					case 2:
						dataPtr = extractData<V2iData>( attribs, attrRef );
						break;
					case 3:
						dataPtr = extractData<V3iData>( attribs, attrRef );
						break;
					default:
						break;
				}
				break;
 			}
 			case GB_ATTRIB_VECTOR :
 			{
				unsigned dimensions = attr->getSize() / (sizeof( float ) * 3);
				if ( dimensions == 1 ) // only support single element vectors
				{
					dataPtr = extractData<V3fData>( attribs, attrRef );
				}
 				break;
 			}
			default :
			{
				break;
			}
		}
		
		if ( dataPtr )
		{
			result->variables[ std::string( attr->getName() ) ] = PrimitiveVariable( interpolation, dataPtr );
		}
	}
}

void FromHoudiniSopConverter::transferPointAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PointList &points ) const
{
	const GEO_PointAttribDict &attribs = geo->pointAttribs();
	
	for( UT_LinkNode *current=attribs.head(); current != 0; current = attribs.next( current ) )
	{
		GB_Attribute *attr = dynamic_cast<GB_Attribute*>( current );
		if ( !attr )
		{
			continue;
		}
		
		GB_AttributeRef attrRef = geo->findPointAttrib( attr );
		if ( GBisAttributeRefInvalid( attrRef ) )
		{
			continue;
		}
		
		transferAttribData<GEO_PointList>( points, result, interpolation, attr, attrRef );
	}
}

void FromHoudiniSopConverter::transferPrimitiveAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PrimList &primitives ) const
{
	const GEO_PrimAttribDict &attribs = geo->primitiveAttribs();
	
	for( UT_LinkNode *current=attribs.head(); current != 0; current = attribs.next( current ) )
	{
		GB_Attribute *attr = dynamic_cast<GB_Attribute*>( current );
		if ( !attr )
		{
			continue;
		}
		
		GB_AttributeRef attrRef = geo->findPrimAttrib( attr );
		if ( GBisAttributeRefInvalid( attrRef ) )
		{
			continue;
		}
		
		transferAttribData<GEO_PrimList>( primitives, result, interpolation, attr, attrRef );
	}
}

void FromHoudiniSopConverter::transferVertexAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const VertexList &vertices ) const
{
	const GEO_VertexAttribDict &attribs = geo->vertexAttribs();
	
	for( UT_LinkNode *current=attribs.head(); current != 0; current = attribs.next( current ) )
	{
		GB_Attribute *attr = dynamic_cast<GB_Attribute*>( current );
		if ( !attr )
		{
			continue;
		}
		
		GB_AttributeRef attrRef = geo->findVertexAttrib( attr );
		if ( GBisAttributeRefInvalid( attrRef ) )
		{
			continue;
		}
		
		transferAttribData<VertexList>( vertices, result, interpolation, attr, attrRef );
	}
}
