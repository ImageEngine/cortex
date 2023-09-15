//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design. All rights reserved.
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

#include "USDScene.h"

#include "IECoreUSD/AttributeAlgo.h"
#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"
#include "IECoreUSD/ShaderAlgo.h"

#include "IECoreScene/ShaderNetwork.h"

#include "IECore/LRUCache.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/usd/usd/collectionAPI.h"
#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdGeom/gprim.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdGeom/pointInstancer.h"
#include "pxr/usd/usdGeom/primvar.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#include "pxr/usd/usdGeom/scope.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
#if PXR_VERSION >= 2111
#include "pxr/usd/usdLux/lightAPI.h"
#endif
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/materialBindingAPI.h"
#include "pxr/usd/usdShade/connectableAPI.h"
#ifdef IECOREUSD_WITH_OPENVDB
#include "pxr/usd/usdVol/fieldBase.h"
#endif
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/format.hpp"
#include "boost/functional/hash.hpp"

#include "tbb/concurrent_hash_map.h"

#include <iostream>
#include <mutex>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

#if PXR_VERSION < 2011
#define GetPrimInPrototype GetPrimInMaster
#endif

namespace
{

void append( const pxr::SdfPath &path, IECore::MurmurHash &h )
{
	h.append( (uint64_t)pxr::SdfPath::Hash()( path ) );
}

void appendPrimOrMasterPath( const pxr::UsdPrim &prim, IECore::MurmurHash &h )
{
	if( prim.IsInstanceProxy() )
	{
		append( prim.GetPrimInPrototype().GetPrimPath(), h );
	}
	else
	{
		append( prim.GetPrimPath(), h );
	}
}

SceneInterface::Path fromUSDWithoutPrefix( const pxr::SdfPath &path, size_t prefixSize )
{
	size_t i = path.GetPathElementCount() - prefixSize;
	SceneInterface::Path result( i );
	pxr::SdfPath p = path;
	while( i )
	{
		result[--i] = p.GetElementString();
		p = p.GetParentPath();
	}
	return result;
}

pxr::TfToken validName( const std::string &name )
{
	// `TfMakeValidIdentifier` _almost_ does what we want, but in Gaffer
	// we use purely numeric identifiers for instance names, and
	// `TfMakeValidIdentifier` replaces leading non-alphanumeric characters
	// with '_', meaning that `0-9` all become `_`. We want to _prefix_ with
	// an `_` instead to preserve uniqueness.

	if( name.size() && '0' <= name[0] && name[0] <= '9' )
	{
		return pxr::TfToken( pxr::TfMakeValidIdentifier( "_" + name ) );
	}
	else
	{
		return pxr::TfToken( pxr::TfMakeValidIdentifier( name ) );
	}
}

template<typename T>
T *reportedCast( const IECore::RunTimeTyped *v, const char *context, const char *name )
{
	if( T *t = IECore::runTimeCast<T>( v ) )
	{
		return t;
	}

	IECore::msg( IECore::Msg::Warning, context, boost::format( "Expected %s but got %s for \"%s\"." ) % T::staticTypeName() % v->typeName() % name );
	return nullptr;
}

static pxr::TfToken g_tagsPrimName( "cortexTags" );
static pxr::TfToken g_metadataAutoMaterials( "cortex_autoMaterials" );

bool isSceneChild( const pxr::UsdPrim &prim )
{
	if( !prim || !prim.IsDefined() || prim.GetName() == g_tagsPrimName )
	{
		return false;
	}

#ifdef IECOREUSD_WITH_OPENVDB
	if( prim.IsA<pxr::UsdVolFieldBase>() )
	{
		// This will be absorbed into the VBDObject loaded by VDBAlgo.
		return false;
	}
#endif

	bool autoMaterials = false;
	prim.GetMetadata( g_metadataAutoMaterials, &autoMaterials );

	return (!autoMaterials) && (
		prim.GetTypeName().IsEmpty() ||
		pxr::UsdGeomImageable( prim )
	);
}

void writeSetInternal( const pxr::UsdPrim &prim, const pxr::TfToken &name, const IECore::PathMatcher &set )
{
	if( prim.IsPseudoRoot() )
	{
		// Can't write sets at the root. Split them across the children.
		for( PathMatcher::RawIterator it = set.begin(), eIt = set.end(); it != eIt; ++it )
		{
			if( it->empty() )
			{
				// Skip root
				continue;
			}
			pxr::UsdPrim childPrim = prim.GetStage()->DefinePrim( USDScene::toUSD( *it ) );
			writeSetInternal( childPrim, validName( name ), set.subTree( *it ) );
			it.prune(); // Only visit children of root
		}
		return;
	}

	pxr::SdfPathVector targets;
	for( PathMatcher::Iterator it = set.begin(); it != set.end(); ++it )
	{
		targets.push_back( USDScene::toUSD( *it, /* relative = */ true ) );
	}

#if PXR_VERSION < 2009

	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( prim, validName( name ), pxr::UsdTokens->explicitOnly );

#else

	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::Apply( prim, validName( name ) );
	collection.CreateExpansionRuleAttr( pxr::VtValue( pxr::UsdTokens->explicitOnly ) );

#endif

	collection.CreateIncludesRel().SetTargets( targets );
}

using PrimPredicate = bool (pxr::UsdPrim::*)() const;
boost::container::flat_map<pxr::TfToken, PrimPredicate> g_schemaTypeSetPredicates = {
	{ pxr::TfToken( "__cameras" ), &pxr::UsdPrim::IsA<pxr::UsdGeomCamera> },
#if PXR_VERSION >= 2111
	{  pxr::TfToken( "__lights" ), &pxr::UsdPrim::HasAPI<pxr::UsdLuxLightAPI> },
#endif
	{ pxr::TfToken( "usd:pointInstancers" ), &pxr::UsdPrim::IsA<pxr::UsdGeomPointInstancer> }
};

// If `predicate` is non-null then it is called to determine if _this_ prim is in the set. If null,
// then the set is loaded from a UsdCollection called `name`.
IECore::PathMatcher localSet( const pxr::UsdPrim &prim, const pxr::TfToken &name, PrimPredicate predicate, const Canceller *canceller )
{
	PathMatcher result;

	if( predicate )
	{
		if( (prim.*predicate)() )
		{
			result.addPath( std::vector<IECore::InternedString>() );
		}
		return result;
	}

	const size_t prefixSize = prim.GetPath().GetPathElementCount();
	if( auto collection = pxr::UsdCollectionAPI( prim, name ) )
	{
		Canceller::check( canceller );
		pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();

		Canceller::check( canceller );
		pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths( membershipQuery, prim.GetStage() );

		for( const auto &path : includedPaths )
		{
			Canceller::check( canceller );
			if( path.HasPrefix( prim.GetPath() ) )
			{
				result.addPath( fromUSDWithoutPrefix( path, prefixSize ) );
			}
			else
			{
				IECore::msg(
					IECore::Msg::Level::Warning, "USDScene",
					boost::format( "Ignoring path \"%1%\" in collection \"%2%\" because it is not beneath the collection root \"%3%\"" ) %
						path % collection.GetName() % prim.GetPath()
				);
			}
		}
	}

	return result;
}

using SetMap = std::map<pxr::SdfPath, IECore::PathMatcher>;
IECore::PathMatcher recursiveSet( const pxr::UsdPrim &prim, const pxr::TfToken &name, PrimPredicate predicate, SetMap &prototypeSets, const Canceller *canceller )
{
	// Read set from this prim

	PathMatcher result = localSet( prim, name, predicate, canceller );

	// Recurse to descendant prims

	Canceller::check( canceller );

	if( prim.IsInstance() )
	{
		// We only need to descend into a prototype the first time we encounter it. After that
		// we can just instance the PathMatcher from it into our result.
		auto [it, inserted] = prototypeSets.insert( { prim.GetPrototype().GetPath(), IECore::PathMatcher() } );
		if( inserted )
		{
			it->second = recursiveSet( prim.GetPrototype(), name, predicate, prototypeSets, canceller );
		}
		result.addPaths( it->second );
	}
	else
	{
		for( const auto &childPrim : prim.GetChildren() )
		{
			if( !isSceneChild( childPrim ) )
			{
				continue;
			}

			IECore::PathMatcher childSet = recursiveSet( childPrim, name, predicate, prototypeSets, canceller );
			if( !childSet.isEmpty() )
			{
				result.addPaths( childSet, { childPrim.GetPath().GetName() } );
			}
		}
	}

	return result;
}

IECore::PathMatcher readSetInternal( const pxr::UsdPrim &prim, const pxr::TfToken &name, bool includeDescendantSets, const Canceller *canceller )
{
	PrimPredicate predicate = nullptr;
	auto it = g_schemaTypeSetPredicates.find( name );
	if( it != g_schemaTypeSetPredicates.end() )
	{
		if( !prim.IsPseudoRoot() )
		{
			return PathMatcher();
		}
		else
		{
			predicate = it->second;
			includeDescendantSets = true;
		}
	}

	if( includeDescendantSets )
	{
		SetMap prototypeSets;
		return recursiveSet( prim, name, predicate, prototypeSets, canceller );
	}
	else
	{
		return localSet( prim, name, nullptr, canceller );
	}
}

SceneInterface::NameList localSetNames( const pxr::UsdPrim &prim )
{
	SceneInterface::NameList result;
	if( !prim.IsPseudoRoot() )
	{
		std::vector<pxr::UsdCollectionAPI> allCollections = pxr::UsdCollectionAPI::GetAllCollections( prim );
		result.reserve( allCollections.size() );
		for( const pxr::UsdCollectionAPI &collection : allCollections )
		{
			result.push_back( collection.GetName().GetString() );
		}
	}
	else
	{
		// Root. USD doesn't allow collections to be written here, but we automatically
		// generate sets to represent the locations of a few key schema types.
		for( const auto &s : g_schemaTypeSetPredicates )
		{
			result.push_back( s.first.GetString() );
		}
	}

	return result;
}

SceneInterface::NameList recursiveSetNames( const pxr::UsdPrim &prim, pxr::SdfPathSet &visitedPrototypes )
{
	// Get local names.

	SceneInterface::NameList result = localSetNames( prim );

	// Add names from descendants.

	if( prim.IsInstance() )
	{
		// We only need to descend into a prototype the first time we encounter it.
		if( visitedPrototypes.insert( prim.GetPrototype().GetPath() ).second )
		{
			SceneInterface::NameList prototypeSetNames = recursiveSetNames( prim.GetPrototype(), visitedPrototypes );
			result.insert( result.end(), prototypeSetNames.begin(), prototypeSetNames.end() );
		}
	}
	else
	{
		for( const auto &childPrim : prim.GetChildren( ) )
		{
			if( !isSceneChild( childPrim ) )
			{
				continue;
			}
			SceneInterface::NameList childSetNames = recursiveSetNames( childPrim, visitedPrototypes );
			result.insert( result.end(), childSetNames.begin(), childSetNames.end() );
		}
	}

	// Remove duplicates
	std::sort( result.begin(), result.end() );
	result.erase( std::unique( result.begin(), result.end() ), result.end() );

	return result;
}

void populateMaterial( pxr::UsdShadeMaterial &mat, const boost::container::flat_map<pxr::TfToken, IECoreScene::ConstShaderNetworkPtr> &shaders )
{
	for( const auto &[output, shaderNetwork] : shaders )
	{
		pxr::UsdShadeOutput matOutput = mat.CreateOutput( output, pxr::SdfValueTypeNames->Token );

		std::string shaderContainerName = boost::replace_all_copy( output.GetString(), ":", "_" ) + "_shaders";
		pxr::UsdGeomScope shaderContainer = pxr::UsdGeomScope::Define( mat.GetPrim().GetStage(), mat.GetPath().AppendChild( pxr::TfToken( shaderContainerName ) ) );
		pxr::UsdShadeOutput networkOut = ShaderAlgo::writeShaderNetwork( shaderNetwork.get(), shaderContainer.GetPrim() );

		if( networkOut.GetPrim().IsValid() )
		{
			matOutput.ConnectToSource( networkOut );
		}
	}
}

std::tuple<pxr::TfToken, pxr::TfToken> materialOutputAndPurpose( const std::string &attributeName )
{
	for( const auto &purpose : { pxr::UsdShadeTokens->preview, pxr::UsdShadeTokens->full } )
	{
		if(
			boost::ends_with( attributeName, purpose.GetString() ) &&
			attributeName.size() > purpose.GetString().size()
		)
		{
			size_t colonIndex = attributeName.size() - purpose.GetString().size() - 1;
			if( attributeName[colonIndex] == ':' )
			{
				return std::make_tuple(
					AttributeAlgo::nameToUSD( attributeName.substr( 0, colonIndex ) ).name,
					pxr::TfToken( attributeName.substr( colonIndex + 1 ) )
				);
			}
		}
	}
	return { AttributeAlgo::nameToUSD( attributeName ).name, pxr::UsdShadeTokens->allPurpose };
}

/// SdfPath is the appropriate cache key for _storage_, but we need a
/// `UsdShadeOutput` for computation. This struct provides the implicit
/// conversion that LRUCache needs to make that possible.
struct ShaderNetworkCacheGetterKey : public pxr::UsdShadeOutput
{
	ShaderNetworkCacheGetterKey( const pxr::UsdShadeOutput &output )
		:	pxr::UsdShadeOutput( output )
	{
	}

