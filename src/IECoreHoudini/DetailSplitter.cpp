//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "GU/GU_Detail.h"
#include "OP/OP_Context.h"

#include "boost/regex.hpp"
#include "boost/algorithm/string/replace.hpp"

#include "tbb/tbb.h"

#include "IECoreHoudini/DetailSplitter.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/CurvesAlgo.h"
#include "IECoreScene/PointsAlgo.h"

#include "IECore/PathMatcher.h"
#include "IECore/DataAlgo.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundParameter.h"

#include "boost/algorithm/string/join.hpp"
#include "IECoreHoudini/FromHoudiniPolygonsConverter.h"
#include "IECoreHoudini/FromHoudiniCurvesConverter.h"
#include "IECoreHoudini/FromHoudiniPointsConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

namespace
{

IECore::InternedString g_Tags( "tags" );
static std::string attrName = "name";

/// ensure we have a normalised path with leading '/'
/// examples: '///a/b/c//d' -> '/a/b/c/d'
/// 'e/f/g' -> '/e/f/g'
/// unlike a regular normalise it doesn't handle .. or .
std::string normalisePath(const std::string& str)
{
	std::string cleanedPath;
	SceneInterface::Path p;
	SceneInterface::stringToPath(str, p);
	SceneInterface::pathToString(p, cleanedPath);
	return cleanedPath;
}

IntVectorDataPtr preprocessNames( PrimitiveVariable &primVar, int numIds )
{
	// we want to segment on indices rather than strings. the results are the
	// same but the comparison operations in the segment algorithm are quicker.
	primVar.data = primVar.indices;
	primVar.indices = nullptr;
	IntVectorDataPtr uniqueIds = new IntVectorData;
	auto &ids = uniqueIds->writable();
	ids.reserve( numIds );
	for( int i = 0; i < numIds; ++i )
	{
		ids.push_back( i );
	}

	return uniqueIds;
}

std::string postprocessNames( Primitive &primitive, const std::vector<std::string> &segmentNames )
{
	int id = runTimeCast<IntVectorData>( primitive.variables[attrName].data )->readable()[0];
	primitive.variables.erase( attrName );
	return segmentNames[id];
}

void processTags( Primitive &primitive, const std::string &name, CompoundData *blindData )
{
	if( auto tagMap = blindData->member<CompoundData>( g_Tags ) )
	{
		if( InternedStringVectorDataPtr tags = tagMap->member<InternedStringVectorData>( name ) )
		{
			primitive.blindData()->writable()[g_Tags] = tags;
		}
	}
}

/// For a given detail get all the unique names
DetailSplitter::Names getNames( const GU_Detail *detail )
{
	std::set<InternedString> uniqueNames;
	DetailSplitter::Names allNames;

	GA_ROAttributeRef nameAttrRef = detail->findStringTuple( GA_ATTRIB_PRIMITIVE, attrName.c_str() );
	if( !nameAttrRef.isValid() )
	{
		return allNames;
	}

	const GA_Attribute *nameAttr = nameAttrRef.getAttribute();
	const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();

	for( GA_AIFSharedStringTuple::iterator it = tuple->begin( nameAttr ); !it.atEnd(); ++it )
	{
		allNames.push_back( it.getString() );
	}

	GA_Range allPrimsRange = detail->getPrimitiveRange();
	for( GA_Iterator it = allPrimsRange.begin(); !it.atEnd(); ++it )
	{
		const int index = tuple->getHandle( nameAttr, it.getOffset() );

		if( index < 0 )
		{
			continue;
		}
		else
		{
			uniqueNames.insert( allNames[index] );
		}
	}

	return DetailSplitter::Names( uniqueNames.begin(), uniqueNames.end() );
}

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

void weldUVs( std::vector<MeshPrimitivePtr> &meshes )
{
	std::vector<PrimitiveVariable*> uvVariables;
	for( auto mesh : meshes )
	{
		for( auto &kv : mesh->variables )
		{
			if( const auto *input = runTimeCast<V2fVectorData>( kv.second.data.get() ) )
			{
				if( input->getInterpretation() == GeometricData::UV )
				{
					uvVariables.push_back( &kv.second );
				}
			}
		}
	}

	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, uvVariables.size() ),
		[&uvVariables]( const tbb::blocked_range<size_t> &r )
		{
			for( auto i = r.begin(); i != r.end(); ++i )
			{
				// \todo: this is largely duplicated from FromHoudiniGeometryConverter. should it be a MeshAlgo?
				auto primVar = uvVariables[i];
				const auto *input = runTimeCast<V2fVectorData>( primVar->data.get() );
				const auto &inUvs = input->readable();

				V2fVectorDataPtr output = new V2fVectorData;
				output->setInterpretation( GeometricData::UV );
				auto &outUVs = output->writable();
				// this is an overestimate but should be safe
				outUVs.reserve( inUvs.size() );

				IntVectorDataPtr indexData = new IntVectorData;
				auto &indices = indexData->writable();

				std::unordered_map<Imath::V2f, size_t, Hasher> uniqueUVs;

				for( const auto &inUV : inUvs )
				{
					int newIndex = uniqueUVs.size();
					auto uvIt = uniqueUVs.insert( { inUV, newIndex } );
					if( uvIt.second )
					{
						indices.push_back( newIndex );
						outUVs.push_back( inUV );
					}
					else
					{
						indices.push_back( uvIt.first->second );
					}
				}

				primVar->data = output;
				primVar->indices = indexData;
			}
		}
	);
}

} // namespace

