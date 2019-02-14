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

#include "tbb/tbb.h"

#include "boost/functional/hash.hpp"
#include "boost/lexical_cast.hpp"

#include "CH/CH_Manager.h"
#include "GA/GA_Names.h"
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_Version.h"
#include "UT/UT_WorkArgs.h"

#if UT_MAJOR_VERSION_INT >= 17

#include "UT/UT_StdUtil.h"

#endif

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECoreScene/private/PrimitiveVariableAlgos.h"
#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

#include <unordered_set>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniGeometryConverter );

namespace
{

/// Generate a hash for Imath::V2f to allow it to be used in a std::unordered_map.
/// Accepted wisdom says we should define a template specialisation of hash in the std namespace
/// but perhaps that might be better done by the imath headers themselves?
class Hasher
{
	public:

		size_t operator()( const Imath::V2f &v ) const
		{
			size_t result = 0;
			boost::hash_combine( result, v.x );
			boost::hash_combine( result, v.y );
			return result;
		}
};

const IECore::InternedString g_Tags( "tags" );
IECore::InternedString g_uniqueTags( "__uniqueTags" );
const UT_String g_tagGroupPrefix( "ieTag_" );

} // namepspace

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

	m_preserveNameParameter  = new BoolParameter(
		"preserveName",
		"Keep the name attribute in conversion",
		false
	);

	m_weldUVsParameter = new BoolParameter(
		"weldUVs",
		"Generate UV indices by de-duplicating identical values. This can be desirable for subdivision rendering or conversion to other DCCs (eg Maya).",
		true
	);

	parameters()->addParameter( m_attributeFilterParameter );
	parameters()->addParameter( m_convertStandardAttributesParameter );
	parameters()->addParameter( m_preserveNameParameter );
	parameters()->addParameter( m_weldUVsParameter );
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

	// build the attribute filter
	UT_String p( "P" );
	UT_String filter( operands->member<StringData>( "attributeFilter" )->readable() );
	UT_StringMMPattern attribFilter;
	// force P and optionally prevent name
	if ( !operands->member<BoolData>("preserveName")->readable() )
	{
		filter += " ^name";
	}

	if ( !p.match( filter ) )
	{
		filter += " P";
	}
	attribFilter.compile( filter );

	// add detail attribs
	if ( result->variableSize( detailInterpolation ) == 1 )
	{
		transferDetailAttribs( geo, operands, attribFilter, result, detailInterpolation );
	}

	// add point attribs
	if ( result->variableSize( pointInterpolation ) == (unsigned)geo->getNumPoints() )
	{
		transferElementAttribs( geo, geo->getPointRange(), operands, geo->pointAttribs(), attribFilter, result, pointInterpolation );
	}

	// add primitive attribs
	size_t numPrims = geo->getNumPrimitives();
	if ( result->variableSize( primitiveInterpolation ) == numPrims )
	{
		transferElementAttribs( geo, geo->getPrimitiveRange(), operands, geo->primitiveAttribs(), attribFilter, result, primitiveInterpolation );
	}

	// add vertex attribs
	size_t numVerts = geo->getNumVertices();
	if ( geo->vertexAttribs().entries() && result->variableSize( vertexInterpolation ) == numVerts )
	{
		const GA_PrimitiveList &primitives = geo->getPrimitiveList();

		GA_OffsetList offsets;
		offsets.harden( numVerts );
		offsets.setEntries( numVerts );

		size_t i = 0;
		GA_Offset start, end;
		for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
		{
			for( GA_Offset offset = start; offset < end; ++offset )
			{
				const GA_Primitive *prim = primitives.get( offset );
				/// \todo: we shouldn't reverse winding for open polys (eg linear curves)
				bool reverseWinding = ( prim->getTypeId() == GEO_PRIMPOLY );

				size_t numPrimVerts = prim->getVertexCount();
				for ( size_t v=0; v < numPrimVerts; ++v, ++i )
				{
					if( reverseWinding )
					{
						offsets.set( i, prim->getVertexOffset( numPrimVerts - 1 - v ) );
					}
					else
					{
						offsets.set( i, prim->getVertexOffset( v ) );
					}
				}
			}
		}

		// \todo: Apparently the loop above could be more efficient using UT_Array<GA_Offset> if we didn't need the GA_Range.
		// Consider changing the transferElementAttribs API to allow for that.
		GA_Range vertRange( geo->getVertexMap(), offsets );
		transferElementAttribs( geo, vertRange, operands, geo->vertexAttribs(), attribFilter, result, vertexInterpolation );
	}

	transferTags( geo, result );
}