	operator pxr::SdfPath () const
	{
		return GetAttr().GetPath();
	}
};

class ShaderNetworkCache : public LRUCache<pxr::SdfPath, IECoreScene::ConstShaderNetworkPtr, LRUCachePolicy::Parallel, ShaderNetworkCacheGetterKey>
{

	public :

		ShaderNetworkCache( size_t maxBytes )
			:	LRUCache<pxr::SdfPath, IECoreScene::ConstShaderNetworkPtr, LRUCachePolicy::Parallel, ShaderNetworkCacheGetterKey>( getter, maxBytes )
		{
		}

	private :

		static IECoreScene::ConstShaderNetworkPtr getter( const ShaderNetworkCacheGetterKey &key, size_t &cost )
		{
			IECoreScene::ConstShaderNetworkPtr result = ShaderAlgo::readShaderNetwork( key );
			cost = result ? result ->Object::memoryUsage() : 0;
			return result;
		}

};

Imath::M44d localTransform( const pxr::UsdPrim &prim, pxr::UsdTimeCode time )
{
	pxr::UsdGeomXformable transformable( prim );
	if( !transformable )
	{
		return Imath::M44d();
	}

	pxr::GfMatrix4d transform;
	bool reset = false;
	transformable.GetLocalTransformation( &transform, &reset, time );
	Imath::M44d result = DataAlgo::fromUSD( transform );

	if( reset )
	{
		// Apply inverse of parent's world transform.
		Imath::M44d parentWorldTransform;
		pxr::UsdPrim parentPrim = prim.GetParent();
		while( parentPrim )
		{
			parentWorldTransform = parentWorldTransform * localTransform( parentPrim, time );
			parentPrim = parentPrim.GetParent();
		}
		result = result * parentWorldTransform.inverse();
	}

	if( ( prim.GetParent().IsPseudoRoot() || reset ) && pxr::UsdGeomGetStageUpAxis( prim.GetStage() ) == pxr::UsdGeomTokens->z )
	{
		// Apply Z-up to Y-up correction
		static Imath::M44d b(
			0, 0, 1, 0,
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 1
		);
		result = result * b;
	}

	return result;
}

// Used to assign a unique hash to each USD file. Using a global counter rather than the file name
// means that we treat the same file as separate if it is closed and reopened. This means it's not
// a problem if USD changes things when a file is reopened. USD appears to not in general guarantee
// that anything is the same when reopening an unchanged file - things we're aware of that could
// cause problems without this conservative uniquifying are: how instance prototype names are
// assigned, and hashes ( indices ) of SdfPaths
std::atomic< int > g_usdFileCounter = 0;

} // namespace

