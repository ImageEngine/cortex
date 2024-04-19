//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/LiveScene.h"

#include "DDImage/Scene.h"
#include "DDImage/Execute.h"
#include "DDImage/ParticleOp.h"

#include "IECoreNuke/Convert.h"
#include "IECoreNuke/MeshFromNuke.h"
#include "IECoreNuke/FromNukePointsConverter.h"

#include "IECore/Exception.h"
#include "IECore/NullObject.h"
#include "IECore/TransformationMatrixData.h"

#include "Imath/ImathBoxAlgo.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"

#include "tbb/recursive_mutex.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreNuke;
using namespace DD::Image;

namespace
{

IECore::TransformationMatrixd convertTransformMatrix( DD::Image::Matrix4& from )
{
	auto to = IECore::TransformationMatrixd();
	DD::Image::Vector3 rotation, translation, scale, shear;
	from.decompose( rotation, translation, scale, shear, DD::Image::Matrix4::RotationOrder::eXYZ );
	to.scale = IECore::convert<Imath::V3f>( scale );
	to.shear = IECore::convert<Imath::V3f>( shear );
	to.rotate = IECore::convert<Imath::V3f>( rotation );
	to.translate = IECore::convert<Imath::V3f>( translation );
	return to;
}

tbb::recursive_mutex g_mutex;

LiveScene::LiveSceneGeometryCache &cachedGeometryListMap()
{
	static LiveScene::LiveSceneGeometryCache *cache = new LiveScene::LiveSceneGeometryCache();
	return *cache;
}

}

const std::string& LiveScene::nameAttribute( "ieName" );

IE_CORE_DEFINERUNTIMETYPED( LiveScene );

LiveScene::LiveScene() : m_op( nullptr )
{
}

LiveScene::LiveScene( GeoOp *op, const IECoreScene::SceneInterface::Path& rootPath ) : m_op( op ), m_rootPath( rootPath )
{
	m_pathMatcher = IECore::PathMatcher();
	m_pathMatcher.addPath( m_rootPath );
}

LiveScene::~LiveScene()
{
}

void LiveScene::setOp( DD::Image::GeoOp* op )
{
	m_op = op;
	m_objectPathMap.clear();
}

const GeoOp *LiveScene::getOp() const
{
	return m_op;
}

double LiveScene::timeToFrame( const double& time )
{
	return time * DD::Image::root_real_fps();
}

double LiveScene::frameToTime( const int& frame )
{
	return frame / double( DD::Image::root_real_fps() );
}

std::string LiveScene::geoInfoPath( const int& index ) const
{
	auto it = m_objectPathMap.find( index );
	if ( it != m_objectPathMap.end() )
	{
		return it->second;
	}
	else
	{
		auto info = object( index );
		if ( !info )
		{
			return "/undefined" + std::to_string( index );
		}
		std::string nameValue;
		if( auto nameAttrib = info->get_group_attribute( GroupType::Group_Object, nameAttribute.data() ) )
		{
			nameValue = nameAttrib->stdstring();
		}
		else
		{
			nameValue =  "/object" + std::to_string( index );
		}

		m_objectPathMap[index] = nameValue;

		return nameValue;
	}
}

void LiveScene::cacheGeometryList( const double& frame ) const
{
	DD::Image::Hash h;
	if( auto parent = m_op->parent() )
	{
		h = static_cast<Op*>( parent )->hash();
	}
	else
	{
		h = static_cast<Op*>( m_op )->hash();
	}
	auto it = cachedGeometryListMap().find( this );
	if ( it == cachedGeometryListMap().end() )
	{
		auto geomList = geometryList( frame );
		cachedGeometryListMap()[this][h][frame] = geomList;
	}
	else
	{
		auto jit = cachedGeometryListMap()[this].find( h );
		if ( jit == cachedGeometryListMap()[this].end() )
		{
			auto geomList = geometryList( frame );
			cachedGeometryListMap()[this][h][frame] = geomList;
		}
		else
		{
			auto kit = cachedGeometryListMap()[this][h].find( frame );
			if ( kit == cachedGeometryListMap()[this][h].end() )
			{
				auto geomList = geometryList( frame );
				cachedGeometryListMap()[this][h][frame] = geomList;
			}
		}
	}
}