void FromHoudiniGeometryConverter::transferElementAttribs( const GU_Detail *geo, const GA_Range &range, const IECore::CompoundObject *operands, const GA_AttributeDict &attribs, const UT_StringMMPattern &attribFilter, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
{
	std::vector<const GA_Attribute *> attrs;
	for( GA_AttributeDict::iterator it=attribs.begin( GA_SCOPE_PUBLIC ); it != attribs.end(); ++it )
	{
		const GA_Attribute *attr = it.attrib();
		if( !attr )
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

		attrs.push_back( attr );
	}

	std::vector<PrimitiveVariable> primVars;
	primVars.resize( attrs.size() );

	std::vector<std::string> names;
	names.resize( attrs.size() );

	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, attrs.size() ),
		[this, &geo, &range, &operands, &attrs, &primVars, &names]( const tbb::blocked_range<size_t> &r )
		{
			for( auto i = r.begin(); i != r.end(); ++i )
			{
				transferElementAttrib( geo, range, operands, attrs[i], primVars[i], names[i] );
			}
		}
	);

	for( size_t i = 0; i < attrs.size(); ++i )
	{
		if( primVars[i].data )
		{
			primVars[i].interpolation = interpolation;
			result->variables[names[i]] = primVars[i];
		}
	}
}

void FromHoudiniGeometryConverter::transferElementAttrib(
	const GU_Detail *geo, const GA_Range &range, const IECore::CompoundObject *operands,
	const GA_Attribute *attr, PrimitiveVariable &result, std::string &resultName
) const
{
	// special case for uvs
	if( attr->getOptions().typeInfo() == GA_TYPE_TEXTURE_COORD || attr->getName().equal( "uv" ) )
	{
		// uvs are V3f in Houdini, so we must extract individual components
		FloatVectorDataPtr uData = extractData<FloatVectorData>( attr, range, 0 );
		FloatVectorDataPtr vData = extractData<FloatVectorData>( attr, range, 1 );
		const std::vector<float> &u = uData->readable();
		const std::vector<float> &v = vData->readable();

		IntVectorDataPtr indexData = nullptr;
		V2fVectorDataPtr uvData = new V2fVectorData;
		uvData->setInterpretation( GeometricData::UV );

		std::vector<Imath::V2f> &uvs = uvData->writable();
		uvs.reserve( u.size() );

		if( operands->member<BoolData>("weldUVs")->readable() )
		{
			indexData = new IntVectorData;
			std::vector<int> &indices = indexData->writable();

			std::unordered_map<Imath::V2f, size_t, Hasher> uniqueUVs;

			for( size_t i = 0; i < u.size(); ++i )
			{
				Imath::V2f uv( u[i], v[i] );

				int newIndex = uniqueUVs.size();
				auto uvIt = uniqueUVs.insert( { uv, newIndex } );
				if( uvIt.second )
				{
					indices.push_back( newIndex );
					uvs.push_back( uv );
				}
				else
				{
					indices.push_back( uvIt.first->second );
				}
			}
		}
		else
		{
			for( size_t i = 0; i < u.size(); ++i )
			{
				uvs.emplace_back( u[i], v[i] );
			}
		}

		result.data = uvData;
		result.indices = indexData;
		resultName = attr->getName().toStdString();

		return;
	}

	// check for remapping information for this attribute
	const GA_ROAttributeRef attrRef( attr );
	transferAttribData( result, resultName, attrRef, range );
}

