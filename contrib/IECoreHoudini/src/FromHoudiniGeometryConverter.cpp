//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "boost/tokenizer.hpp"

#include "GEO/GEO_AttributeHandle.h"

#include "IECore/CompoundObject.h"

#include "CoreHoudini.h"
#include "Convert.h"
#include "FromHoudiniGeometryConverter.h"

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

const GU_DetailHandle &FromHoudiniGeometryConverter::handle() const
{
	return m_geoHandle;
}

ObjectPtr FromHoudiniGeometryConverter::doConversion( ConstCompoundObjectPtr operands ) const
{
	GU_DetailHandleAutoReadLock readHandle( m_geoHandle );
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return 0;
	}
	
	return doPrimitiveConversion( geo );
}

/// Create a remapping matrix of names, types and interpolation classes for all attributes specified in the 'rixlate' detail attribute.
void FromHoudiniGeometryConverter::remapAttributes( const GU_Detail *geo, AttributeMap &pointAttributeMap, AttributeMap &primitiveAttributeMap ) const
{
	const GA_ROAttributeRef remapRef = geo->findGlobalAttribute( "rixlate" );
	if ( remapRef.isInvalid() )
	{
		return;
	}
	
	const GA_Attribute *remapAttr = remapRef.getAttribute();
	const GA_AIFSharedStringTuple *tuple = remapAttr->getAIFSharedStringTuple();
	if ( !tuple )
	{
		return;
	}

	for ( GA_AIFSharedStringTuple::iterator it=tuple->begin( remapAttr ); !it.atEnd(); ++it )
	{
		RemapInfo info;

		// split up our rixlate string
		typedef boost::char_separator< char > Sep;
		typedef boost::tokenizer< Sep > Tokeniser;
		std::vector<std::string> tokens;
		Tokeniser attributeSplit( std::string( it.getString() ), Sep( ":" ) );
		for ( Tokeniser::iterator it=attributeSplit.begin(); it != attributeSplit.end(); ++it )
		{
			tokens.push_back( *it );
		}

		// not enough elements!
		if ( tokens.size() < 4 )
		{
			continue;
		}

		// our data types
		std::vector<std::string> dataTokens;
		Tokeniser dataSplit( tokens[3], Sep("_") );
		for ( Tokeniser::iterator it=dataSplit.begin(); it != dataSplit.end(); ++it )
		{
			dataTokens.push_back( *it );
		}

		if ( dataTokens.size() == 2 ) // we need both class & type!
		{
			// our interpolation type
			std::string classStr = dataTokens[0];
			if ( classStr == "vtx" )
			{
				info.interpolation = IECore::PrimitiveVariable::Vertex;
			}
			else if ( classStr == "v" )
			{
				info.interpolation = IECore::PrimitiveVariable::Varying;
			}
			else if ( classStr == "u" )
			{
				info.interpolation = IECore::PrimitiveVariable::Uniform;
			}
			else if ( classStr == "c" )
			{
				info.interpolation = IECore::PrimitiveVariable::Constant;
			}

			// our types
			std::string typeStr = dataTokens[1];
			if ( typeStr == "float" )
			{
				info.type = IECore::FloatVectorDataTypeId;
			}
			else if ( typeStr == "color" )
			{
				info.type = IECore::Color3fVectorDataTypeId;
			}
			else if ( typeStr == "point" )
			{
				info.type = IECore::V3fVectorDataTypeId;
			}
			else if ( typeStr == "vector" )
			{
				info.type = IECore::V3fVectorDataTypeId;
			}
			else if ( typeStr == "normal" )
			{
				info.type = IECore::V3fVectorDataTypeId;
			}
			else if ( typeStr == "string" )
			{
				info.type = IECore::StringVectorDataTypeId;
			}
		}

		info.name = tokens[2];
		info.elementIndex = ( tokens.size() == 5 ) ? boost::lexical_cast<int>( tokens[4] ) : 0;

		if ( tokens[0] == "prim" )
		{
			primitiveAttributeMap[ tokens[1] ].push_back( info );
		}
		else
		{
			pointAttributeMap[ tokens[1] ].push_back( info );
		}
	}
}

