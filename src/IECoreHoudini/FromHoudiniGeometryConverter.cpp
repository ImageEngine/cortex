//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_Version.h"
#include "UT/UT_WorkArgs.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreScene;
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
		"Performs automated conversion of Houdini Attributes to standard PrimitiveVariables (i.e. rest->Pref ; Cd->Cs)",
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

	return doDetailConversion( geo, operands.get() );
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
		UT_String remapString( remapStrings( i ) );
		remapString.tokenize( workArgs, ":" );
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
				info.interpolation = IECoreScene::PrimitiveVariable::Vertex;
			}
			else if ( classStr == "v" )
			{
				info.interpolation = IECoreScene::PrimitiveVariable::Varying;
			}
			else if ( classStr == "u" )
			{
				info.interpolation = IECoreScene::PrimitiveVariable::Uniform;
			}
			else if ( classStr == "c" )
			{
				info.interpolation = IECoreScene::PrimitiveVariable::Constant;
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
	const GU_Detail *geo, IECoreScene::Primitive *result, const CompoundObject *operands,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{

#if UT_MAJOR_VERSION_INT < 15

	// add position (this can't be done as a regular attrib because it would be V4fVectorData)
	GA_Range pointRange = geo->getPointRange();
	std::vector<Imath::V3f> pData( pointRange.getEntries() );
	for ( GA_Iterator it=pointRange.begin(); !it.atEnd(); ++it )
	{
		pData[it.getIndex()] = IECore::convert<Imath::V3f>( geo->getPos3( it.getOffset() ) );
	}

	result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( pData, GeometricData::Point ) );

#endif

	// get RI remapping information from the detail
	AttributeMap pointAttributeMap;
	AttributeMap primitiveAttributeMap;
	remapAttributes( geo, pointAttributeMap, primitiveAttributeMap );

	// build the attribute filter
	UT_String p( "P" );
	UT_String filter( operands->member<StringData>( "attributeFilter" )->readable() );
	UT_StringMMPattern attribFilter;
	// force P and prevent name
	// filter += " ^name";
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
		if( name.equal( "uv" ) )
		{
			IntVectorDataPtr indexData = new IntVectorData;
			std::vector<int> &indices = indexData->writable();

			// uvs are V3f in Houdini, so we must extract individual components
			FloatVectorDataPtr uData = extractData<FloatVectorData>( attr, range, 0 );
			FloatVectorDataPtr vData = extractData<FloatVectorData>( attr, range, 1 );
			const std::vector<float> &u = uData->readable();
			const std::vector<float> &v = vData->readable();

			V2fVectorDataPtr uvData = new V2fVectorData;
			uvData->setInterpretation( GeometricData::UV );
			std::vector<Imath::V2f> &uvs = uvData->writable();
			uvs.reserve( u.size() );
			for( size_t i = 0, nextIndex = 0; i < u.size(); ++i )
			{
				Imath::V2f uv( u[i], v[i] );

				const auto uvIt = std::find( uvs.begin(), uvs.end(), uv );
				if( uvIt != uvs.end() )
				{
					indices.push_back( (int)(uvIt - uvs.begin()) );
				}
				else
				{
					indices.push_back( (int)nextIndex );
					uvs.push_back( uv );
					++nextIndex;
				}
			}

			result->variables["uv"] = PrimitiveVariable( interpolation, uvData, indexData );

			continue;
		}

		// check for remapping information for this attribute
		if ( attributeMap.count( name.buffer() ) == 1 )
		{
			std::vector<RemapInfo> &map = attributeMap[name.buffer()];
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
	IECoreScene::Primitive *result, IECoreScene::PrimitiveVariable::Interpolation interpolation,
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

	IntVectorDataPtr indices = nullptr;

	switch ( attrRef.getStorageClass() )
	{
		case GA_STORECLASS_FLOAT :
		{
			if( !attr->getAIFTuple() )
			{
				// not supporting variable lists
				return;
			}
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

#if UT_MAJOR_VERSION_INT >= 15

									// special case for rest/Pref since Houdini considers rest Numeric
									// but Cortex is expecting it to be Point.
									if ( attr->getName().equal( "rest" ) || attr->getName().equal( "Pref" ) )
									{
										V3fVectorData *restData = IECore::runTimeCast<V3fVectorData>( dataPtr.get() );
										restData->setInterpretation( IECore::GeometricData::Point );
									}

#endif
									break;
								}

							}
							break;
						}
					}
					break;
				}
				case 4 :
				{
					if( attr->getTypeInfo() == GA_TYPE_QUATERNION || ( !strcmp( attr->getName(), "orient" ) && attr->getTypeInfo() == GA_TYPE_VOID ) )
					{
						dataPtr = extractData<QuatfVectorData>( attr, range, elementIndex );
					}
					break;
				}
				case 9 :
				{
					dataPtr = extractData<M33fVectorData>( attr, range, elementIndex );
					break;
				}
				case 16 :
				{
					dataPtr = extractData<M44fVectorData>( attr, range, elementIndex );
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
			if( !attr->getAIFTuple() )
			{
				// not supporting variable lists
				return;
			}
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
			dataPtr = extractStringVectorData( attr, range, indices );
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
		result->variables[ varName ] = PrimitiveVariable( varInterpolation, dataPtr, indices );
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
				if( !attr->getAIFTuple() )
				{
					// not supporting variable lists
					continue;
				}
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
					case 9:
					{
						dataPtr = extractData<M33fData>( attr );
						break;
					}
					case 16:
					{
						dataPtr = extractData<M44fData>( attr );
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
				if( !attr->getAIFTuple() )
				{
					// not supporting variable lists
					continue;
				}
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


	size_t numStrings = 0;
	std::vector<std::string> strings;
	const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
	for ( GA_AIFSharedStringTuple::iterator it=tuple->begin( attr ); !it.atEnd(); ++it )
	{
		strings.push_back( it.getString() );
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
		adjustedHandles[ handles(i) ] = i;
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
				strings.push_back( "" );
				adjustedDefault = true;
			}

			indices[i] = numStrings;
		}
		else
		{
			indices[i] = adjustedHandles[index];
		}
	}

	// The order of "strings" is not guaranteed, and can be unstable
	// from frame to frame in certain situations, so we sort them
	// alphabetically and update the indices accordingly:

	// find a permutation that orders strings alphabetically:
	std::vector<int> alphabeticalOrdering;
	for( size_t i=0; i < strings.size(); ++i )
	{
		alphabeticalOrdering.push_back( i );
	}
	struct Comparator
	{
		Comparator( const std::vector<std::string> &s ) : strings( s )
		{
		}

		const std::vector<std::string> &strings;
		bool operator()( int a, int b )
		{
			return strings[a] < strings[b];
		}
	};
	std::sort(
		alphabeticalOrdering.begin(),
		alphabeticalOrdering.end(),
		Comparator(strings)
	);

	// find inverse:
	std::vector<int> inverseAlphabeticalOrdering( strings.size() );
	for( size_t i=0; i < alphabeticalOrdering.size(); ++i )
	{
		inverseAlphabeticalOrdering[ alphabeticalOrdering[i] ] = i;
	}

	// apply permutation:
	for( size_t i=0; i < indexData->readable().size(); ++i )
	{
		indices[i] = inverseAlphabeticalOrdering[ indices[i] ];
	}
	std::vector<std::string> &dest = data->writable();
	dest.resize( alphabeticalOrdering.size() );
	for( size_t i=0; i < alphabeticalOrdering.size(); ++i )
	{
		dest[ i ] = strings[ alphabeticalOrdering[i] ];
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
			newGeo->incrementMetaCacheCount();
			GU_DetailHandle newHandle;
			newHandle.allocateAndSet( newGeo );
			return newHandle;
		}
	}

	return GU_DetailHandle();
}

