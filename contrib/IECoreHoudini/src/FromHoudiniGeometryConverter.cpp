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
#include "FromHoudiniGeometryConverter.h"

#include <boost/tokenizer.hpp>

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniGeometryConverter );

FromHoudiniGeometryConverter::FromHoudiniGeometryConverter( const GU_DetailHandle &handle, const std::string &description )
	: FromHoudiniConverter( description ), m_geoHandle( handle )
{
}

FromHoudiniGeometryConverter::FromHoudiniGeometryConverter( const SOP_Node *sop, const std::string &description )
	: FromHoudiniConverter( description )
{
	m_geoHandle = handle( sop );
}

FromHoudiniGeometryConverter::~FromHoudiniGeometryConverter()
{
}

const GU_DetailHandle FromHoudiniGeometryConverter::handle( const SOP_Node *sop )
{
	// find global time
	float time = CoreHoudini::currTime();

	// create the work context
	OP_Context context;
	context.setTime( time );
	
	return ((SOP_Node*)sop)->getCookedGeoHandle( context );
}

ObjectPtr FromHoudiniGeometryConverter::doConversion( ConstCompoundObjectPtr operands ) const
{
	GU_DetailHandleAutoReadLock readHandle( m_geoHandle );
	
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return 0;
	}
	
	return doPrimitiveConversion( geo, operands );
}

/// Create a remapping matrix of names, types and interpolation classes for all attributes specified in the 'rixlate' detail attribute.
FromHoudiniGeometryConverter::AttributeRemapping FromHoudiniGeometryConverter::getAttributeRemapping( const GU_Detail *geo ) const
{
	AttributeRemapping remap;
	remap[RemappingInfo::Point] = MappingMap();
	remap[RemappingInfo::Primitive] = MappingMap();

	const GB_AttributeTable &attribs = geo->attribs();
	GB_Attribute *remap_attr = attribs.find("rixlate", GB_ATTRIB_INDEX);
	if ( remap_attr!=0 )
	{
		std::vector<std::string> strings;
		int num_strings = remap_attr->getIndexSize();
		for ( int i=0; i<num_strings; ++i )
		{
			RemappingInfo info;
			const char *str = remap_attr->getIndex(i);
			std::string attribute_str( str );

			// split up our rixlate string
			typedef boost::char_separator< char > Sep;
			typedef boost::tokenizer< Sep > Tokeniser;
			std::vector<std::string> tokens;
			Tokeniser attribute_split( attribute_str, Sep(":") );
			for ( Tokeniser::iterator it=attribute_split.begin(); it!=attribute_split.end(); ++it )
			{
				tokens.push_back( *it );
			}

			// not enough elements!
			if ( tokens.size()<4 )
			{
				continue;
			}

			// our attribute type
			RemappingInfo::AttrType attr_type = RemappingInfo::Point;
			if ( tokens[0]=="prim" )
				attr_type = RemappingInfo::Primitive;

			// our source attribute
			std::string from_name = tokens[1];

			// our destination attribute
			info.name = tokens[2];

			// our data types
			std::vector<std::string> data_tokens;
			Tokeniser data_split( tokens[3], Sep("_") );
			for ( Tokeniser::iterator it=data_split.begin(); it!=data_split.end(); ++it )
			{
				data_tokens.push_back( *it );
			}
			if ( data_tokens.size()==2 ) // we need both class & type!
			{
				// our interpolation type
				std::string class_str = data_tokens[0];
				if ( class_str=="vtx" )
				{
					info.interpolation = IECore::PrimitiveVariable::Vertex;
				}
				else if ( class_str=="v" )
				{
					info.interpolation = IECore::PrimitiveVariable::Varying;
				}
				else if ( class_str=="u" )
				{
					info.interpolation = IECore::PrimitiveVariable::Uniform;
				}
				else if ( class_str=="c" )
				{
					info.interpolation = IECore::PrimitiveVariable::Constant;
				}

				// our types
				std::string type_str = data_tokens[1];
				if ( type_str=="float" )
				{
					info.type = IECore::FloatVectorDataTypeId;
				}
				else if ( type_str=="color" )
				{
					info.type = IECore::Color3fVectorDataTypeId;
				}
				else if ( type_str=="point" )
				{
					info.type = IECore::V3fVectorDataTypeId;
				}
				else if ( type_str=="vector" )
				{
					info.type = IECore::V3fVectorDataTypeId;
				}
				else if ( type_str=="normal" )
				{
					info.type = IECore::V3fVectorDataTypeId;
				}
				else if ( type_str=="string" )
				{
					info.type = IECore::StringVectorDataTypeId;
				}
			}

			// our data offset
			info.offset = 0;
			if ( tokens.size()==5 )
			{
				info.offset = boost::lexical_cast<int>( tokens[4] );
			}

			// put our remapping information into our map
			remap[attr_type][from_name].push_back( info );
		}
	}

	// return our vector of attribute maps - 0: point attrs, 1: prim attrs
	return remap;
}