class USDScene::Location : public RefCounted
{
	public:
		Location(pxr::UsdPrim prim ) : prim(prim) {}
		pxr::UsdPrim prim;
};

class USDScene::IO : public RefCounted
{

	public :

		IO( const std::string &fileName, IndexedIO::OpenMode openMode )
			:	IO( fileName, makeStage( fileName, openMode ), openMode )
		{
		}

		IO( const std::string &fileName, const pxr::UsdStageRefPtr &stage, IndexedIO::OpenMode openMode )
			:	m_fileName( fileName ), m_openMode( openMode ), m_stage( stage ),
				m_rootPrim( m_stage->GetPseudoRoot() ),
				m_timeCodesPerSecond( m_stage->GetTimeCodesPerSecond() ),
				m_shaderNetworkCache( 10 * 1024 * 1024 ), // 10Mb
				m_uniqueId( g_usdFileCounter.fetch_add( 1, std::memory_order_relaxed ) )
		{
			// Although the USD API implies otherwise, we need a different
			// cache per-purpose because `UsdShadeMaterialBindingAPI::ComputeBoundMaterial()`
			// gives inconsistent results if the cache is shared. We pre-populate
			// `m_usdBindingsCaches` here because it wouldn't be thread-safe to
			// make insertions in `computeBoundMaterial()`.
			m_usdBindingsCaches[pxr::UsdShadeTokens->allPurpose];
			m_usdBindingsCaches[pxr::UsdShadeTokens->full];
			m_usdBindingsCaches[pxr::UsdShadeTokens->preview];
		}

		~IO() override
		{
			if( m_openMode == IndexedIO::Write )
			{
				for( auto &tagSet : tagSets )
				{
					writeSetInternal( root(), pxr::TfToken( tagSet.first.string() ), tagSet.second );
				}
				m_stage->GetRootLayer()->Save();
			}
		}

		const std::string &fileName() const
		{
			return m_fileName;
		}

		IndexedIO::OpenMode openMode() const
		{
			return m_openMode;
		}

		pxr::UsdPrim &root()
		{
			return m_rootPrim;
		}