template <>
IECore::QuatfVectorDataPtr FromHoudiniGeometryConverter::extractData<IECore::QuatfVectorData>( const GA_Attribute *attr, const GA_Range &range, int elementIndex ) const
{
	QuatfVectorDataPtr data = new QuatfVectorData;
	data->writable().resize( range.getEntries() );
	QuatfVectorData::BaseType *dest = data->baseWritable();

	if ( elementIndex == -1 )
	{
		attr->getAIFTuple()->getRange( attr, range, dest );
	}
	else
	{
		attr->getAIFTuple()->getRange( attr, range, dest, elementIndex, 1 );
	}

	// rearrange quaternion components: houdini stores them as "i,j,k,s", Imath
	// stores them as "s,i,j,k":
	for( std::vector<Imath::Quatf>::iterator it = data->writable().begin(); it != data->writable().end(); ++it )
	{
		*it = Imath::Quatf( (*it)[3], (*it)[0], (*it)[1], (*it)[2] );
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
			if ( *typeIt == IECore::InvalidTypeId || it->first.resultType == InvalidTypeId || *typeIt == it->first.resultType || find( derivedTypes.begin(), derivedTypes.end(), it->first.resultType ) != derivedTypes.end() )
			{
				if ( handle.isNull() && it->first.resultType != InvalidTypeId )
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