void FromHoudiniGeometryConverter::transferAttribs(
	const GU_Detail *geo, IECore::Primitive *result,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{
	// get RI remapping information from the detail
	AttributeRemapping attribute_remapping = getAttributeRemapping( geo );

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
		transferPointAttribs( geo, result, pointInterpolation, points, attribute_remapping );
	}
	
	// add primitive attribs
	const GEO_PrimList &primitives = geo->primitives();
	size_t numPrims = primitives.entries();
	
	if ( result->variableSize( primitiveInterpolation ) == numPrims )
	{
		transferPrimitiveAttribs( geo, result, primitiveInterpolation, primitives, attribute_remapping );
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

void FromHoudiniGeometryConverter::transferDetailAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
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

void FromHoudiniGeometryConverter::transferPointAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PointList &points, AttributeRemapping &attribute_remap ) const
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
		
		// check for remapping information for this attribute
		std::string attr_name(attr->getName());
		if ( attribute_remap[RemappingInfo::Point].count(attr_name)==1 )
		{
			std::vector<RemappingInfo> &map = attribute_remap[RemappingInfo::Point][attr_name];
			for ( std::vector<RemappingInfo>::iterator it=map.begin();
					it!=map.end(); ++it )
			{
				transferAttribData<GEO_PointList>( points, result, interpolation, attr, attrRef, &*it );
			}
		}
		else // no remapping - use regular transfer
		{
			transferAttribData<GEO_PointList>( points, result, interpolation, attr, attrRef );
		}
	}
}

void FromHoudiniGeometryConverter::transferPrimitiveAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PrimList &primitives, AttributeRemapping &attribute_remap ) const
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

		// check for remapping information for this attribute
		std::string attr_name(attr->getName());
		if ( attribute_remap[RemappingInfo::Primitive].count(attr_name)==1 )
		{
			std::vector<RemappingInfo> &map = attribute_remap[RemappingInfo::Primitive][attr_name];
			for ( std::vector<RemappingInfo>::iterator it=map.begin();
					it!=map.end(); ++it )
			{
				transferAttribData<GEO_PrimList>( primitives, result, interpolation, attr, attrRef, &*it );
			}
		}
		else // no remapping - use regular transfer
		{
			transferAttribData<GEO_PrimList>( primitives, result, interpolation, attr, attrRef );
		}
	}
}

void FromHoudiniGeometryConverter::transferVertexAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const VertexList &vertices ) const
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

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const GU_DetailHandle &handle, IECore::TypeId resultType )
{
	const TypesToFnsMap *m = typesToFns();

	TypesToFnsMap::const_iterator it = m->find( Types( resultType ) );
	if ( it != m->end() )
	{
		return it->second( handle );
	}
	
	return 0;
}

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const SOP_Node *sop, IECore::TypeId resultType )
{
	return create( handle( sop ), resultType );
}

void FromHoudiniGeometryConverter::registerConverter( IECore::TypeId resultType, bool isDefault, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( resultType ), creator ) );
	
	if ( isDefault )
	{
		m->insert( TypesToFnsMap::value_type( Types( IECore::InvalidTypeId ), creator ) );
	}
}

FromHoudiniGeometryConverter::TypesToFnsMap *FromHoudiniGeometryConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

FromHoudiniGeometryConverter::Types::Types( IECore::TypeId result )
	: resultType( result )
{
}

bool FromHoudiniGeometryConverter::Types::operator < ( const Types &other ) const
{
	return resultType < other.resultType;
}