void FromHoudiniGeometryConverter::transferAttribs(
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
		pData[i] = IECore::convert<Imath::V3f>( point->getPos() );
	}
	
	result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( pData ) );
	
	// get RI remapping information from the detail
	AttributeMap pointAttributeMap;
	AttributeMap primitiveAttributeMap;
	remapAttributes( geo, pointAttributeMap, primitiveAttributeMap );
	
	// add detail attribs	
	if ( result->variableSize( detailInterpolation ) == 1 )
	{
		transferDetailAttribs( geo, result, detailInterpolation );
	}
	
	// add point attribs
	if ( result->variableSize( pointInterpolation ) == numPoints )
	{
		transferPointAttribs( geo, result, pointInterpolation, points, pointAttributeMap );
	}
	
	// add primitive attribs
	const GEO_PrimList &primitives = geo->primitives();
	size_t numPrims = primitives.entries();
	
	if ( result->variableSize( primitiveInterpolation ) == numPrims )
	{
		transferPrimitiveAttribs( geo, result, primitiveInterpolation, primitives, primitiveAttributeMap );
	}
	
	// add vertex attribs
	size_t numVerts = 0;
	
	for ( size_t i=0; i < numPrims; i++ )
	{
		numVerts += primitives[i]->getVertexCount();
	}
	
	if ( geo->vertexAttribs().length() && result->variableSize( vertexInterpolation ) == numVerts )
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
					case 1 :
					{
						dataPtr = extractData<FloatData>( attribs, attrRef );
						break;
					}
					case 2 :
					{
						dataPtr = extractData<V2fData>( attribs, attrRef );
						break;
					}
					case 3 :
					{
						dataPtr = extractData<V3fData>( attribs, attrRef );
						break;
					}
					default :
					{
						break;
					}
				}
				break;
			}
			case GB_ATTRIB_INT :
 			{
				unsigned dimensions = attr->getSize() / sizeof( float );
				switch ( dimensions )
				{
					case 1 :
					{
						dataPtr = extractData<IntData>( attribs, attrRef );
						break;
					}
					case 2 :
					{
						dataPtr = extractData<V2iData>( attribs, attrRef );
						break;
					}
					case 3 :
					{
						dataPtr = extractData<V3iData>( attribs, attrRef );
						break;
					}
					default :
					{
						break;
					}
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
			case GB_ATTRIB_INDEX :
 			{
				dataPtr = extractStringData( geo, attr );
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

void FromHoudiniGeometryConverter::transferPointAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PointList &points, AttributeMap &attributeMap ) const
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
		if ( attributeMap.count( attr->getName() ) == 1 )
		{
			std::vector<RemapInfo> &map = attributeMap[attr->getName()];
			for ( std::vector<RemapInfo>::iterator rIt=map.begin(); rIt != map.end(); ++rIt )
			{
				transferAttribData<GEO_PointList>( points, result, interpolation, attr, attrRef, &*it );
			}
		}
		else
		{
			transferAttribData<GEO_PointList>( points, result, interpolation, attr, attrRef );
		}
	}
}

void FromHoudiniGeometryConverter::transferPrimitiveAttribs( const GU_Detail *geo, Primitive *result, PrimitiveVariable::Interpolation interpolation, const GEO_PrimList &primitives, AttributeMap &attributeMap ) const
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
		if ( attributeMap.count( attr->getName() ) == 1 )
		{
			std::vector<RemapInfo> &map = attributeMap[attr->getName()];
			for ( std::vector<RemapInfo>::iterator rIt=map.begin(); rIt != map.end(); ++rIt )
			{
				transferAttribData<GEO_PrimList>( primitives, result, interpolation, attr, attrRef, &*it );
			}
		}
		else
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

DataPtr FromHoudiniGeometryConverter::extractStringVectorData( const GA_Attribute *attr, const GA_Range &range, IntVectorDataPtr &indexData ) const
{
	StringVectorDataPtr data = new StringVectorData();
	
	std::vector<std::string> &dest = data->writable();
	
	size_t numStrings = 0;
	const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
	for ( GA_AIFSharedStringTuple::iterator it=tuple->begin( attr ); !it.atEnd(); ++it )
	{
		dest.push_back( it.getString() );
		numStrings++;
	}
	
	indexData = new IntVectorData();
	std::vector<int> &indexContainer = indexData->writable();
	indexContainer.resize( range.getEntries() );
	int *indices = indexData->baseWritable();
	
	size_t i = 0;
	bool adjustedDefault = false;
	for ( GA_Iterator it=range.begin(); !it.atEnd(); ++it, ++i )
	{
		const int index = tuple->getHandle( attr, it.getOffset() );
		
		if ( index < 0 )
		{
			if ( !adjustedDefault )
			{
				dest.push_back( "" );
				adjustedDefault = true;
			}
			
			indices[i] = numStrings;
		}
		else
		{
			indices[i] = index;
		}
	}
	
	return data;
}

DataPtr FromHoudiniGeometryConverter::extractStringData( const GU_Detail *geo, const GA_Attribute *attr ) const
{
	StringDataPtr data = new StringData();
	
	const char *src = attr->getAIFStringTuple()->getString( attr, 0 );
	if ( src )
	{
		data->writable() = src;
	}
	
	return data;
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const GU_DetailHandle &handle, IECore::TypeId resultType )
{
	std::set<IECore::TypeId> types;
	types.insert( resultType );
	
	return create( handle, types );
}

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const GU_DetailHandle &handle, const std::set<IECore::TypeId> &resultTypes )
{
	const TypesToFnsMap *m = typesToFns();
	
	Convertability best = InvalidValue;
	TypesToFnsMap::const_iterator bestIt = m->end();
	
	for ( std::set<IECore::TypeId>::iterator typeIt=resultTypes.begin(); typeIt != resultTypes.end(); typeIt++ )
	{
		const std::set<IECore::TypeId> &derivedTypes = RunTimeTyped::derivedTypeIds( *typeIt );

		// find the best possible converter
		for ( TypesToFnsMap::const_iterator it=m->begin(); it != m->end(); it ++ )
		{
			if ( *typeIt != IECore::InvalidTypeId && *typeIt != it->first.resultType && find( derivedTypes.begin(), derivedTypes.end(), it->first.resultType ) == derivedTypes.end() )
			{
				// we want something specific, but this converter won't give it to us, nor something that derives from it
				continue;
			}

			Convertability current = it->second.second( handle );
			if ( current && current < best )
			{
				best = current;
				bestIt = it;
			}
		}
	}

	// return the best converter if it was found
	if ( bestIt != m->end() )
	{
		return bestIt->second.first( handle );
	}

	// there were no suitable converters
	return 0;
}

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const SOP_Node *sop, IECore::TypeId resultType )
{
	return create( handle( sop ), resultType );
}

void FromHoudiniGeometryConverter::registerConverter( IECore::TypeId resultType, CreatorFn creator, ConvertabilityFn canConvert )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( resultType ), std::pair<CreatorFn, ConvertabilityFn>( creator, canConvert ) ) );
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