		const pxr::UsdStageRefPtr &getStage() const
		{
			return m_stage;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		// Tags
		// ====
		//
		// We want to transition away from tags completely and move
		// to sets, because they have native representation in Gaffer and
		// map much better to USD and Alembic collections. To help this
		// transition, we implement the tags API so that it reads and writes
		// UsdCollections that can also be read via the sets API. We
		// buffer tags as IECore::PathMatcher objects as writing or reading
		// them one location at a time via the UsdCollection API is
		// prohibitively slow.

		using TagSetsMap = tbb::concurrent_hash_map<IECore::InternedString, IECore::PathMatcher>;
		TagSetsMap tagSets;

		const SceneInterface::NameList &allTags()
		{
			assert( m_openMode == IndexedIO::Read );
			std::call_once(
				m_allTagsFlag,
				[this]() {
					pxr::SdfPathSet visitedPrototypes;
					m_allTags = recursiveSetNames( m_rootPrim, visitedPrototypes );
				}
			);
			return m_allTags;
		}

		/// \todo This "flattens" material assignment, so that materials assigned at `/root` are loaded as attributes
		/// on `/root/child` as well. This is not really what we want - we want to load sparsely and let attribute
		/// inheritance do the rest. This would be complicated by two factors :
		///
		/// - USD's collection-based bindings. A collection-based binding on an ancestor prim would need to be transformed
		///   into a Cortex attribute on the prim, if the collection includes the prim.
		/// - USD's `bindingStrength` concept, where `UsdShadeTokens->strongerThanDescendants` allows an ancestor's
		///   binding to clobber descendant bindings during resolution. It is not clear how to represent that in Cortex -
		///   perhaps by not loading the descendant attributes at all?
		pxr::UsdShadeMaterial computeBoundMaterial( const pxr::UsdPrim &prim, const pxr::TfToken &materialPurpose )
		{
			// This should be thread safe, despite using caches, because
			// BindingsCache and CollectionQueryCache are implemented by USD as
			// tbb::concurrent_unordered_map
			pxr::UsdRelationship bindingRelationship;
			pxr::UsdShadeMaterial material = pxr::UsdShadeMaterialBindingAPI( prim ).ComputeBoundMaterial(
				&m_usdBindingsCaches.at( materialPurpose ), &m_usdCollectionQueryCache, materialPurpose, &bindingRelationship
			);
			if( material && materialPurpose != pxr::UsdShadeTokens->allPurpose && bindingRelationship.GetBaseName() != materialPurpose )
			{
				// Ignore USD fallback to the all purpose binding. We want to load only the bindings that actually exist,
				// and then allow people to manage them after loading.
				return pxr::UsdShadeMaterial();
			}

			if( material.GetPrim().IsInstanceProxy() )
			{
				// Use the prototype so that we only load once in `readShaderNetwork()`,
				// and so instanced locations have the same `attributesHash()`.
				return pxr::UsdShadeMaterial( material.GetPrim().GetPrimInPrototype() );
			}

			return material;
		}

		IECoreScene::ConstShaderNetworkPtr readShaderNetwork( const pxr::UsdShadeOutput &output )
		{
			return m_shaderNetworkCache.get( output );
		}

		inline int uniqueId()
		{
			return m_uniqueId;
		}

	private :

		static pxr::UsdStageRefPtr makeStage( const std::string &fileName, IndexedIO::OpenMode openMode )
		{
			switch( openMode )
			{
				case IndexedIO::Read : {
					pxr::UsdStageRefPtr stage = pxr::UsdStage::Open( fileName );
					if( !stage )
					{
						throw IECore::Exception( boost::str( boost::format( "USDScene : Failed to open USD file: '%1%'" ) % fileName ) );
					}
					return stage;
				}
				case IndexedIO::Write :
					return pxr::UsdStage::CreateNew( fileName );
				default:
					throw Exception( "Unsupported OpenMode" );
			}
		}

		std::string m_fileName;
		IndexedIO::OpenMode m_openMode;
		pxr::UsdStageRefPtr m_stage;
		pxr::UsdPrim m_rootPrim;
		double m_timeCodesPerSecond;

		std::once_flag m_allTagsFlag;
		SceneInterface::NameList m_allTags;

		boost::container::flat_map<pxr::TfToken, pxr::UsdShadeMaterialBindingAPI::BindingsCache> m_usdBindingsCaches;
		pxr::UsdShadeMaterialBindingAPI::CollectionQueryCache m_usdCollectionQueryCache;

		ShaderNetworkCache m_shaderNetworkCache;

		// Used to identify a file uniquely ( including between different openings of the same filename,
		// since closing and reopening a file may cause USD to shuffle the contents ).
		const int m_uniqueId;

};

USDScene::USDScene( const std::string &fileName, IndexedIO::OpenMode openMode )
	:	m_root( new IO( fileName, openMode ) ),
		m_location( new Location( m_root->root() ) )
{
}

USDScene::USDScene( const pxr::UsdStageRefPtr &stage, IndexedIO::OpenMode openMode )
	:	m_root( new IO( "", stage, openMode ) ),
		m_location( new Location( m_root->root() ) )
{
}

USDScene::USDScene( IOPtr io, LocationPtr location )
	:	m_root( io ), m_location( location )
{
}

USDScene::~USDScene()
{
	if( m_materials.size() )
	{
		try
		{
			// The root of the scene can't be referenced, so store our shaders 1 step above the root level
			pxr::SdfPath topAncestor = m_location->prim.GetPath();
			while( topAncestor.GetPathElementCount() > 1 )
			{
				topAncestor = topAncestor.GetParentPath();
			}

			pxr::UsdGeomScope materialContainer = pxr::UsdGeomScope::Get( m_root->getStage(), topAncestor.AppendChild( pxr::TfToken( "materials" ) ) );
			if( !materialContainer )
			{
				// Create a /topLevel/materials container since it doesn't already exist
				materialContainer = pxr::UsdGeomScope::Define( m_root->getStage(), topAncestor.AppendChild( pxr::TfToken( "materials" ) ) );

				// Label with metadata to say that this is not a real location in the scene graph
				materialContainer.GetPrim().SetMetadata( g_metadataAutoMaterials, true );
			}

			for( const auto &[purpose, material] : m_materials )
			{
				// Use a hash to identify the combination of shaders in this material.
				IECore::MurmurHash materialHash;
				for( const auto &[output, shaderNetwork] : material )
				{
					materialHash.append( output );
					materialHash.append( shaderNetwork->Object::hash() );
				}
				pxr::TfToken matName( "material_" + materialHash.toString() );

				// Write the material if it hasn't been written already.
				pxr::SdfPath matPath = materialContainer.GetPrim().GetPath().AppendChild( matName );
				pxr::UsdShadeMaterial mat = pxr::UsdShadeMaterial::Get( materialContainer.GetPrim().GetStage(), matPath );
				if( !mat )
				{
					mat = pxr::UsdShadeMaterial::Define( materialContainer.GetPrim().GetStage(), matPath );
					populateMaterial( mat, material );
				}

				// Bind the material to this location
				pxr::UsdShadeMaterialBindingAPI::Apply( m_location->prim ).Bind(
					mat, pxr::UsdShadeTokens->fallbackStrength, purpose
				);
			}
		}
		catch( std::exception &e )
		{
				IECore::msg(
					IECore::Msg::Error, "USDScene::~USDScene",
					boost::format( "Failed to write shaders with exception \"%1%\"" ) % e.what()
				);
		}
	}
}

std::string USDScene::fileName() const
{
	return m_root->fileName();
}

Imath::Box3d USDScene::readBound( double time ) const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	if( !boundable )
	{
		return Imath::Box3d();
	}

	pxr::UsdAttribute attr = boundable.GetExtentAttr();
	if( !attr.IsValid() )
	{
		return Imath::Box3d();
	}

	pxr::VtArray<pxr::GfVec3f> extents;
	attr.Get<pxr::VtArray<pxr::GfVec3f> >( &extents, m_root->getTime( time ) );

	if( extents.size() == 2 )
	{
		return Imath::Box3d(
			DataAlgo::fromUSD( extents[0] ),
			DataAlgo::fromUSD( extents[1] )
		);
	}

	return Imath::Box3d();
}

