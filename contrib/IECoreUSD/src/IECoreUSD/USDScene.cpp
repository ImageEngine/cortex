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
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdGeom/pointInstancer.h"
#include "pxr/usd/usdGeom/primvar.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#include "pxr/usd/usdGeom/scope.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/materialBindingAPI.h"
#include "pxr/usd/usdShade/connectableAPI.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/container/flat_map.hpp"
#include "boost/format.hpp"
#include "boost/functional/hash.hpp"

#include "tbb/concurrent_hash_map.h"

#include <iostream>

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
	if( !prim.IsDefined() || prim.GetName() == g_tagsPrimName )
	{
		return false;
	}

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

template<typename SchemaType>
IECore::PathMatcher readSchemaTypeSet( const pxr::UsdPrim &prim )
{
	IECore::PathMatcher result;
	for( const auto &descendant : prim.GetDescendants() )
	{
		if( descendant.IsA<SchemaType>() )
		{
			result.addPath( USDScene::fromUSD( descendant.GetPath() ) );
		}
	}
	return result;
}

boost::container::flat_map<IECore::InternedString, IECore::PathMatcher (*)( const pxr::UsdPrim & )> g_schemaTypeSetReaders = {
	{ "__cameras", readSchemaTypeSet<pxr::UsdGeomCamera> },
	{ "usd:pointInstancers", readSchemaTypeSet<pxr::UsdGeomPointInstancer> }
};

IECore::PathMatcher readSetInternal( const pxr::UsdPrim &prim, const pxr::TfToken &name, bool includeDescendantSets, const Canceller *canceller )
{
	// Special cases for auto-generated sets

	auto it = g_schemaTypeSetReaders.find( name.GetString() );
	if( it != g_schemaTypeSetReaders.end() )
	{
		if( !prim.IsPseudoRoot() )
		{
			return PathMatcher();
		}
		return it->second( prim );
	}

	IECore::PathMatcher result;

	// Read set from local collection

	const size_t prefixSize = prim.GetPath().GetPathElementCount();
	if( auto collection = pxr::UsdCollectionAPI( prim, name ) )
	{
		pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
		pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths( membershipQuery, prim.GetStage() );

		for( const auto &path : includedPaths )
		{
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

	// Recurse to descendant collections

	if( includeDescendantSets )
	{
		Canceller::check( canceller );

		/// \todo We could visit each instance master only once, and then instance in the set collected
		/// from it.
		for( const auto &childPrim : prim.GetFilteredChildren( pxr::UsdTraverseInstanceProxies() ) )
		{
			if( !isSceneChild( childPrim ) )
			{
				continue;
			}

			IECore::PathMatcher childSet = readSetInternal( childPrim, name, includeDescendantSets, canceller );
			if( !childSet.isEmpty() )
			{
				result.addPaths( childSet, { childPrim.GetPath().GetName() } );
			}
		}
	}

	return result;
}

SceneInterface::NameList setNamesInternal( const pxr::UsdPrim &prim, bool includeDescendantSets )
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
		for( const auto &s : g_schemaTypeSetReaders )
		{
			result.push_back( s.first );
		}
	}

	if( includeDescendantSets )
	{
		for( const auto &childPrim : prim.GetFilteredChildren( pxr::UsdTraverseInstanceProxies() ) )
		{
			if( !isSceneChild( childPrim ) )
			{
				continue;
			}
			SceneInterface::NameList childSetNames = setNamesInternal( childPrim, includeDescendantSets );
			result.insert( result.end(), childSetNames.begin(), childSetNames.end() );
		}

		// Remove duplicates
		std::sort( result.begin(), result.end() );
		result.erase( std::unique( result.begin(), result.end() ), result.end() );
	}

	return result;
}