DetailSplitter::DetailSplitter( const GU_DetailHandle &handle, const std::string &key, bool useHoudiniSegment )
	: m_time( 0.0 ), m_objNode( nullptr ), m_lastMetaCount( -1 ), m_key( key ), m_handle( handle ), m_useHoudiniSegment( useHoudiniSegment )
{
}

DetailSplitter::DetailSplitter( OBJ_Node *objNode, double time, const std::string &key , bool useHoudiniSegment  )
	: m_time( time ),
	m_objNode( objNode ),
	m_lastMetaCount( -1 ),
	m_key( key ),
	m_useHoudiniSegment( useHoudiniSegment ),
	m_context( time ),
	m_handle( objNode->getRenderGeometryHandle(m_context, false ) )
{
}

DetailSplitter::~DetailSplitter()
{
}

const GU_DetailHandle &DetailSplitter::handle() const
{
	return m_handle;
}

const GU_DetailHandle DetailSplitter::split( const std::string &value )
{
	if ( !validate() )
	{
		return GU_DetailHandle();
	}

	Cache::const_iterator it = m_cache.find( value );
	if ( it != m_cache.end() )
	{
		return it->second;
	}

	return GU_DetailHandle();
}

IECore::ObjectPtr DetailSplitter::splitObject( const std::string& value)
{
	if ( !validate() )
	{
		return nullptr;
	}

	auto it = m_segmentMap.find( value );
	if ( it != m_segmentMap.end() )
	{
		return it->second;
	}

	return nullptr;
}

