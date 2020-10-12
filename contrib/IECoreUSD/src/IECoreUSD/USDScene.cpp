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
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
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

void convertPath( SceneInterface::Path& dst, const pxr::SdfPath& src)
{
	SceneInterface::stringToPath(src.GetString(), dst);
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

SceneInterface::Name convertAttributeName(const pxr::TfToken& attributeName)
{
	return SceneInterface::Name ( boost::algorithm::replace_first_copy( attributeName.GetString(), "cortex:", "" ) );
}

pxr::TfToken convertAttributeName(const SceneInterface::Name &attributeName)
{
	return pxr::TfToken( std::string("cortex:") +  attributeName.string() );
}

bool isAttributeName( const pxr::TfToken& attributeName )
{
	return boost::algorithm::starts_with( attributeName.GetString(), "cortex:" );
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

} // namespace

class USDScene::Location : public RefCounted
{
	public:
		Location(pxr::UsdPrim prim ) : prim(prim) {}
		pxr::UsdPrim prim;
};

class USDScene::IO : public RefCounted
{
	public:
		IO( const std::string &fileName ) : m_fileName( fileName )
		{
		}

		virtual ~IO()
		{
		}

		const std::string &fileName() const
		{
			return m_fileName;
		}

		virtual pxr::UsdPrim root() const = 0;
		virtual pxr::UsdTimeCode getTime( double timeSeconds ) const = 0;

		virtual bool isReader() const = 0;

		pxr::UsdStageRefPtr getStage() const { return m_usdStage; }
	protected:
		pxr::UsdStageRefPtr m_usdStage;

	private:
		std::string m_fileName;
};

class USDScene::Reader : public USDScene::IO
{
	public:
		Reader( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::Open( fileName );

			if ( !m_usdStage )
			{
				throw IECore::Exception( boost::str( boost::format( "USDScene::Reader() Failed to open usd file: '%1%'" ) % fileName ) );
			}

			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return true; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

class USDScene::Writer : public USDScene::IO
{
	public:
		Writer( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::CreateNew( fileName );
			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		~Writer() override
		{
			m_usdStage->GetRootLayer()->Save();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return false; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

USDScene::USDScene( const std::string &path, IndexedIO::OpenMode &mode )
{
	switch( mode )
	{
		case IndexedIO::Read :
			m_root = new Reader( path );
			m_location = new Location( m_root->root() );
			break;
		case IndexedIO::Write :
			m_root = new Writer( path );
			m_location = new Location( m_root->root() );
			break;
		default:
			throw Exception( " Unsupported OpenMode " );
	}
}

USDScene::USDScene( IOPtr root, LocationPtr location) : m_root( root ), m_location( location )
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

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{
	if( name == SceneInterface::visibilityName )
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

	pxr::UsdAttribute attribute = m_location->prim.GetAttribute( convertAttributeName( name ) );

	if ( !attribute )
	{
		return nullptr;
	}

	pxr::VtValue value;
	if ( !attribute.Get(&value, m_root->getTime( time ) ) )
	{
		return nullptr;
	}

	DataPtr data = DataAlgo::fromUSD( value, attribute.GetTypeName() );
	if( !data )
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format( "Unknown type %1% on attribute %2%") % attribute.GetTypeName() % name.string());
	}

	return data;
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
	std::vector<std::string> parts;
	pxr::SdfPath path = m_location->prim.GetPath();
	boost::split( parts, path.GetString(), boost::is_any_of( "/" ) );

	p.reserve( parts.size() );

	for( const auto &part : parts )
	{
		if( part != "" )
		{
			p.push_back( IECore::InternedString( part ) );
		}
	}
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

bool USDScene::hasAttribute( const SceneInterface::Name &name ) const
{
	if( name == SceneInterface::visibilityName )
	{
		return pxr::UsdGeomImageable( m_location->prim ).GetVisibilityAttr().HasAuthoredValue();
	}
	else
	{
		return m_location->prim.HasAttribute( convertAttributeName( name ) );
	}
}

void USDScene::attributeNames( SceneInterface::NameList &attrs ) const
{
	std::vector<pxr::UsdAttribute> attributes = m_location->prim.GetAttributes();

	attrs.clear();
	attrs.reserve( attributes.size() );

	auto visibilityAttr = pxr::UsdGeomImageable( m_location->prim ).GetVisibilityAttr();
	if( visibilityAttr.HasAuthoredValue() )
	{
		attrs.push_back( SceneInterface::visibilityName );
	}

	for( const auto &attr : attributes )
	{
		if ( isAttributeName( attr.GetName() ) )
		{
			attrs.push_back( convertAttributeName ( attr.GetName() ) );
		}
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
	else if( auto *data = IECore::runTimeCast<const IECore::Data>( attribute ) )
	{
		const pxr::UsdTimeCode timeCode = m_root->getTime( time );
		pxr::UsdAttribute attribute = m_location->prim.CreateAttribute( convertAttributeName( name ), DataAlgo::valueTypeName( data ), true );
		attribute.Set( DataAlgo::toUSD( data ), timeCode );
	}
}

bool USDScene::hasTag( const SceneInterface::Name &name, int filter ) const
{
	pxr::UsdPrim tagsPrim = m_root->root().GetChild( g_tagsPrimName );
	if( !tagsPrim )
	{
		return false;
	}

	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI( tagsPrim, pxr::TfToken( name.string() ) );
	if (!collection)
	{
		return false;
	}

	pxr::SdfPath p = m_location->prim.GetPath();

	pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
	pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths(membershipQuery, m_root->getStage());

	/// TODO. This will need to be updated once the Gaffer path matcher functionality has been moved into cortex
	for ( const auto &path : includedPaths )
	{
		if (path == p && filter & SceneInterface::LocalTag)
		{
			return true;
		}

		if (filter & SceneInterface::DescendantTag && boost::algorithm::starts_with( path.GetString(), p.GetString() ) && path != p )
		{
			return true;
		}

		if (filter & SceneInterface::AncestorTag && boost::algorithm::starts_with( p.GetString(), path.GetString() ) && path != p)
		{
			return true;
		}
	}

	return false;
}

void USDScene::readTags( SceneInterface::NameList &tags, int filter ) const
{
	tags.clear();

	pxr::UsdPrim tagsPrim = m_root->root().GetChild( g_tagsPrimName );
	if( !tagsPrim )
	{
		return;
	}

	std::vector<pxr::UsdCollectionAPI> collectionAPIs = pxr::UsdCollectionAPI::GetAllCollections( tagsPrim );
	pxr::SdfPath currentPath = m_location->prim.GetPath();

	pxr::SdfPath p = m_location->prim.GetPath();

	/// TODO. This will need to be updated once the Gaffer path matcher functionality has been moved into cortex
	std::set<SceneInterface::Name> tagsSet;
	for ( const auto &collection : collectionAPIs)
	{
		pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
		pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths(membershipQuery, m_root->getStage());

		for ( const auto &path : includedPaths )
		{
			bool match = false;
			if (path == p && filter & SceneInterface::LocalTag)
			{
				match = true;
			}

			if (filter & SceneInterface::DescendantTag && boost::algorithm::starts_with( path.GetString(), p.GetString() ) && path != p )
			{
				match = true;
			}

			if (filter & SceneInterface::AncestorTag && boost::algorithm::starts_with( p.GetString(), path.GetString() ) && path != p )
			{
				match = true;
			}

			if ( match )
			{
				tagsSet.insert( collection.GetName().GetString() );
			}
		}
	}

	for (const auto& i : tagsSet)
	{
		tags.push_back( i );
	}

}

void USDScene::writeTags( const SceneInterface::NameList &tags )
{
	pxr::UsdPrim tagsPrim = m_root->getStage()->DefinePrim( pxr::SdfPath( "/" + g_tagsPrimName.GetString() ) );
	for( const auto &tag : tags )
	{
		pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( tagsPrim, pxr::TfToken( tag.string() ), pxr::UsdTokens->explicitOnly );
		collection.CreateIncludesRel().AddTarget( m_location->prim.GetPath() );
	}
}

SceneInterface::NameList USDScene::setNames( bool includeDescendantSets ) const
{
	std::vector<pxr::UsdCollectionAPI> allCollections = pxr::UsdCollectionAPI::GetAllCollections( m_location->prim );
	NameList setNames;

	setNames.reserve( allCollections.size() );
	for( const pxr::UsdCollectionAPI &collection : allCollections )
	{
		setNames.push_back( collection.GetName().GetString() );
	}

	if ( includeDescendantSets )
	{
		NameList children;
		childNames( children );
		for( const SceneInterface::Name &childName : children )
		{
			NameList childSetNames = child( childName, ThrowIfMissing )->setNames( includeDescendantSets );
			setNames.insert( setNames.begin(), childSetNames.begin(), childSetNames.end() );
		}
	}

	// ensure our set names are unique
	std::sort( setNames.begin(), setNames.end() );
	return NameList( setNames.begin(), std::unique( setNames.begin(), setNames.end() ) );
}

PathMatcher USDScene::readSet( const Name &name, bool includeDescendantSets ) const
{
	SceneInterface::Path prefix;
	PathMatcher pathMatcher;
	recurseReadSet( prefix, name, pathMatcher, includeDescendantSets );

	return pathMatcher;
}

void USDScene::recurseReadSet( const SceneInterface::Path &prefix, const Name &name, IECore::PathMatcher &pathMatcher, bool includeDescendantSets ) const
{
	if( PathMatcherDataPtr pathMatcherData = readLocalSet( name ) )
	{
		pathMatcher.addPaths( pathMatcherData->readable(), prefix );
	}

	if ( !includeDescendantSets )
	{
		return;
	}

	NameList children;
	childNames( children );

	SceneInterface::Path childPrefix = prefix;
	childPrefix.resize( prefix.size() + 1 );

	for( InternedString &childName : children )
	{
		*childPrefix.rbegin() = childName;
		runTimeCast<const USDScene>( child( childName, SceneInterface::ThrowIfMissing ) )->recurseReadSet( childPrefix, name, pathMatcher, includeDescendantSets );
	}
}

IECore::PathMatcherDataPtr USDScene::readLocalSet( const Name &name ) const
{
	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI( m_location->prim, pxr::TfToken( name.string() ) );

	if( !collection )
	{
		return new IECore::PathMatcherData();
	}

	pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
	pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths( membershipQuery, m_root->getStage() );

	PathMatcherDataPtr pathMatcherData = new PathMatcherData();
	PathMatcher &pathMatcher = pathMatcherData->writable();

	for( pxr::SdfPath path : includedPaths )
	{
		path = path.ReplacePrefix( m_location->prim.GetPath(), pxr::SdfPath( "/" ) );

		SceneInterface::Path cortexPath;
		convertPath( cortexPath, path );

		pathMatcher.addPath( cortexPath );
	}

	return pathMatcherData;
}

void USDScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( m_location->prim, pxr::TfToken( name.string() ), pxr::UsdTokens->explicitOnly );

	pxr::SdfPathVector targets;
	for( PathMatcher::Iterator it = set.begin(); it != set.end(); ++it )
	{
		const SceneInterface::Path &path = *it;

		if ( path.empty() )
		{
			IECore::msg(
				IECore::MessageHandler::Error,
				"USDScene::writeSet",
				boost::str( boost::format( "Unable to add path '%2%' to  set: '%1%' at location: '%2%' " ) % name.string() % m_location->prim.GetPath().GetString() )
			);
			continue;
		}

		targets.push_back( toUSD( path, /* relative = */ true ) );
	}

	collection.CreateIncludesRel().SetTargets( targets );
}

void USDScene::hashSet( const Name &name, IECore::MurmurHash &h ) const
{
	SceneInterface::hashSet( name, h );

	SceneInterface::Path path;
	convertPath( path, m_location->prim.GetPath() );

	h.append( m_root->fileName() );
	h.append( &path[0], path.size() );
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
			if( m_root->isReader() )
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

	auto visibilityAttr = pxr::UsdGeomImageable( m_location->prim ).GetVisibilityAttr();
	if( visibilityAttr.HasAuthoredValue() )
	{
		haveAttributes = true;
		mightBeTimeVarying = visibilityAttr.ValueMightBeTimeVarying();
	}

	if( !mightBeTimeVarying )
	{
		std::vector<pxr::UsdAttribute> attributes = m_location->prim.GetAttributes();
		for( const auto &attr : attributes )
		{
			if( isAttributeName( attr.GetName() ) )
			{
				haveAttributes = true;
				mightBeTimeVarying = mightBeTimeVarying || attr.ValueMightBeTimeVarying();
			}
			if( mightBeTimeVarying )
			{
				break;
			}
		}
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