void populateMaterial( pxr::UsdShadeMaterial &mat, const std::map< const InternedString, ConstShaderNetworkPtr > &shaderTypes )
{
	for( auto &shaderType : shaderTypes )
	{
		std::string type = AttributeAlgo::nameToUSD( shaderType.first.string() ).name.GetString();
		std::string prefix;
		size_t colonPos = type.rfind( ":" );
		if( colonPos != std::string::npos )
		{
			prefix = type.substr( 0, colonPos );
			type = type.substr( colonPos + 1 );
		}

		pxr::UsdShadeOutput matOutput;
		pxr::TfToken renderContext = prefix.size() ? pxr::TfToken( prefix ) : pxr::UsdShadeTokens->universalRenderContext;
		if( type == "surface" )
		{
			matOutput = mat.CreateSurfaceOutput( renderContext );
		}
		else if( type == "displacement" )
		{
			matOutput = mat.CreateDisplacementOutput( renderContext );
		}
		else if( type == "volume" )
		{
			matOutput = mat.CreateVolumeOutput( renderContext );
		}
		else
		{
			IECore::msg(
				IECore::Msg::Warning, "IECoreUSD::ShaderAlgo::writeShaderNetwork",
				boost::format( "Unrecognized shader type \"%1%\"" ) % type
			);

			continue;
		}

		std::string shaderContainerName = boost::replace_all_copy( shaderType.first.string(), ":", "_" ) + "_shaders";
		pxr::UsdGeomScope shaderContainer = pxr::UsdGeomScope::Define( mat.GetPrim().GetStage(), mat.GetPath().AppendChild( pxr::TfToken( shaderContainerName ) ) );
		pxr::UsdShadeOutput networkOut = ShaderAlgo::writeShaderNetwork( shaderType.second.get(), shaderContainer.GetPrim() );

		if( networkOut.GetPrim().IsValid() )
		{
			matOutput.ConnectToSource( networkOut );
		}
	}
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
			IECoreScene::ConstShaderNetworkPtr result;

			/// \todo I'm pretty sure that the `readShaderNetwork()` signature is overly complex,
			/// and it should just be passed a single `UsdShadeOutput &` like this function.
			/// I suspect that `writeShaderNetwork()` could take a single `UsdShadeOutput &` too,
			/// for symmetry between the two functions.

			pxr::UsdShadeConnectableAPI source;
			pxr::TfToken sourceName;
			pxr::UsdShadeAttributeType sourceType;
			if( key.GetConnectedSource( &source, &sourceName, &sourceType ) )
			{
				pxr::UsdShadeShader s( source.GetPrim() );
				result = ShaderAlgo::readShaderNetwork( source.GetPrim().GetParent().GetPath(), s, sourceName );
			}
			else
			{
				result = new IECoreScene::ShaderNetwork();
			}

			cost = result->Object::memoryUsage();
			return result;
		}

};

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
				m_shaderNetworkCache( 10 * 1024 * 1024 ) // 10Mb
		{
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
					m_allTags = setNamesInternal( m_rootPrim, /* includeDescendantSets = */ true );
				}
			);
			return m_allTags;
		}

		pxr::UsdShadeMaterial computeBoundMaterial( const pxr::UsdPrim &prim )
		{
			// This should be thread safe, despite using caches, because
			// BindingsCache and CollectionQueryCache are implemented by USD as
			// tbb::concurrent_unordered_map
			return pxr::UsdShadeMaterialBindingAPI( prim ).ComputeBoundMaterial(
				&m_usdBindingsCache, &m_usdCollectionQueryCache
			);
		}

		IECoreScene::ConstShaderNetworkPtr readShaderNetwork( const pxr::UsdShadeOutput &output )
		{
			return m_shaderNetworkCache.get( output );
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

		pxr::UsdShadeMaterialBindingAPI::BindingsCache m_usdBindingsCache;
		pxr::UsdShadeMaterialBindingAPI::CollectionQueryCache m_usdCollectionQueryCache;

		ShaderNetworkCache m_shaderNetworkCache;

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
	if( m_shaders.size() )
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

			// Use a hash to identify the combination of shaders in this material
			IECore::MurmurHash materialHash;
			for( auto &shaderType : m_shaders )
			{
				materialHash.append( shaderType.first );
				materialHash.append( shaderType.second->Object::hash() );
			}
			pxr::TfToken matName( "material_" + materialHash.toString() );

			pxr::SdfPath matPath = materialContainer.GetPrim().GetPath().AppendChild( matName );
			pxr::UsdShadeMaterial mat = pxr::UsdShadeMaterial::Get( materialContainer.GetPrim().GetStage(), matPath );
			if( !mat )
			{
				// Another location has not yet defined this material
				mat = pxr::UsdShadeMaterial::Define( materialContainer.GetPrim().GetStage(), matPath );
				populateMaterial( mat, m_shaders );
			}
			pxr::UsdShadeMaterialBindingAPI( m_location->prim ).Bind( mat );
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
	pxr::UsdGeomXformable transformable( m_location->prim );
	if( !transformable )
	{
		return Imath::M44d();
	}

	bool zUp = m_location->prim.GetParent().IsPseudoRoot() && pxr::UsdGeomGetStageUpAxis( m_root->getStage() ) == pxr::UsdGeomTokens->z;

	pxr::GfMatrix4d transform;
	bool reset = false;

	transformable.GetLocalTransformation( &transform, &reset, m_root->getTime( time ) );
	Imath::M44d returnValue = DataAlgo::fromUSD( transform );

	if ( zUp )
	{
		static Imath::M44d b
			(
				0, 0, 1, 0,
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 1
			);

		returnValue = returnValue * b;
	}
	return returnValue;
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
	else if( auto attribute = AttributeAlgo::findUSDAttribute( m_location->prim, name.string() ) )
	{
		return attribute.HasAuthoredValue();
	}
	else
	{
		pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim );
		pxr::UsdPrim matPrim = mat.GetPrim();

		if( matPrim.IsValid() )
		{
			pxr::TfToken n = AttributeAlgo::nameToUSD( name.string() ).name;
			pxr::UsdShadeOutput o = mat.GetOutput( n );
			if( o && pxr::UsdAttribute( o ).IsAuthored() )
			{
				return true;
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

	pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim );

	if( mat.GetPrim().IsValid() )
	{
		for( pxr::UsdShadeOutput &o : mat.GetOutputs() )
		{
			if( o && pxr::UsdAttribute( o ).IsAuthored() )
			{
				attrs.push_back( AttributeAlgo::nameFromUSD( { o.GetBaseName(), false } ) );
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
	else if( name == g_kindAttributeName )
	{
		pxr::TfToken kind;
		if( !pxr::UsdModelAPI( m_location->prim ).GetKind( &kind ) )
		{
			return nullptr;
		}
		return new StringData( kind.GetString() );
	}
	else if( pxr::UsdAttribute attribute = AttributeAlgo::findUSDAttribute( m_location->prim, name.string() ) )
	{
		return DataAlgo::fromUSD( attribute, m_root->getTime( time ) );
	}
	else
	{
		pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim );

		if( mat.GetPrim().IsValid() )
		{
			pxr::TfToken n = AttributeAlgo::nameToUSD( name.string() ).name;

			// If there's no output declared, then we will return nullptr, versus
			// having an output with no source connected, which will return an
			// empty shader network
			pxr::UsdShadeOutput o = mat.GetOutput( n );
			if( o && pxr::UsdAttribute( o ).IsAuthored() )
			{
				return m_root->readShaderNetwork( o );
			}
		}
	}

	return nullptr;
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
	else if( const IECoreScene::ShaderNetwork *shaderNetwork = runTimeCast<const ShaderNetwork>( attribute ) )
	{
		m_shaders[name] = shaderNetwork;
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
	return setNamesInternal( m_location->prim, includeDescendantSets );
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

	h.append( m_root->fileName() );
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

	return (bool)childPrim;
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
	if( pxr::TfIsValidIdentifier( name.string() ) )
	{
		childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );
	}

	if( childPrim && isSceneChild( childPrim ) )
	{
		return new USDScene( m_root, new Location( childPrim ) );
	}

	switch( missingBehaviour )
	{
		case SceneInterface::NullIfMissing :
			return nullptr;
		case SceneInterface::ThrowIfMissing :
			throw IOException( "Child \"" + name.string() + "\" does not exist" );
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
		h.append( m_root->fileName() );
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
		h.append( m_root->fileName() );
		appendPrimOrMasterPath( m_location->prim, h );
		if( xformable.TransformMightBeTimeVarying() )
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

	pxr::UsdShadeMaterial mat = m_root->computeBoundMaterial( m_location->prim );

	if( haveAttributes || mat.GetPrim().IsValid() )
	{
		h.append( m_root->fileName() );

		if( haveAttributes )
		{
			// \todo - Seems pretty harmful that having an attribute at the location results in it having
			// a unique hash, even if the attribute is the same, especially if we end up doing shader
			// parsing work per location
			appendPrimOrMasterPath( m_location->prim, h );
		}

		if( mat.GetPrim().IsValid() )
		{
			// \todo - This does not consider the possibility that the material could contain time-varying
			// attributes
			append( mat.GetPrim().GetPath(), h );
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
		h.append( m_root->fileName() );
		appendPrimOrMasterPath( m_location->prim, h );
		if( ObjectAlgo::objectMightBeTimeVarying( m_location->prim ) )
		{
			h.append( time );
		}
	}
}
void USDScene::childNamesHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_root->fileName() );
	appendPrimOrMasterPath( m_location->prim, h );
}

void USDScene::hierarchyHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_root->fileName() );
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

} // namespace