unsigned LiveScene::objectNum( const double* time) const
{
	double frame;
	if ( time )
	{
		frame = timeToFrame( *time );
	}
	else
	{
		frame = m_op->outputContext().frame();
	}
	cacheGeometryList( frame );

	DD::Image::Hash h;
	if( auto parent = m_op->parent() )
	{
		h = static_cast<Op*>( parent )->hash();
	}
	else
	{
		h = static_cast<Op*>( m_op )->hash();
	}

	auto cit = cachedGeometryListMap().find( this );
	if ( cit != cachedGeometryListMap().end() )
	{
		auto jit = cachedGeometryListMap()[this].find( h );
		if ( jit != cachedGeometryListMap()[this].end() )
		{

			auto kit = cachedGeometryListMap()[this][h].find( frame );
			if ( kit != cachedGeometryListMap()[this][h].end() )
			{
				return cachedGeometryListMap()[this][h][frame].objects();
			}
		}
	}

	return 0;
}

DD::Image::GeoInfo* LiveScene::object( const unsigned& index, const double* time ) const
{
	double frame;
	if ( time )
	{
		frame = timeToFrame( *time );
	}
	else
	{
		frame = m_op->outputContext().frame();
	}
	cacheGeometryList( frame );

	DD::Image::Hash h;
	if( auto parent = m_op->parent() )
	{
		h = static_cast<Op*>( parent )->hash();
	}
	else
	{
		h = static_cast<Op*>( m_op )->hash();
	}

	auto cit = cachedGeometryListMap().find( this );
	if ( cit != cachedGeometryListMap().end() )
	{
		auto jit = cachedGeometryListMap()[this].find( h );
		if ( jit != cachedGeometryListMap()[this].end() )
		{

			auto kit = cachedGeometryListMap()[this][h].find( frame );
			if ( kit != cachedGeometryListMap()[this][h].end() )
			{
				return &kit->second.object( index );
			}
		}
	}
	return nullptr;
}

DD::Image::GeometryList LiveScene::geometryList( DD::Image::Op* op, const double& frame ) const
{
	boost::shared_ptr<DD::Image::Scene> scene( new DD::Image::Scene() );
	boost::shared_ptr<DD::Image::GeometryList> geo( new DD::Image::GeometryList() );

	auto executioner = Execute();
	auto executableOp = executioner.generateOp( op, 0, frame );
	if ( !executableOp )
	{
		return *geo;
	}

	DD::Image::GeoOp* geoOp = executableOp->geoOp();
	if ( !geoOp )
	{
		return *geo;
	}

	geoOp->validate(true);

	geoOp->get_geometry( *scene, *geo );

	return *geo;
}

DD::Image::GeometryList LiveScene::geometryList( const double& frame ) const
{
	// Nuke Geometry API is not thread safe so we need to use a mutex here to avoid crashes.
	tbb::recursive_mutex::scoped_lock l( g_mutex );
	DD::Image::GeometryList result;

	if( m_op->input0() && m_op->input0()->particleOp() )
	{
		auto particleToGeo = Op::create( "ParticleToGeo", m_op );
		particleToGeo->set_input( 0, m_op->input0() );
		result = geometryList( particleToGeo, frame );
	}
	else
	{
		result = geometryList( m_op, frame );
	}

	return result;
}

std::string LiveScene::fileName() const
{
	throw Exception( "IECoreNuke::LiveScene does not support fileName()." );
}

SceneInterface::Name LiveScene::name() const
{
	if ( m_rootPath.empty() )
	{
		return IECoreScene::SceneInterface::rootName;
	}

	return *m_rootPath.rbegin();
}

void LiveScene::path( Path &p ) const
{
	p.clear();
	p = m_rootPath;
}

Imath::Box3d LiveScene::readBound( double time ) const
{
	Imath::Box3d bound;
	bound.makeEmpty();
	std::string rootPathStr;
	IECoreScene::SceneInterface::Path currentPath;
	for( unsigned i=0; i < objectNum( &time ); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( ( result != IECore::PathMatcher::AncestorMatch ) && ( result != IECore::PathMatcher::ExactMatch ) )
		{
			continue;
		}
		IECoreScene::SceneInterface::pathToString( m_rootPath, rootPathStr );
		IECoreScene::SceneInterface::stringToPath( nameValue, currentPath );

		auto info = object( i, &time );
		if ( !info )
		{
			return bound;
		}

		Box3 objectBound;
		if ( ( currentPath.size() > 1 ) && ( ( currentPath.size() == m_rootPath.size() + 1 ) || ( nameValue == rootPathStr ) ) )
		{
			// object space bound
			objectBound = info->bbox();
		}
		else
		{
			objectBound = info->getTransformedBBox();
		}
		Imath::Box3d b = IECore::convert<Imath::Box3d, Box3>( objectBound );

		if( b.hasVolume() )
		{
			bound.extendBy( b );
		}
	}

	return bound;
}

void LiveScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw Exception( "IECoreNuke::LiveScene::writeBound: write operations not supported!" );
}

ConstDataPtr LiveScene::readTransform( double time ) const
{
	for( unsigned i=0; i < objectNum( &time ); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( result == IECore::PathMatcher::ExactMatch )
		{
			auto geoInfo = object( i, &time );
			if( !geoInfo )
			{
				return new TransformationMatrixdData( IECore::TransformationMatrixd() );
			}
			auto from = geoInfo->matrix;
			return new TransformationMatrixdData( convertTransformMatrix( from ) );
		}
	}

	return new TransformationMatrixdData( IECore::TransformationMatrixd() );
}

Imath::M44d LiveScene::readTransformAsMatrix( double time ) const
{
	return runTimeCast< const TransformationMatrixdData >( readTransform( time ) )->readable().transform();
}

void LiveScene::writeTransform( const Data *transform, double time )
{
	throw Exception( "IECoreNuke::LiveScene::writeTransform: write operations not supported!" );
}

bool LiveScene::hasAttribute( const Name &name ) const
{
	return false;
}

void LiveScene::attributeNames( NameList &attrs ) const
{
}

ConstObjectPtr LiveScene::readAttribute( const Name &name, double time ) const
{
	return IECore::NullObject::defaultNullObject();
}

void LiveScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreNuke::LiveScene::writeAttribute: write operations not supported!" );
}

bool LiveScene::hasTag( const Name &name, int filter ) const
{
	return false;
}

void LiveScene::readTags( NameList &tags, int filter ) const
{
}

void LiveScene::writeTags( const NameList &tags )
{
	throw Exception( "IECoreNuke::LiveScene::writeTags not supported" );
}

SceneInterface::NameList LiveScene::setNames( bool includeDescendantSets ) const
{
	return SceneInterface::NameList();
}

IECore::PathMatcher LiveScene::readSet( const Name &name, bool includeDescendantSets, const IECore::Canceller *canceller ) const
{
	return IECore::PathMatcher();
}

void LiveScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	throw Exception( "IECoreNuke::LiveScene::writeSet not supported" );
}

void LiveScene::hashSet( const Name& setName, IECore::MurmurHash &h ) const
{
}