ConstDataPtr USDScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d USDScene::readTransformAsMatrix( double time ) const
{
	return localTransform( m_location->prim, m_root->getTime( time ) );
}

ConstObjectPtr USDScene::readObject( double time, const Canceller *canceller ) const
{
	return ObjectAlgo::readObject( m_location->prim, m_root->getTime( time ), canceller );
}

SceneInterface::Name USDScene::name() const
{
	return SceneInterface::Name( m_location->prim.GetName() );
}

void USDScene::path( SceneInterface::Path &p ) const
{
	p = fromUSD( m_location->prim.GetPath() );
}

bool USDScene::hasBound() const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	pxr::UsdAttribute attr;

	if( boundable )
	{
		attr = boundable.GetExtentAttr();
	}

	return attr.IsValid();
}

void USDScene::writeBound( const Imath::Box3d &bound, double time )
{
	// unable to write bounds on root scene graph location
	if( m_location->prim.GetPath().IsEmpty() )
	{
		return;
	}

	pxr::UsdGeomBoundable boundable( m_location->prim );
	if( !boundable )
	{
		return;
	}

	pxr::VtArray<pxr::GfVec3f> extent;
	extent.push_back( DataAlgo::toUSD( Imath::V3f( bound.min ) ) );
	extent.push_back( DataAlgo::toUSD( Imath::V3f( bound.max ) ) );

	pxr::UsdAttribute extentAttr = boundable.CreateExtentAttr();
	extentAttr.Set( pxr::VtValue( extent ) );
}

void USDScene::writeTransform( const Data *transform, double time )
{
	const M44dData *m44 = IECore::runTimeCast<const M44dData>( transform );
	if( !m44 )
	{
		return;
	}

	pxr::UsdGeomXformable xformable( m_location->prim );
	if( xformable )
	{
		pxr::UsdGeomXformOp transformOp = xformable.MakeMatrixXform();
		const pxr::UsdTimeCode timeCode = m_root->getTime( time );
		transformOp.Set( DataAlgo::toUSD( m44->readable() ), timeCode );
	}
}

namespace
{

const IECore::InternedString g_purposeAttributeName( "usd:purpose" );
const IECore::InternedString g_kindAttributeName( "usd:kind" );
const IECore::InternedString g_lightAttributeName( "light" );
const IECore::InternedString g_doubleSidedAttributeName( "doubleSided" );

} // namespace

bool USDScene::hasAttribute( const SceneInterface::Name &name ) const
{
	if( m_location->prim.IsPseudoRoot() )
	{
		// Can't store attributes here.
		return false;
	}
	if( name == SceneInterface::visibilityName )
	{
		return pxr::UsdGeomImageable( m_location->prim ).GetVisibilityAttr().HasAuthoredValue();
	}
	else if( name == g_purposeAttributeName )
	{
		pxr::UsdGeomImageable imageable( m_location->prim );
		return imageable && imageable.GetPurposeAttr().HasAuthoredValue();
	}
	else if( name == g_kindAttributeName )
	{
		pxr::UsdModelAPI model( m_location->prim );
		pxr::TfToken kind;
		return model.GetKind( &kind );
	}
#if PXR_VERSION >= 2111
	else if( name == g_lightAttributeName )
	{
		return m_location->prim.HasAPI<pxr::UsdLuxLightAPI>();
	}
#endif
	else if( name == g_doubleSidedAttributeName )
	{
		return pxr::UsdGeomGprim( m_location->prim ).GetDoubleSidedAttr().HasAuthoredValue();
	}
	else if( auto attribute = AttributeAlgo::findUSDAttribute( m_location->prim, name.string() ) )
	{
		return attribute.HasAuthoredValue();
	}
	else
	{
		const auto &[output, purpose] = materialOutputAndPurpose( name.string() );
		if( pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim, purpose ) )
		{
			if( pxr::UsdShadeOutput o = mat.GetOutput( output ) )
			{
				return ShaderAlgo::canReadShaderNetwork( o );
			}
		}
		return false;
	}
}

void USDScene::attributeNames( SceneInterface::NameList &attrs ) const
{
	attrs.clear();
	if( m_location->prim.IsPseudoRoot() )
	{
		// No attributes here
		return;
	}

	pxr::UsdGeomImageable imageable( m_location->prim );
	if( imageable.GetVisibilityAttr().HasAuthoredValue() )
	{
		attrs.push_back( SceneInterface::visibilityName );
	}
	if( imageable && imageable.GetPurposeAttr().HasAuthoredValue() )
	{
		attrs.push_back( g_purposeAttributeName );
	}

	pxr::TfToken kind;
	if( pxr::UsdModelAPI( m_location->prim ).GetKind( &kind ) )
	{
		attrs.push_back( g_kindAttributeName );
	}

#if PXR_VERSION >= 2111
	if( m_location->prim.HasAPI<pxr::UsdLuxLightAPI>() )
	{
		attrs.push_back( g_lightAttributeName );
	}
#endif

	if( pxr::UsdGeomGprim( m_location->prim ).GetDoubleSidedAttr().HasAuthoredValue() )
	{
		attrs.push_back( g_doubleSidedAttributeName );
	}

	std::vector<pxr::UsdAttribute> attributes = m_location->prim.GetAuthoredAttributes();
	for( const auto &attribute : attributes )
	{
		if( !attribute.HasAuthoredValue() )
		{
			continue;
		}
		IECore::InternedString name = IECoreUSD::AttributeAlgo::cortexAttributeName( attribute );
		if( name.string().size() )
		{
			attrs.push_back( name );
		}
	}

	for( const auto &purpose : { pxr::UsdShadeTokens->allPurpose, pxr::UsdShadeTokens->preview, pxr::UsdShadeTokens->full } )
	{
		if( pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim, purpose ) )
		{
			for( pxr::UsdShadeOutput &o : mat.GetOutputs( /* onlyAuthored = */ true ) )
			{
				if( !ShaderAlgo::canReadShaderNetwork( o ) )
				{
					continue;
				}
				InternedString attrName = AttributeAlgo::nameFromUSD( { o.GetBaseName() , false } );
				if( !purpose.IsEmpty() )
				{
					attrName = attrName.string() + ":" + purpose.GetString();
				}
				attrs.push_back( attrName );
			}
		}
	}
}

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{

	if( m_location->prim.IsPseudoRoot() )
	{
		// No attributes here
		return nullptr;
	}


	if( name == SceneInterface::visibilityName )
	{
		pxr::UsdGeomImageable imageable( m_location->prim );
		if( !imageable )
		{
			return nullptr;
		}
		auto attr = imageable.GetVisibilityAttr();
		if( !attr.HasAuthoredValue() )
		{
			return nullptr;
		}
		pxr::TfToken value; attr.Get( &value, m_root->getTime( time ) );
		if( value == pxr::UsdGeomTokens->inherited )
		{
			return new BoolData( true );
		}
		else if( value == pxr::UsdGeomTokens->invisible )
		{
			return new BoolData( false );
		}
		return nullptr;
	}
	else if( name == g_purposeAttributeName )
	{
		pxr::UsdGeomImageable imageable( m_location->prim );
		if( !imageable )
		{
			return nullptr;
		}
		auto attr = imageable.GetPurposeAttr();
		if( !attr.HasAuthoredValue() )
		{
			return nullptr;
		}
		pxr::TfToken value; attr.Get( &value );
		return new StringData( value.GetString() );
	}
#if PXR_VERSION >= 2111
	else if( name == g_lightAttributeName )
	{
		return ShaderAlgo::readLight( pxr::UsdLuxLightAPI( m_location->prim ) );
	}
#endif
	else if( name == g_kindAttributeName )
	{
		pxr::TfToken kind;
		if( !pxr::UsdModelAPI( m_location->prim ).GetKind( &kind ) )
		{
			return nullptr;
		}
		return new StringData( kind.GetString() );
	}
	else if( name == g_doubleSidedAttributeName )
	{
		pxr::UsdAttribute attr = pxr::UsdGeomGprim( m_location->prim ).GetDoubleSidedAttr();
		bool doubleSided;
		if( attr.HasAuthoredValue() && attr.Get( &doubleSided, m_root->getTime( time ) ) )
		{
			return new BoolData( doubleSided );
		}
		return nullptr;
	}
	else if( pxr::UsdAttribute attribute = AttributeAlgo::findUSDAttribute( m_location->prim, name.string() ) )
	{
		return DataAlgo::fromUSD( attribute, m_root->getTime( time ) );
	}
	else
	{
		const auto &[output, purpose] = materialOutputAndPurpose( name.string() );
		if( pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim, purpose ) )
		{
			if( pxr::UsdShadeOutput o = mat.GetOutput( output ) )
			{
				return m_root->readShaderNetwork( o );
			}
		}
		return nullptr;
	}
}

