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

#include "IECore/CompoundData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"

#include "Convert.h"
#include "ToHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniGeometryConverter );

ToHoudiniGeometryConverter::ToHoudiniGeometryConverter( const Primitive *primitive, const std::string &description )
	:	ToHoudiniConverter( description, PrimitiveTypeId )
{
	srcParameter()->setValue( (Primitive *)primitive );
}

ToHoudiniGeometryConverter::~ToHoudiniGeometryConverter()
{
}

bool ToHoudiniGeometryConverter::convert( GU_DetailHandle handle ) const
{
	ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<CompoundObject>();
	GU_DetailHandleAutoWriteLock writeHandle( handle );
	
	GU_Detail *geo = writeHandle.getGdp();
	if ( !geo )
	{
		return false;
	}
	
	return doPrimitiveConversion( (const Primitive *)srcParameter()->getValidatedValue(), geo, operands );
}

GEO_PointList ToHoudiniGeometryConverter::appendPoints( GU_Detail *geo, const IECore::V3fVectorData *positions ) const
{
	if ( !positions )
	{
		return GEO_PointList();
	}
	
	GEO_PointList points;
	const std::vector<Imath::V3f> &pos = positions->readable();
	for ( size_t i=0; i < pos.size(); i++ )
	{
		GEO_Point *p = geo->appendPoint();
		p->setPos( IECore::convert<UT_Vector3>( pos[i] ) );
		points.append( p );
	}
	
	return points;
}

void ToHoudiniGeometryConverter::transferAttribs(
	const Primitive *primitive, GU_Detail *geo,
	GEO_PointList *newPoints, GEO_PrimList *newPrims,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{
	// gather the vertices
	size_t numVerts = 0;
	size_t numPrims = newPrims ? newPrims->entries() : 0;
	for ( size_t i=0; i < numPrims; i++ )
	{
		numVerts += (*newPrims)[i]->getVertexCount();
	}
	
	size_t vertCount = 0;
	VertexList vertices( numVerts );
	for ( size_t i=0; i < numPrims; i++ )
	{
		GEO_Primitive *prim = (*newPrims)[i];
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
	
 	// add the primitive variables to the various GEO_AttribDicts based on interpolation type
	for ( PrimitiveVariableMap::const_iterator it=primitive->variables.begin() ; it != primitive->variables.end(); it++ )
	{
		// P should already have been added as points
		if ( it->first == "P" )
		{
			continue;
		}
		else if ( it->second.interpolation == detailInterpolation )
 		{
			// add detail attribs
			TransferDetailAttrib func( geo, it->first );
			despatchTypedData<TransferDetailAttrib, TypeTraits::IsDetailGbAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data, func );
	 	}
		else if ( it->second.interpolation == pointInterpolation )
		{
			// add point attribs
			TransferAttrib<GEO_PointList> func( geo, newPoints, it->first, GEO_POINT_DICT );
			despatchTypedData<TransferAttrib<GEO_PointList>, TypeTraits::IsVectorGbAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data, func );
		}
		else if ( it->second.interpolation == primitiveInterpolation )
		{
			// add primitive attribs
			TransferAttrib<GEO_PrimList> func( geo, newPrims, it->first, GEO_PRIMITIVE_DICT );
			despatchTypedData<TransferAttrib<GEO_PrimList>, TypeTraits::IsVectorGbAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data, func );
		}
		else if ( it->second.interpolation == vertexInterpolation )
		{
			// add vertex attribs
			TransferAttrib<VertexList> func( geo, &vertices, it->first, GEO_VERTEX_DICT );
			despatchTypedData<TransferAttrib<VertexList>, TypeTraits::IsVectorGbAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data, func );
		}
	}
}

ToHoudiniGeometryConverter::TransferDetailAttrib::TransferDetailAttrib( GU_Detail *geo, std::string name )
	: m_geo( geo ), m_name( name )
{
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniGeometryConverterPtr ToHoudiniGeometryConverter::create( const Primitive *primitive )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( primitive->typeId() ) );
	if( it!=m->end() )
	{
		return it->second( primitive );
	}
	
	return 0;
}

void ToHoudiniGeometryConverter::registerConverter( IECore::TypeId fromType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType ), creator ) );
}

ToHoudiniGeometryConverter::TypesToFnsMap *ToHoudiniGeometryConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniGeometryConverter::Types::Types( IECore::TypeId from ) : fromType( from )
{
}

bool ToHoudiniGeometryConverter::Types::operator < ( const Types &other ) const
{
	return fromType < other.fromType;
}
