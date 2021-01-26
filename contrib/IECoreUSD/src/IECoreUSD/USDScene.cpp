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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"

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
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/container/flat_map.hpp"
#include "boost/format.hpp"
#include "boost/functional/hash.hpp"

#include <iostream>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

/// \todo Use the standard PXR_VERSION instead. We can't do that until
/// everyone is using USD 19.11 though, because prior to that PXR_VERSION
/// was malformed (octal, and not comparable in any way).
#define USD_VERSION ( PXR_MAJOR_VERSION * 10000 + PXR_MINOR_VERSION * 100 + PXR_PATCH_VERSION )

#if USD_VERSION < 1903
#define HasAuthoredValue HasAuthoredValueOpinion
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
		append( prim.GetPrimInMaster().GetPrimPath(), h );
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

SceneInterface::Path fromUSD( const pxr::SdfPath &path )
{
	return fromUSDWithoutPrefix( path, 0 );
}

pxr::SdfPath toUSD( const SceneInterface::Path &path, const bool relative = false )
{
	pxr::SdfPath result = relative ? pxr::SdfPath::ReflexiveRelativePath() : pxr::SdfPath::AbsoluteRootPath();
	for( const auto &name : path )
	{
		result = result.AppendElementString( name.string() );
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

bool isSceneChild( const pxr::UsdPrim &prim )
{
	if( !prim.IsDefined() || prim.GetName() == g_tagsPrimName )
	{
		return false;
	}

	return
		prim.GetTypeName().IsEmpty() ||
		pxr::UsdGeomImageable( prim )
	;
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
			pxr::UsdPrim childPrim = prim.GetStage()->DefinePrim( toUSD( *it ) );
			writeSetInternal( childPrim, name, set.subTree( *it ) );
			it.prune(); // Only visit children of root
		}
		return;
	}

	pxr::SdfPathVector targets;
	for( PathMatcher::Iterator it = set.begin(); it != set.end(); ++it )
	{
		targets.push_back( toUSD( *it, /* relative = */ true ) );
	}

	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( prim, name, pxr::UsdTokens->explicitOnly );
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
			result.addPath( fromUSD( descendant.GetPath() ) );
		}
	}
	return result;
}

boost::container::flat_map<IECore::InternedString, IECore::PathMatcher (*)( const pxr::UsdPrim & )> g_schemaTypeSetReaders = {
	{ "__cameras", readSchemaTypeSet<pxr::UsdGeomCamera> },
	{ "usd:pointInstancers", readSchemaTypeSet<pxr::UsdGeomPointInstancer> }
};

IECore::PathMatcher readSetInternal( const pxr::UsdPrim &prim, const pxr::TfToken &name, bool includeDescendantSets )
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
		/// \todo We could visit each instance master only once, and then instance in the set collected
		/// from it.
		for( const auto &childPrim : prim.GetFilteredChildren( pxr::UsdTraverseInstanceProxies() ) )
		{
			if( !isSceneChild( childPrim ) )
			{
				continue;
			}

			IECore::PathMatcher childSet = readSetInternal( childPrim, name, includeDescendantSets );
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
			: m_fileName( fileName ), m_openMode( openMode )
		{
			switch( m_openMode )
			{
				case IndexedIO::Read :
					m_stage = pxr::UsdStage::Open( fileName );
					if( !m_stage )
					{
						throw IECore::Exception( boost::str( boost::format( "USDScene : Failed to open USD file: '%1%'" ) % fileName ) );
					}
					break;
				case IndexedIO::Write :
					m_stage = pxr::UsdStage::CreateNew( fileName );
					break;
				default:
					throw Exception( "Unsupported OpenMode" );
			}

			initStage();
		}

		IO( const pxr::UsdStageRefPtr &stage, IndexedIO::OpenMode openMode )
			: m_fileName( "" ), m_openMode( openMode ), m_stage( stage )
		{
			initStage();
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

		void initStage()
		{
			m_timeCodesPerSecond = m_stage->GetTimeCodesPerSecond();
			m_rootPrim = m_stage->GetPseudoRoot();
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

	private :

		std::string m_fileName;
		IndexedIO::OpenMode m_openMode;
		pxr::UsdStageRefPtr m_stage;
		pxr::UsdPrim m_rootPrim;
		double m_timeCodesPerSecond;

		std::once_flag m_allTagsFlag;
		SceneInterface::NameList m_allTags;

};

USDScene::USDScene( const std::string &fileName, IndexedIO::OpenMode openMode )
	:	m_root( new IO( fileName, openMode ) ),
		m_location( new Location( m_root->root() ) )
{
}

USDScene::USDScene( const pxr::UsdStageRefPtr &stage )
	:	m_root( new IO( stage, IndexedIO::Read ) ),
		m_location( new Location( m_root->root() ) )
{
}

USDScene::USDScene( IOPtr io, LocationPtr location )
	:	m_root( io ), m_location( location )
{
}

USDScene::~USDScene()
{
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

ConstObjectPtr USDScene::readObject( double time ) const
{
	return ObjectAlgo::readObject( m_location->prim, m_root->getTime( time ) );
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
	else
	{
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
}

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{
	if( m_location->prim.IsPseudoRoot() )
	{
		// No attributes here
		return nullptr;
	}
	else if( name == SceneInterface::visibilityName )
	{
		auto attr = pxr::UsdGeomImageable( m_location->prim ).GetVisibilityAttr();
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
			writeAccessor->second = readSetInternal( m_root->root(), pxr::TfToken( name.string() ), /* includeDescendantSets = */ true );
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

PathMatcher USDScene::readSet( const Name &name, bool includeDescendantSets ) const
{
	return readSetInternal( m_location->prim, pxr::TfToken( name.string() ), includeDescendantSets );
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

	if( haveAttributes )
	{
		h.append( m_root->fileName() );
		appendPrimOrMasterPath( m_location->prim, h );
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

namespace
{

SceneInterface::FileFormatDescription<USDScene> g_descriptionUSD( ".usd", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDA( ".usda", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDC( ".usdc", IndexedIO::Read | IndexedIO::Write );

} // namespace