void USDScene::writeAttribute( const SceneInterface::Name &name, const Object *attribute, double time )
{
	if( name == SceneInterface::visibilityName )
	{
		if( auto *data = reportedCast<const BoolData>( attribute, "USDScene::writeAttribute", name.c_str() ) )
		{
			pxr::UsdGeomImageable imageable( m_location->prim );
			imageable.GetVisibilityAttr().Set(
				data->readable() ? pxr::UsdGeomTokens->inherited : pxr::UsdGeomTokens->invisible,
				m_root->getTime( time )
			);
		}
	}
	else if( name == g_purposeAttributeName )
	{
		if( auto *data = reportedCast<const StringData>( attribute, "USDScene::writeAttribute", name.c_str() ) )
		{
			pxr::UsdGeomImageable imageable( m_location->prim );
			imageable.GetPurposeAttr().Set( pxr::TfToken( data->readable() ) );
		}
	}
	else if( name == g_kindAttributeName )
	{
		if( auto *data = reportedCast<const StringData>( attribute, "USDScene::writeAttribute", name.c_str() ) )
		{
			pxr::UsdModelAPI model( m_location->prim );
			if( !model.SetKind( pxr::TfToken( data->readable() ) ) )
			{
				IECore::msg(
					IECore::Msg::Warning, "USDScene::writeAttribute",
					boost::format( "Unable to write kind \"%1%\" to \"%2%\"" ) % data->readable() % m_location->prim.GetPath()
				);
			}
		}
	}
	else if( name == g_doubleSidedAttributeName )
	{
		if( auto *data = reportedCast<const BoolData>( attribute, "USDScene::writeAttribute", name.c_str() ) )
		{
			pxr::UsdGeomGprim gprim( m_location->prim );
			if( gprim )
			{
				gprim.GetDoubleSidedAttr().Set( data->readable(), m_root->getTime( time ) );
			}
			else
			{
				// We're hamstrung by the fact that USD considers `doubleSided` to be a property
				// of a Gprim and not an inheritable attribute as it was in RenderMan and is in Cortex.
				// We can't author a Gprim here, because it isn't a concrete type, so we must rely on
				// `writeObject()` having been called first to get a suitable concrete type in place.
				IECore::msg(
					IECore::Msg::Warning, "USDScene::writeAttribute",
					boost::format( "Unable to write attribute \"%1%\" to \"%2%\", because it is not a Gprim" ) % name % m_location->prim.GetPath()
				);
			}
		}
	}
	else if( const IECoreScene::ShaderNetwork *shaderNetwork = runTimeCast<const ShaderNetwork>( attribute ) )
	{
		if( name == g_lightAttributeName )
		{
			ShaderAlgo::writeLight( shaderNetwork, m_location->prim );
		}
		else
		{
			const auto &[output, purpose] = materialOutputAndPurpose( name.string() );
			m_materials[purpose][output] = shaderNetwork;
		}
	}
	else if( name.string() == "gaffer:globals" )
	{
		// This is some very preliminary support for globals - we just support Arnold options, and don't read
		// them yet.  But this is already enough to test out some stuff with reading Gaffer's USD's in Arnold
		if( const IECore::CompoundObject *globals = runTimeCast<const CompoundObject>( attribute ) )
		{
			for( const auto &g : globals->members() )
			{
				const IECore::Data *d = IECore::runTimeCast<const Data>( g.second.get() );
				if( d && boost::algorithm::starts_with( g.first.string(), "option:ai:" ) )
				{
					pxr::UsdPrim options = m_root->getStage()->DefinePrim( pxr::SdfPath( "/options" ), pxr::TfToken( "ArnoldOptions" ) );
					pxr::UsdAttribute globalAttribute = options.CreateAttribute(
						pxr::TfToken( g.first.string().substr( 10 ) ),
						DataAlgo::valueTypeName( d )
					);
					globalAttribute.Set( DataAlgo::toUSD( d ) );
				}
			}
		}
	}
	else if( name.string().find( ':' ) != std::string::npos )
	{
		if( const Data *data = runTimeCast<const Data>( attribute ) )
		{
			AttributeAlgo::Name usdName = AttributeAlgo::nameToUSD( name.string() );
			if ( usdName.isPrimvar )
			{
				const pxr::UsdGeomPrimvarsAPI primvarsAPI = pxr::UsdGeomPrimvarsAPI( m_location->prim );
				pxr::UsdGeomPrimvar usdPrimvar = primvarsAPI.CreatePrimvar(
					usdName.name, DataAlgo::valueTypeName( data ), pxr::UsdGeomTokens->constant
				);
				usdPrimvar.Set( DataAlgo::toUSD( data ), time );
			}
			else
			{
				pxr::UsdAttribute newAttribute = m_location->prim.CreateAttribute(
					usdName.name, DataAlgo::valueTypeName( data )
				);
				newAttribute.Set( DataAlgo::toUSD( data ), time );
			}
		}
	}
}