bool LiveScene::hasObject() const
{
	for( unsigned i=0; i < objectNum(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( result == IECore::PathMatcher::ExactMatch )
		{
			return true;
		}
	}

	return false;
}

ConstObjectPtr LiveScene::readObject( double time, const IECore::Canceller *canceller) const
{
	for( unsigned i=0; i < objectNum(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( result == IECore::PathMatcher::ExactMatch )
		{
			auto geoInfo = object( i, &time );
			if ( !geoInfo )
			{
				return IECore::NullObject::defaultNullObject();
			}
			if ( geoInfo->primitives() == 1 && ( geoInfo->primitive( 0 )->getPrimitiveType() == DD::Image::PrimitiveType::eParticlesSprite ) )
			{
				auto converter = new IECoreNuke::FromNukePointsConverter( geoInfo, m_op->input0() );
				return converter->convert();
			}
			else
			{
				MeshFromNukePtr converter = new IECoreNuke::MeshFromNuke( geoInfo );
				return converter->convert();
			}
		}
	}

	return IECore::NullObject::defaultNullObject();
}

PrimitiveVariableMap LiveScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	throw Exception( "IECoreNuke::readObjectPrimitiveVariables() not implemented!" );
}

void LiveScene::writeObject( const Object *object, double time )
{
	throw Exception( "IECoreNuke::LiveScene::writeObject: write operations not supported!" );
}

void LiveScene::childNames( NameList &childNames ) const
{
	childNames.clear();
	std::vector<std::string> allPaths;

	for( unsigned i=0; i < objectNum(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( ( result == IECore::PathMatcher::AncestorMatch ) || ( result == IECore::PathMatcher::ExactMatch ) )
		{
			allPaths.push_back( nameValue );
		}
	}

	// filter only children
	IECoreScene::SceneInterface::Path allPath;
	std::string rootPathStr;
	IECoreScene::SceneInterface::pathToString( m_rootPath, rootPathStr );
	for ( auto& path : allPaths )
	{
		// ignore children with a different root path
		if ( !( path.rfind( rootPathStr, 0 ) == 0 ) )
		{
			continue;
		}

		allPath.clear();
		IECoreScene::SceneInterface::stringToPath( path, allPath );
		if ( m_rootPath.size() < allPath.size() )
		{
			// ignore duplicates.
			if ( find( childNames.begin(), childNames.end(), allPath[m_rootPath.size()] ) != childNames.end() )
			{
				continue;
			}
			childNames.push_back( allPath[m_rootPath.size()] );
		}
	}
}

bool LiveScene::hasChild( const Name &name ) const
{
	IECoreScene::SceneInterface::NameList names;
	childNames( names );

	return find( names.cbegin(), names.cend(), name ) != names.cend();
}

SceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	IECoreScene::SceneInterface::NameList names;
	childNames( names );

	if( find( names.cbegin(), names.cend(), name ) == names.cend() )
	{
		switch ( missingBehaviour )
		{
			case MissingBehaviour::ThrowIfMissing:
				throw Exception( "IECoreNuke::LiveScene: Name \"" + name.string() + "\" is not a valid childName." );
			case MissingBehaviour::NullIfMissing:
				return nullptr;
			case MissingBehaviour::CreateIfMissing:
				throw Exception( "IECoreNuke::LiveScene: Name\"" + name.string() + "\" is missing and LiveScene is read-only" );
		}
	}

	IECoreScene::SceneInterface::Path newPath = m_rootPath;
	newPath.push_back( name.string() );

	return new LiveScene( m_op, newPath );
}

ConstSceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	IECoreScene::SceneInterface::NameList names;
	childNames( names );

	if( find( names.cbegin(), names.cend(), name ) == names.cend() )
	{
		switch ( missingBehaviour )
		{
			case MissingBehaviour::ThrowIfMissing:
				throw Exception( "IECoreNuke::LiveScene: Name \"" + name.string() + "\" is not a valid childName." );
			case MissingBehaviour::NullIfMissing:
				return nullptr;
			case MissingBehaviour::CreateIfMissing:
				throw Exception( "IECoreNuke::LiveScene: Name\"" + name.string() + "\" is missing and LiveScene is read-only" );
		}
	}

	IECoreScene::SceneInterface::Path newPath = m_rootPath;
	newPath.push_back( name.string() );

	return new LiveScene( m_op, newPath );
}

SceneInterfacePtr LiveScene::createChild( const Name &name )
{
	throw Exception( "IECoreNuke::LiveScene is read-only" );
}

ConstSceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	IECoreNuke::ConstLiveScenePtr currentScene( this );
	for ( const auto& child : path )
	{
		if ( auto childScene = currentScene->child( child, missingBehaviour ) )
		{
			currentScene = dynamic_cast<const IECoreNuke::LiveScene*>( childScene.get() );
		}
		else
		{
			switch ( missingBehaviour )
			{
				case MissingBehaviour::ThrowIfMissing:
					throw Exception( "IECoreNuke::LiveScene: Name \"" + child.string() + "\" is not a valid childName." );
				case MissingBehaviour::NullIfMissing:
					return nullptr;
				case MissingBehaviour::CreateIfMissing:
					throw Exception( "IECoreNuke::LiveScene: Name\"" + child.string() + "\" is missing and LiveScene is read-only" );
			}
		}
	}

	return new LiveScene( m_op, path );
}

SceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	IECoreNuke::LiveScenePtr currentScene( this );
	for ( const auto& child : path )
	{
		if ( auto childScene = currentScene->child( child, missingBehaviour ) )
		{
			currentScene = dynamic_cast<IECoreNuke::LiveScene*>( childScene.get() );
		}
		else
		{
			switch ( missingBehaviour )
			{
				case MissingBehaviour::ThrowIfMissing:
					throw Exception( "IECoreNuke::LiveScene: Name \"" + child.string() + "\" is not a valid childName." );
				case MissingBehaviour::NullIfMissing:
					return nullptr;
				case MissingBehaviour::CreateIfMissing:
					throw Exception( "IECoreNuke::LiveScene: Name\"" + child.string() + "\" is missing and LiveScene is read-only" );
			}
		}
	}

	return new LiveScene( m_op, path );
}

void LiveScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	Path p;
	path( p );
	h.append( &( p[0] ), p.size() );
}
