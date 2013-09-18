//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "boost/lexical_cast.hpp"

#include "CH/CH_Manager.h"
#include "GEO/GEO_AttributeHandle.h"
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_WorkArgs.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniGeometryConverter );

FromHoudiniGeometryConverter::FromHoudiniGeometryConverter( const GU_DetailHandle &handle, const std::string &description )
	: FromHoudiniConverter( description ), m_geoHandle( handle )
{
	constructCommon();
}

FromHoudiniGeometryConverter::FromHoudiniGeometryConverter( const SOP_Node *sop, const std::string &description )
	: FromHoudiniConverter( description )
{
	m_geoHandle = handle( sop );
	
	constructCommon();
}

FromHoudiniGeometryConverter::~FromHoudiniGeometryConverter()
{
}

void FromHoudiniGeometryConverter::constructCommon()
{
	m_attributeFilterParameter = new StringParameter(
		"attributeFilter",
		"A list of attribute names to convert, if they exist. Uses Houdini matching syntax. P will always be converted",
		"*"
	);
	
	m_convertStandardAttributesParameter = new BoolParameter(
		"convertStandardAttributes",
		"Performs automated conversion of Houdini Attributes to standard PrimitiveVariables (i.e. rest->Pref ; Cd->Cs ; uv->s,t)",
		true
	);
	
	parameters()->addParameter( m_attributeFilterParameter );
	parameters()->addParameter( m_convertStandardAttributesParameter );
}