void FromHoudiniGeometryConverter::transferAttribData(
	IECoreScene::PrimitiveVariable &result, std::string &resultName,
	const GA_ROAttributeRef &attrRef, const GA_Range &range
) const
{
	DataPtr dataPtr = nullptr;

	// we use this initial value to indicate we don't have a remapping so just
	// guess what destination type to use.
	IECore::TypeId varType = IECore::InvalidTypeId;

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
							dataPtr = extractData<FloatVectorData>( attr, range );
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
							dataPtr = extractData<FloatVectorData>( attr, range );
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
						dataPtr = extractData<QuatfVectorData>( attr, range );
					}
					break;
				}
				case 9 :
				{
					dataPtr = extractData<M33fVectorData>( attr, range );
					break;
				}
				case 16 :
				{
					dataPtr = extractData<M44fVectorData>( attr, range );
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
		if ( m_convertStandardAttributesParameter->getTypedValue() )
		{
			varName = processPrimitiveVariableName( varName );
		}

		// add the primitive variable to our result
		result.data = dataPtr;
		result.indices = indices;
		resultName = varName;
	}
}

void FromHoudiniGeometryConverter::transferDetailAttribs( const GU_Detail *geo, const IECore::CompoundObject *operands, const UT_StringMMPattern &attribFilter, Primitive *result, PrimitiveVariable::Interpolation interpolation ) const
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