bool DetailSplitter::validate()
{
	GU_DetailHandleAutoReadLock readHandle( m_handle );
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return false;
	}

	if ( geo->getMetaCacheCount() == m_lastMetaCount )
	{
		return true;
	}

	m_names = ::getNames( geo );

	if ( !m_pathMatcher )
	{
		m_pathMatcher = new IECore::PathMatcherData();
	}

	IECore::PathMatcher &pathMatcher = m_pathMatcher->writable();
	pathMatcher.clear();

	for( const auto &name : m_names )
	{
		pathMatcher.addPath( name );
	}

	m_cache.clear();
	m_lastMetaCount = geo->getMetaCacheCount();

	GA_ROAttributeRef attrRef = geo->findStringTuple( GA_ATTRIB_PRIMITIVE, m_key.c_str() );
	if ( !attrRef.isValid() )
	{
		m_cache[""] = m_handle;
		return true;
	}

	m_segmentMap.clear();

	if( !m_useHoudiniSegment )
	{
		auto converter = FromHoudiniGeometryConverter::create( m_handle );

		if ( converter )
		{
			converter->parameters()->parameter<BoolParameter>( "preserveName" )->setTypedValue( true );
			// disable UV welding during conversion to improve performance of the named segmentation.
			converter->parameters()->parameter<BoolParameter>( "weldUVs" )->setTypedValue( false );

			if( runTimeCast<FromHoudiniPolygonsConverter>( converter ) )
			{
				ObjectPtr o = converter->convert();
				MeshPrimitive *mesh = runTimeCast<MeshPrimitive>( o.get() );

				auto it = mesh->variables.find( attrName );
				if( it != mesh->variables.end() )
				{
					if( const StringVectorDataPtr nameData = runTimeCast<StringVectorData>( it->second.data ) )
					{
						const std::vector<std::string> &segmentNames = nameData->readable();
						IntVectorDataPtr uniqueIds = ::preprocessNames( it->second, segmentNames.size() );
						std::vector<MeshPrimitivePtr> segments = MeshAlgo::segment( mesh, it->second, uniqueIds.get() );
						// weld the mesh UVs to prevent discontinuity when subdivided
						::weldUVs( segments );
						for( size_t i = 0; i < segments.size(); ++i )
						{
							std::string name = ::postprocessNames( *segments[i], segmentNames );
							::processTags( *segments[i], name, mesh->blindData() );
							m_segmentMap[normalisePath( name )] = segments[i];
						}
						return true;
					}
				}
			}
			else if( runTimeCast<FromHoudiniCurvesConverter>( converter ) )
			{
				ObjectPtr o = converter->convert();
				CurvesPrimitive *curves = runTimeCast<CurvesPrimitive>( o.get() );

				auto it = curves->variables.find( attrName );
				if( it != curves->variables.end() )
				{
					if( const StringVectorDataPtr nameData = runTimeCast<StringVectorData>( it->second.data ) )
					{
						const std::vector<std::string> &segmentNames = nameData->readable();
						IntVectorDataPtr uniqueIds = ::preprocessNames( it->second, segmentNames.size() );
						std::vector<CurvesPrimitivePtr> segments = CurvesAlgo::segment( curves, it->second, uniqueIds.get() );
						for( size_t i = 0; i < segments.size(); ++i )
						{
							std::string name = ::postprocessNames( *segments[i], segmentNames );
							::processTags( *segments[i], name, curves->blindData() );
							m_segmentMap[normalisePath( name )] = segments[i];
						}
						return true;
					}
				}
			}
			else if( runTimeCast<FromHoudiniPointsConverter>( converter ) )
			{
				ObjectPtr o = converter->convert();
				PointsPrimitive *points = runTimeCast<PointsPrimitive>( o.get() );

				auto it = points->variables.find( attrName );
				if( it != points->variables.end() )
				{
					if( const StringVectorDataPtr nameData = runTimeCast<StringVectorData>( it->second.data ) )
					{
						const std::vector<std::string> &segmentNames = nameData->readable();
						IntVectorDataPtr uniqueIds = ::preprocessNames( it->second, segmentNames.size() );
						std::vector<PointsPrimitivePtr> segments = PointsAlgo::segment( points, it->second, uniqueIds.get() );
						for( size_t i = 0; i < segments.size(); ++i )
						{
							std::string name = ::postprocessNames( *segments[i], segmentNames );
							::processTags( *segments[i], name, points->blindData() );
							m_segmentMap[normalisePath( name )] = segments[i];
						}
						return true;
					}
				}
			}
		}
	}

	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();

	std::map<GA_StringIndexType, GA_OffsetList> offsets;
	GA_Range primRange = geo->getPrimitiveRange();
	for ( GA_Iterator it = primRange.begin(); !it.atEnd(); ++it )
	{
		GA_StringIndexType currentHandle = tuple->getHandle( attr, it.getOffset() );

		std::map<GA_StringIndexType, GA_OffsetList>::iterator oIt = offsets.find( currentHandle );
		if ( oIt == offsets.end() )
		{
			oIt = offsets.insert( std::pair<GA_StringIndexType, GA_OffsetList>( currentHandle, GA_OffsetList() ) ).first;
		}

		oIt->second.append( it.getOffset() );
	}

	for ( std::map<GA_StringIndexType, GA_OffsetList>::iterator oIt = offsets.begin(); oIt != offsets.end(); ++oIt )
	{
		GU_Detail *newGeo = new GU_Detail();
		GA_Range matchPrims( geo->getPrimitiveMap(), oIt->second );
		newGeo->mergePrimitives( *geo, matchPrims );
		newGeo->incrementMetaCacheCount();

		GU_DetailHandle handle;
		handle.allocateAndSet( newGeo, true );

		std::string current = "";
		if ( const char *value = tuple->getTableString( attr, oIt->first ) )
		{
			current = value;
		}

		m_cache[current] = handle;
	}

	return !m_cache.empty();
}

void DetailSplitter::values( std::vector<std::string> &result )
{
	/// \todo: do we really want this method to create the cache? should it just look at the names instead?
	if ( !validate() )
	{
		return;
	}

	result.clear();
	result.reserve( m_cache.size() );

	for ( Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++it )
	{
		result.push_back( it->first );
	}
}

DetailSplitter::Names DetailSplitter::getNames(const std::vector<IECore::InternedString>& path)
{
	Names names;

	if ( !validate())
	{
		return names;
	}

	IECore::PathMatcher subTree = m_pathMatcher->readable().subTree( path );
	for ( IECore::PathMatcher::RawIterator it = subTree.begin(); it != subTree.end(); ++it )
	{
		if ( !it->empty() )
		{
			names.push_back( *it->rbegin() );
			it.prune();
		}
	}

	return names;
}

bool DetailSplitter::hasPath( const IECoreScene::SceneInterface::Path& path, bool isExplicit )
{
	if ( !validate() )
	{
		return false;
	}

	if ( isExplicit )
	{
		PathMatcher::RawIterator rawIt = m_pathMatcher->readable().find( path );

		return rawIt != m_pathMatcher->readable().end() && rawIt.exactMatch();
	}

	return 	m_pathMatcher->readable().find( path ) != m_pathMatcher->readable().end();
}

bool DetailSplitter::hasPaths()
{
	if ( !validate() )
	{
		return false;
	}
	return m_pathMatcher && !m_pathMatcher->readable().isEmpty();
}

bool DetailSplitter::update( OBJ_Node *objNode, double time )
{
	if ( m_objNode == objNode && time == m_time)
	{
		return false;
	}

	m_time = time;
	m_objNode = objNode;
	m_lastMetaCount = -1;
	m_context = OP_Context( time );
	m_handle = m_objNode->getRenderGeometryHandle(m_context, false ) ;

	m_pathMatcher.reset();
	m_names.clear();
	m_segmentMap.clear();
	m_cache.clear();

	return true;
}