bool USDScene::hasTag( const SceneInterface::Name &name, int filter ) const
{
	// Get read access to set in `tagSets`.

	IO::TagSetsMap::const_accessor readAccessor;
	if( !m_root->tagSets.find( readAccessor, name ) )
	{
		// Set not loaded yet. Use a write accessor to do the work, and
		// then downgrade to read access.
		IO::TagSetsMap::accessor writeAccessor;
		if( m_root->tagSets.insert( writeAccessor, name ) )
		{
			// \todo - we should be passing through a canceller here, but I guess the long term plan is
			// to get rid of the tag interface and use the set interface directly from Gaffer
			// If we do add canceller support, we would need to make sure that this code is threadsafe:
			// currently a canceller readSetInternal would result in an unfilled writeAccessor being inserted
			writeAccessor->second = readSetInternal( m_root->root(), pxr::TfToken( name.string() ), /* includeDescendantSets = */ true, /* canceller = */ nullptr );
		}
		writeAccessor.release();
		m_root->tagSets.find( readAccessor, name );
	}

	// Search set to generate tags

	SceneInterface::Path p; path( p );
	const unsigned match = readAccessor->second.match( p );
	return
		( ( filter & SceneInterface::AncestorTag ) && ( match & PathMatcher::AncestorMatch ) ) ||
		( ( filter & SceneInterface::LocalTag ) && ( match & PathMatcher::ExactMatch ) ) ||
		( ( filter & SceneInterface::DescendantTag ) && ( match & PathMatcher::DescendantMatch ) )
	;
}

void USDScene::readTags( SceneInterface::NameList &tags, int filter ) const
{
	tags.clear();
	if( m_location->prim.IsPseudoRoot() )
	{
		// Special case. Gaffer uses this to implement `computeSetNames()`, and
		// we definitely do not want to load all the sets just to achieve that.
		// Gaffer implements `computeSet()` via `hasTag()`, so as long as we
		// don't load every set now, we can load only them on demand in `hasTag()`.
		if( filter & SceneInterface::DescendantTag )
		{
			tags = m_root->allTags();
		}
		return;
	}

	for( const auto &tag : m_root->allTags() )
	{
		if( hasTag( tag, filter ) )
		{
			tags.push_back( tag );
		}
	}
}

void USDScene::writeTags( const SceneInterface::NameList &tags )
{
	if( tags.empty() )
	{
		return;
	}

	const Path p = fromUSD( m_location->prim.GetPath() );
	for( const auto &tag : tags )
	{
		IO::TagSetsMap::accessor a;
		m_root->tagSets.insert( a, tag );
		a->second.addPath( p );
	}
}

SceneInterface::NameList USDScene::setNames( bool includeDescendantSets ) const
{
	if( includeDescendantSets )
	{
		pxr::SdfPathSet visitedPrototypes;
		return recursiveSetNames( m_location->prim, visitedPrototypes );
	}
	else
	{
		return localSetNames( m_location->prim );
	}
}

PathMatcher USDScene::readSet( const Name &name, bool includeDescendantSets, const Canceller *canceller ) const
{
	return readSetInternal( m_location->prim, pxr::TfToken( name.string() ), includeDescendantSets, canceller );
}

void USDScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	writeSetInternal( m_location->prim, pxr::TfToken( name.string() ), set );
}

void USDScene::hashSet( const Name &name, IECore::MurmurHash &h ) const
{
	SceneInterface::hashSet( name, h );

	h.append( m_root->uniqueId() );
	append( m_location->prim.GetPath(), h );
	h.append( name );
}

bool USDScene::hasObject() const
{
	return ObjectAlgo::canReadObject( m_location->prim );
}

PrimitiveVariableMap USDScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	return PrimitiveVariableMap();
}

void USDScene::writeObject( const Object *object, double time )
{
	if( !ObjectAlgo::writeObject( object, m_root->getStage(), m_location->prim.GetPath(), m_root->getTime( time ) ) )
	{
		IECore::msg(
			IECore::Msg::Warning, "USDScene::writeObject",
			boost::format( "Unable to write %1% to \"%2%\" at time %3%" ) % object->typeName() % m_location->prim.GetPath() % time
		);
	}
}

bool USDScene::hasChild( const SceneInterface::Name &name ) const
{
	pxr::UsdPrim childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );
	return isSceneChild( childPrim );
}

void USDScene::childNames( SceneInterface::NameList &childNames ) const
{
	for( const auto &prim : m_location->prim.GetFilteredChildren( pxr::UsdTraverseInstanceProxies() ) )
	{
		if( isSceneChild( prim ) )
		{
			childNames.push_back( IECore::InternedString( prim.GetName() ) );
		}
	}
}

SceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour )
{
	pxr::UsdPrim childPrim;
	const bool validIdentifier = pxr::TfIsValidIdentifier( name.string() );
	if( validIdentifier )
	{
		childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );
	}

	if( isSceneChild( childPrim ) )
	{
		return new USDScene( m_root, new Location( childPrim ) );
	}

	switch( missingBehaviour )
	{
		case SceneInterface::NullIfMissing :
			return nullptr;
		case SceneInterface::ThrowIfMissing :
			if( !validIdentifier )
			{
				throw InvalidArgumentException( "USDScene::child : Name \"" + name.string() + "\" is not a valid identifier" );
			}
			else if( !childPrim )
			{
				throw IOException( "USDScene::child : UsdPrim \"" + m_location->prim.GetPath().GetAsString() + "\" has no child named \"" + name.string() + "\"" );
			}
			else
			{
				throw IOException( "USDScene::child : UsdPrim \"" + childPrim.GetPath().GetAsString() + "\" does not contribute to the scene hierarchy" );
			}
		case SceneInterface::CreateIfMissing :
		{
			if( m_root->openMode() == IndexedIO::Read )
			{
				throw InvalidArgumentException( "Child creation not supported" );
			}
			else
			{
				return createChild( name );
			}
		}
		default:
			return nullptr;
	}
}

ConstSceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->child( name, missingBehaviour );
}

SceneInterfacePtr USDScene::createChild( const SceneInterface::Name &name )
{
	pxr::UsdPrim prim = m_location->prim;
	pxr::SdfPath newPath = prim.GetPath().AppendChild( validName( name ) );
	pxr::UsdGeomXform newXform = pxr::UsdGeomXform::Define( m_root->getStage(), newPath );

	return new USDScene( m_root, new Location( newXform.GetPrim() ) );
}

SceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour )
{
	SceneInterfacePtr result = this;
	for( const auto &childName : path )
	{
		result = result->child( childName, missingBehaviour );
		if( !result )
		{
			break;
		}
	}
	return result;
}

ConstSceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->scene( path, missingBehaviour );
}

void USDScene::hash( SceneInterface::HashType hashType, double time, MurmurHash &h ) const
{
	SceneInterface::hash( hashType, time, h );

	h.append( hashType );

	switch( hashType )
	{
		case SceneInterface::TransformHash:
			transformHash( time, h );
			break;
		case SceneInterface::AttributesHash:
			attributesHash( time, h );
			break;
		case SceneInterface::BoundHash:
			boundHash( time, h );
			break;
		case SceneInterface::ObjectHash:
			objectHash( time, h );
			break;
		case SceneInterface::ChildNamesHash:
			childNamesHash( time, h );
			break;
		case SceneInterface::HierarchyHash:
			hierarchyHash( time, h );
			break;
	}
}

void USDScene::boundHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim ) )
	{
		h.append( m_root->uniqueId() );
		appendPrimOrMasterPath( m_location->prim, h );
		if( boundable.GetExtentAttr().ValueMightBeTimeVarying() )
		{
			h.append( time );
		}
	}
}

void USDScene::transformHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomXformable xformable = pxr::UsdGeomXformable( m_location->prim ) )
	{
		h.append( m_root->uniqueId() );
		appendPrimOrMasterPath( m_location->prim, h );

		bool mightBeTimeVarying = xformable.TransformMightBeTimeVarying();
		if( !mightBeTimeVarying && xformable.GetResetXformStack() )
		{
			// Because we have to apply the inverse of our parent's transform, if
			// that is time varying then so are we.
			pxr::UsdPrim parentPrim = m_location->prim.GetParent();
			while( parentPrim )
			{
				pxr::UsdGeomXformable parentXFormable( parentPrim );
				if( parentXFormable && parentXFormable.TransformMightBeTimeVarying() )
				{
					mightBeTimeVarying = true;
					break;
				}
				parentPrim = parentPrim.GetParent();
			}
		}

		if( mightBeTimeVarying )
		{
			h.append( time );
		}
	}
}

void USDScene::attributesHash( double time, IECore::MurmurHash &h ) const
{
	bool haveAttributes = false;
	bool mightBeTimeVarying = false;

	pxr::UsdGeomImageable imageable( m_location->prim );
	auto visibilityAttr = imageable.GetVisibilityAttr();
	if( visibilityAttr.HasAuthoredValue() )
	{
		haveAttributes = true;
		mightBeTimeVarying = visibilityAttr.ValueMightBeTimeVarying();
	}
	if( imageable && imageable.GetPurposeAttr().HasAuthoredValue() )
	{
		haveAttributes = true;
		// Purpose can not be animated so no need to update `mightBeTimeVarying`.
	}

	pxr::TfToken kind;
	if( pxr::UsdModelAPI( m_location->prim ).GetKind( &kind ) )
	{
		haveAttributes = true;
		// Kind can not be animated so no need to update `mightBeTimeVarying`.
	}

#if PXR_VERSION >= 2111
	if( m_location->prim.HasAPI<pxr::UsdLuxLightAPI>() )
	{
		/// \todo Consider time-varying lights - see comment below
		/// for materials.
		haveAttributes = true;
	}
#endif

	auto doubleSidedAttr = pxr::UsdGeomGprim( m_location->prim ).GetDoubleSidedAttr();
	if( doubleSidedAttr && doubleSidedAttr.HasAuthoredValue() )
	{
		haveAttributes = true;
		mightBeTimeVarying |= doubleSidedAttr.ValueMightBeTimeVarying();
	}

	std::vector<pxr::UsdAttribute> attributes = m_location->prim.GetAuthoredAttributes();
	for( const auto &attribute : attributes )
	{
		if( IECoreUSD::AttributeAlgo::cortexAttributeName( attribute ).string().size() )
		{
			haveAttributes = true;
			if( attribute.ValueMightBeTimeVarying() )
			{
				mightBeTimeVarying = true;
				break;
			}
		}
	}

	bool haveMaterials = false;
	for( const auto &purpose : { pxr::UsdShadeTokens->allPurpose, pxr::UsdShadeTokens->preview, pxr::UsdShadeTokens->full } )
	{
		if( pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim, purpose ) )
		{
			// \todo - This does not consider the possibility that the material could contain time-varying
			// attributes
			append( mat.GetPrim().GetPath(), h );
			haveMaterials = true;
		}
	}

	if( haveAttributes || haveMaterials )
	{
		h.append( m_root->uniqueId() );

		if( haveAttributes )
		{
			// \todo - Seems pretty harmful that having an attribute at the location results in it having
			// a unique hash, even if the attribute is the same, especially if we end up doing shader
			// parsing work per location
			appendPrimOrMasterPath( m_location->prim, h );
		}

		if( mightBeTimeVarying )
		{
			h.append( time );
		}
	}
}

void USDScene::objectHash( double time, IECore::MurmurHash &h ) const
{
	if( ObjectAlgo::canReadObject( m_location->prim ) )
	{
		h.append( m_root->uniqueId() );
		appendPrimOrMasterPath( m_location->prim, h );
		if( ObjectAlgo::objectMightBeTimeVarying( m_location->prim ) )
		{
			h.append( time );
		}
	}
}
void USDScene::childNamesHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_root->uniqueId() );
	appendPrimOrMasterPath( m_location->prim, h );
}

void USDScene::hierarchyHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_root->uniqueId() );
	appendPrimOrMasterPath( m_location->prim, h );
	h.append( time );
}

SceneInterface::Path USDScene::fromUSD( const pxr::SdfPath &path )
{
	return fromUSDWithoutPrefix( path, 0 );
}

pxr::SdfPath USDScene::toUSD( const SceneInterface::Path &path, const bool relative )
{
	pxr::SdfPath result = relative ? pxr::SdfPath::ReflexiveRelativePath() : pxr::SdfPath::AbsoluteRootPath();
	for( const auto &name : path )
	{
		result = result.AppendElementString( name.string() );
	}
	return result;
}

namespace
{

SceneInterface::FileFormatDescription<USDScene> g_descriptionUSD( ".usd", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDA( ".usda", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDC( ".usdc", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDZ( ".usdz", IndexedIO::Read | IndexedIO::Write );

} // namespace