const GU_DetailHandle FromHoudiniGeometryConverter::handle( const SOP_Node *sop )
{
	// create the work context
	OP_Context context;
	context.setTime( CHgetEvalTime() );
	
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
	
	return doDetailConversion( geo, operands );
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
	
	UT_StringArray remapStrings;
	UT_IntArray remapHandles;
	tuple->extractStrings( remapAttr, remapStrings, remapHandles );
	
	for ( size_t i=0; i < (size_t)remapStrings.entries(); ++i )
	{
		RemapInfo info;
		
		// split up our rixlate string
		UT_WorkArgs workArgs;
		std::vector<std::string> tokens;
		remapStrings( i ).tokenize( workArgs, ":" );
		workArgs.toStringVector( tokens );

		// not enough elements!
		if ( tokens.size() < 4 )
		{
			continue;
		}

		// our data types
		UT_WorkArgs dataWorkArgs;
		std::vector<std::string> dataTokens;
		UT_String dataString( tokens[3] );
		dataString.tokenize( dataWorkArgs, "_" );
		dataWorkArgs.toStringVector( dataTokens );

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
	const GU_Detail *geo, IECore::Primitive *result, const CompoundObject *operands,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{
	// add position (this can't be done as a regular attrib because it would be V4fVectorData)
	GA_Range pointRange = geo->getPointRange();
	std::vector<Imath::V3f> pData( pointRange.getEntries() );
	for ( GA_Iterator it=pointRange.begin(); !it.atEnd(); ++it )
	{
		pData[it.getIndex()] = IECore::convert<Imath::V3f>( geo->getPos3( it.getOffset() ) );
	}

	result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( pData, GeometricData::Point ) );
	
	// get RI remapping information from the detail
	AttributeMap pointAttributeMap;
	AttributeMap primitiveAttributeMap;
	remapAttributes( geo, pointAttributeMap, primitiveAttributeMap );
	
	// build the attribute filter
	UT_String p( "P" );
	UT_String filter( operands->member<StringData>( "attributeFilter" )->readable() );
	UT_StringMMPattern attribFilter;
	// force P and prevent name
	filter += " ^name";
	if ( !p.match( filter ) )
	{
		filter += " P";
	}
	attribFilter.compile( filter );
	
	// add detail attribs	
	if ( result->variableSize( detailInterpolation ) == 1 )
	{
		transferDetailAttribs( geo, attribFilter, result, detailInterpolation );
	}
	
	// add point attribs
	if ( result->variableSize( pointInterpolation ) == (unsigned)geo->getNumPoints() )
	{
		transferElementAttribs( geo, geo->getPointRange(), geo->pointAttribs(), attribFilter, pointAttributeMap, result, pointInterpolation );
	}
	
	// add primitive attribs
	size_t numPrims = geo->getNumPrimitives();
	if ( result->variableSize( primitiveInterpolation ) == numPrims )
	{
		transferElementAttribs( geo, geo->getPrimitiveRange(), geo->primitiveAttribs(), attribFilter, primitiveAttributeMap, result, primitiveInterpolation );
	}
	
	// add vertex attribs
	size_t numVerts = geo->getNumVertices();
	GA_Range primRange = geo->getPrimitiveRange();
	if ( geo->vertexAttribs().entries() && result->variableSize( vertexInterpolation ) == numVerts )
	{
		const GA_PrimitiveList &primitives = geo->getPrimitiveList();
		
		GA_OffsetList offsets;
		offsets.reserve( numVerts );
		for ( GA_Iterator it=primRange.begin(); !it.atEnd(); ++it )
		{
			const GA_Primitive *prim = primitives.get( it.getOffset() );
			size_t numPrimVerts = prim->getVertexCount();
			for ( size_t v=0; v < numPrimVerts; v++ )
			{
				if ( prim->getTypeId() == GEO_PRIMPOLY )
				{
					offsets.append( prim->getVertexOffset( numPrimVerts - 1 - v ) );
				}
				else
				{
					offsets.append( prim->getVertexOffset( v ) );
				}
			}
		}
		
		GA_Range vertRange( geo->getVertexMap(), offsets );
		
		AttributeMap defaultMap;
		transferElementAttribs( geo, vertRange, geo->vertexAttribs(), attribFilter, defaultMap, result, vertexInterpolation );
	}
}

void FromHoudiniGeometryConverter::transferElementAttribs( const GU_Detail *geo, const GA_Range &range, const GA_AttributeDict &attribs, const UT_StringMMPattern &attribFilter, AttributeMap &attributeMap, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
{
	bool convertStandardAttributes = m_convertStandardAttributesParameter->getTypedValue();
	
	for ( GA_AttributeDict::iterator it=attribs.begin( GA_SCOPE_PUBLIC ); it != attribs.end(); ++it )
	{
		GA_Attribute *attr = it.attrib();
		if ( !attr )
		{
			continue;
		}
		
		const GA_ROAttributeRef attrRef( attr );
		if ( attrRef.isInvalid() )
		{
			continue;
		}
		
		UT_String name( attr->getName() );
		if ( !name.multiMatch( attribFilter ) )
		{
			continue;
		}
		
		// special case for uvs
		if ( convertStandardAttributes && name.equal( "uv" ) )
		{
			FloatVectorDataPtr sData = extractData<FloatVectorData>( attr, range, 0 );
			FloatVectorDataPtr tData = extractData<FloatVectorData>( attr, range, 1 );
			std::vector<float> &tValues = tData->writable();
			for ( size_t i=0; i < tValues.size(); ++i )
			{
				tValues[i] = 1 - tValues[i];
			}
			
			result->variables["s"] = PrimitiveVariable( interpolation, sData );
			result->variables["t"] = PrimitiveVariable( interpolation, tData );
			
			continue;
		}
		
		// check for remapping information for this attribute
		if ( attributeMap.count( attr->getName() ) == 1 )
		{
			std::vector<RemapInfo> &map = attributeMap[attr->getName()];
			for ( std::vector<RemapInfo>::iterator rIt=map.begin(); rIt != map.end(); ++rIt )
			{
				transferAttribData( result, interpolation, attrRef, range, &*rIt );
			}
		}
		else
		{
			transferAttribData( result, interpolation, attrRef, range );
		}
	}
}

void FromHoudiniGeometryConverter::transferAttribData(
	IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation,
	const GA_ROAttributeRef &attrRef, const GA_Range &range, const RemapInfo *remapInfo
) const
{
	DataPtr dataPtr = 0;

	// we use this initial value to indicate we don't have a remapping so just
	// guess what destination type to use.
	IECore::TypeId varType = IECore::InvalidTypeId;
	int elementIndex = -1;
	if ( remapInfo )
	{
		varType = remapInfo->type;
		elementIndex = remapInfo->elementIndex;
	}
	
	const GA_Attribute *attr = attrRef.getAttribute();
	
	switch ( attrRef.getStorageClass() )
	{
		case GA_STORECLASS_FLOAT :
		{
			switch ( attr->getTupleSize() )
			{
				case 1 :
				{
					dataPtr = extractData<FloatVectorData>( attr, range );
					break;
				}
				case 2 :
				{
					// it can be either a single float (sub-component), or (default) just a V2f
					switch( varType )
					{
						case FloatVectorDataTypeId :
						{
							dataPtr = extractData<FloatVectorData>( attr, range, elementIndex );
							break;
						}
						default :
						{
							dataPtr = extractData<V2fVectorData>( attr, range );
							break;
						}
					}
					break;
				}
				case 3 :
				{
					// it can be either a single float (sub-component), Color3f or (default) just a V3f
					switch( varType )
					{
						case FloatVectorDataTypeId :
						{
							dataPtr = extractData<FloatVectorData>( attr, range, elementIndex );
							break;
						}
						case Color3fVectorDataTypeId :
						{
							dataPtr = extractData<Color3fVectorData>( attr, range );
							break;
						}
						default :
						{
							switch( attr->getTypeInfo() )
							{
								case GA_TYPE_COLOR :
								{
									dataPtr = extractData<Color3fVectorData>( attr, range );
									break;
								}
								default :
								{
									dataPtr = extractData<V3fVectorData>( attr, range );
									break;
								}

							}
							break;
						}
					}
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case GA_STORECLASS_INT :
 		{
			switch ( attr->getTupleSize() )
			{
				case 1 :
				{
					dataPtr = extractData<IntVectorData>( attr, range );
					break;
				}
				case 2 :
				{
					dataPtr = extractData<V2iVectorData>( attr, range );
					break;
				}
				case 3 :
				{
					dataPtr = extractData<V3iVectorData>( attr, range );
					break;
				}
				default :
				{
					break;
				}
			}
			break;
 		}
		case GA_STORECLASS_STRING :
 		{
			/// \todo: replace this with IECore::IndexedData once it exists...
			IntVectorDataPtr indexDataPtr = 0;
			dataPtr = extractStringVectorData( attr, range, indexDataPtr );
			if ( indexDataPtr )
			{
				std::string name( attr->getName() );
				if ( remapInfo )
				{
					name = remapInfo->name;
					interpolation = remapInfo->interpolation;
				}
				
				name = name + "Indices";
				result->variables[name] = PrimitiveVariable( interpolation, indexDataPtr );
				interpolation = PrimitiveVariable::Constant;
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
		std::string varName( attr->getName() );
		PrimitiveVariable::Interpolation varInterpolation = interpolation;
		
		// remap our name and interpolation
		if ( remapInfo )
		{
			varName = remapInfo->name;
			varInterpolation = remapInfo->interpolation;
		}
		
		if ( m_convertStandardAttributesParameter->getTypedValue() )
		{
			varName = processPrimitiveVariableName( varName );
		}
		
		// add the primitive variable to our result
		result->variables[ varName ] = PrimitiveVariable( varInterpolation, dataPtr );
	}
}

void FromHoudiniGeometryConverter::transferDetailAttribs( const GU_Detail *geo, const UT_StringMMPattern &attribFilter, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
{
	const GA_AttributeDict &attribs = geo->attribs();
	
	for ( GA_AttributeDict::iterator it=attribs.begin(); it != attribs.end(); ++it )
	{
		GA_Attribute *attr = it.attrib();
		if ( !attr )
		{
			continue;
		}
		
		const GA_ROAttributeRef attrRef( attr );
		if ( attrRef.isInvalid() )
		{
			continue;
		}
		
		UT_String name( attr->getName() );
		if ( !name.multiMatch( attribFilter ) )
		{
			continue;
		}
		
		DataPtr dataPtr = 0;
		
		switch ( attrRef.getStorageClass() )
		{
			case GA_STORECLASS_FLOAT :
			{
				switch ( attr->getTupleSize() )
				{
					case 1 :
					{
						dataPtr = extractData<FloatData>( attr );
						break;
					}
					case 2 :
					{
						dataPtr = extractData<V2fData>( attr );
						break;
					}
					case 3 :
					{
						switch( attr->getTypeInfo() )
						{
							case GA_TYPE_COLOR :
							{
								dataPtr = extractData<Color3fData>( attr );
								break;
							}
							default :
							{
								dataPtr = extractData<V3fData>( attr );
								break;
							}

						}
						break;
					}
					default :
					{
						break;
					}
				}
				break;
			}
			case GA_STORECLASS_INT :
 			{
				switch ( attr->getTupleSize() )
				{
					case 1 :
					{
						dataPtr = extractData<IntData>( attr );
						break;
					}
					case 2 :
					{
						dataPtr = extractData<V2iData>( attr );
						break;
					}
					case 3 :
					{
						dataPtr = extractData<V3iData>( attr );
						break;
					}
					default :
					{
						break;
					}
				}
				break;
 			}
			case GA_STORECLASS_STRING :
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
	
	UT_IntArray handles;
	tuple->extractHandles( attr, handles );
	std::map<int, int> adjustedHandles;
	size_t numHandles = handles.entries();
	for ( size_t i=0; i < numHandles; i++ )
	{
		adjustedHandles[ handles[i] ] = i;
	}
	
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
			indices[i] = adjustedHandles[index];
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

const std::string FromHoudiniGeometryConverter::processPrimitiveVariableName( const std::string &name ) const
{
	/// \todo: This should probably be some formal static map. Make sure to update ToHoudiniGeometryConverter as well.
	if ( name == "Cd" )
	{
		return "Cs";
	}
	else if ( name == "Alpha" )
	{
		return "Os";
	}
	else if ( name == "rest" )
	{
		return "Pref";
	}
	else if ( name == "pscale" )
	{
		return "width";
	}
	
	return name;
}

GU_DetailHandle FromHoudiniGeometryConverter::extract( const GU_Detail *geo, const UT_StringMMPattern &nameFilter )
{
	GA_ROAttributeRef nameAttrRef = geo->findStringTuple( GA_ATTRIB_PRIMITIVE, "name" );
	if ( nameAttrRef.isValid() )
	{
		const GA_Attribute *nameAttr = nameAttrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		
		GA_OffsetList offsets;
		GA_Range primRange = geo->getPrimitiveRange();
		for ( GA_Iterator it = primRange.begin(); !it.atEnd(); ++it )
		{
			const char *currentName = tuple->getString( nameAttr, it.getOffset(), 0 );
			if ( UT_String( currentName ).multiMatch( nameFilter ) )
			{
				offsets.append( it.getOffset() );
			}
		}
		
		if ( offsets.entries() )
		{
			GU_Detail *newGeo = new GU_Detail();
			GA_Range matchPrims( geo->getPrimitiveMap(), offsets );
			newGeo->mergePrimitives( *geo, matchPrims );
			GU_DetailHandle newHandle;
			newHandle.allocateAndSet( newGeo );
			return newHandle;
		}
	}
	
	return GU_DetailHandle();
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
			if ( *typeIt == IECore::InvalidTypeId || *typeIt == it->first.resultType || find( derivedTypes.begin(), derivedTypes.end(), it->first.resultType ) != derivedTypes.end() )
			{
				if ( handle.isNull() )
				{
					// it works, therfor its good enough, since we have no handle to judge Convertability
					return it->second.first( GU_DetailHandle() );
				}
				
				Convertability current = it->second.second( handle );
				if ( current && current < best )
				{
					best = current;
					bestIt = it;
				}
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

FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::create( const SOP_Node *sop, const std::string &nameFilter, IECore::TypeId resultType )
{
	// try to reduce the geo to fit the name
	GU_DetailHandleAutoReadLock readHandle( handle( sop ) );
	if ( const GU_Detail *geo = readHandle.getGdp() )
	{
		UT_StringMMPattern filter;
		filter.compile( nameFilter.c_str() );
		GU_DetailHandle newHandle = extract( geo, filter );
		if ( !newHandle.isNull() )
		{
			return create( newHandle, resultType );
		}
	}
	
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

void FromHoudiniGeometryConverter::supportedTypes( std::set<IECore::TypeId> &types )
{
	types.clear();
	
	const TypesToFnsMap *m = typesToFns();
	for ( TypesToFnsMap::const_iterator it=m->begin(); it != m->end(); it ++ )
	{
		types.insert( it->first.resultType );
	}
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