void FromHoudiniGeometryConverter::transferTags( const GU_Detail *geo, Primitive *result ) const
{
	// map ieTag_ primGroups to names as blind data on the mesh
	GA_ROHandleS nameAttrib( geo, GA_ATTRIB_PRIMITIVE, GA_Names::name );
	if( !nameAttrib.isValid() )
	{
		return;
	}

	// assemble the unique tags and map them to PrimGroups
	std::vector<GA_PrimitiveGroup *> groups;
	InternedStringVectorDataPtr uniqueTagsData = new InternedStringVectorData;
	auto &uniqueTags = uniqueTagsData->writable();
	for( auto it = geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
	{
		if( it.group()->getInternal() || it.group()->isEmpty() )
		{
			continue;
		}

		const UT_String groupName( it.group()->getName() );
		if( !groupName.startsWith( g_tagGroupPrefix ) )
		{
			continue;
		}

		UT_String tag;
		groupName.substr( tag, g_tagGroupPrefix.length() );
		tag.substitute( "_", ":" );

		groups.emplace_back( it.group() );
		uniqueTags.emplace_back( tag.toStdString() );
	}

	// no valid tags
	if( uniqueTags.empty() )
	{
		return;
	}

	CompoundDataPtr tagMapData = new CompoundData;
	auto &tagMap = tagMapData->writable();
	tagMap[g_uniqueTags] = uniqueTagsData;

	// assemble the unique locations and pre-allocate membership vectors per tag
	std::vector<std::vector<bool> *> locationMembership;
	const GA_Attribute *nameAttr = nameAttrib.getAttribute();
	/// \todo: replace with GA_ROHandleS somehow... its not clear how, there don't seem to be iterators.
	const GA_AIFSharedStringTuple *nameTuple = nameAttr->getAIFSharedStringTuple();
	for( GA_AIFSharedStringTuple::iterator it = nameTuple->begin( nameAttr ); !it.atEnd(); ++it )
	{
		BoolVectorDataPtr membershipData = new BoolVectorData;
		tagMap.insert( { it.getString(), membershipData } );

		auto &membership = membershipData->writable();
		membership.resize( uniqueTags.size(), false );

		locationMembership.emplace_back( &membership );
	}

	// calculate the tag membership per location
	for( size_t i = 0; i < groups.size(); ++i )
	{
		GA_Offset start, end;
		for( GA_Iterator it( geo->getPrimitiveRange( groups[i] ) ); it.blockAdvance( start, end ); )
		{
			for( GA_Offset offset = start; offset < end; ++offset )
			{
				int id = nameAttrib.getIndex( offset );
				if( id < 0 )
				{
					continue;
				}

				(*locationMembership[id])[i] = true;
			}
		}
	}

	result->blindData()->writable()[g_Tags] = tagMapData;
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

	size_t i = 0;
	bool adjustedDefault = false;

	GA_Offset start, end;
	GA_ROHandleS attribHandle( attr );
	for( GA_Iterator it( range ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset, ++i )
		{
			const int index = attribHandle.getIndex( offset );
			if( index < 0 )
			{
				if( !adjustedDefault )
				{
					strings.push_back( "" );
					adjustedDefault = true;
				}

				indices[i] = numStrings;
			}
			else
			{
				indices[i] = index;
			}
		}
	}

	// The data table for string attributes may have
	// values which are not indexed. We rebuild the primvar / indices to only included data which is
	// actually indexed.
	IECoreScene::PrimitiveVariableAlgos::IndexedPrimitiveVariableBuilder<std::string, IECore::TypedData> builder( strings.size(), indexContainer.size() );
	PrimitiveVariable::IndexedView<std::string> indexedView( strings, &indexContainer );
	for( size_t i = 0, end = indexedView.size(); i < end; ++i )
	{
		builder.addIndexedValue( indexedView, i );
	}

	indexData = builder.indexedData().indices;
	indexContainer = indexData->writable();
	indices = indexData->baseWritable();
	strings = runTimeCast<IECore::StringVectorData> ( builder.indexedData().data )->readable();

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

	return new IECore::StringVectorData( dest );
}

DataPtr FromHoudiniGeometryConverter::extractStringData( const GU_Detail *geo, const GA_Attribute *attr ) const
{
	if( const GA_AIFStringTuple *stringTuple = attr->getAIFStringTuple() )
	{
		int tupleSize = stringTuple->getTupleSize( attr );
		UT_StringArray strings;

		if( stringTuple->getStrings( attr, 0, strings, tupleSize ) && strings.size() == 1)
		{
			StringDataPtr data = new StringData();
			data->writable() = strings[0].c_str();
			return data;
		}
	}
	else if( const GA_AIFSharedStringArray *sharedStringArray = attr->getAIFSharedStringArray() )
	{
		UT_StringArray strings;
		if( sharedStringArray->get( attr, 0, strings ) )
		{
			StringVectorDataPtr data = new StringVectorData();
			auto &writable = data->writable();
			writable.resize( strings.size() );
			for( int i = 0; i < strings.size(); ++i )
			{
				writable[i] = strings[i].c_str();
			}

			return data;
		}
	}

	return new StringData();
}


bool FromHoudiniGeometryConverter::hasOnlyOpenPolygons( const GU_Detail *geo )
{
	GA_Iterator primIt = geo->getPrimitiveRange().begin();
	if( primIt.atEnd() )
	{
		return false;
	}

	// if we have all open polygons then export as linear curves.
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );
			if( prim->getTypeId() != GEO_PRIMPOLY )
			{
				return false;
			}

			// as per SideFx, this is the most efficient way to determine if the prim is closed
			if( geo->getPrimitiveVertexList( offset ).getExtraFlag() )
			{
				return false;
			}
		}
	}

	return true;
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
	GA_ROHandleS nameAttrib( geo, GA_ATTRIB_PRIMITIVE, GA_Names::name );
	if ( nameAttrib.isValid() )
	{
		GA_OffsetList offsets;
		GA_Offset start, end;
		for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
		{
			for( GA_Offset offset = start; offset < end; ++offset )
			{
				const char *currentName = nameAttrib.get( offset, 0 );
				if ( UT_String( currentName ).multiMatch( nameFilter ) )
				{
					offsets.append( offset );
				}
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
					// it works, therfore its good enough, since we have no handle to judge Convertability
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
