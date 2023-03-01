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

#include "IECoreNuke/Convert.h"
#include "IECoreNuke/MeshFromNuke.h"

#include "IECore/Exception.h"
#include "IECore/NullObject.h"
#include "IECore/TransformationMatrixData.h"

#include "OpenEXR/OpenEXRConfig.h"
#if OPENEXR_VERSION_MAJOR < 3
#include "OpenEXR/ImathBoxAlgo.h"
#else
#include "Imath/ImathBoxAlgo.h"
#endif

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"

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

}

const std::string& LiveScene::nameAttribute( "ieName" );

IE_CORE_DEFINERUNTIMETYPED( LiveScene );

LiveScene::LiveScene() : m_op( nullptr )
{
}

LiveScene::LiveScene( GeoOp *op, const std::string rootPath ) : m_op( op ), m_rootPath( rootPath )
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
		auto info = geometryList().object( index );
		std::string nameValue;
		if( auto nameAttrib = info.get_group_attribute( GroupType::Group_Object, nameAttribute.data() ) )
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

GeometryList LiveScene::geometryList( const double* time ) const
{
	auto oc = OutputContext();
	if ( time )
	{
		oc.setFrame( timeToFrame( *time ) );
	}
	else
	{
		oc.setFrame( m_op->outputContext().frame() );
	}

	auto nodeInputOp = m_op->node_input( 0, Op::EXECUTABLE_INPUT, &oc );
	auto geoOp = dynamic_cast<DD::Image::GeoOp*>( nodeInputOp );

	geoOp->validate(true);
	boost::shared_ptr<DD::Image::Scene> scene( new DD::Image::Scene() );
	geoOp->build_scene( *scene );

	return *scene->object_list();
}

std::string LiveScene::fileName() const
{
	throw Exception( "IECoreNuke::LiveScene does not support fileName()." );
}

SceneInterface::Name LiveScene::name() const
{
	IECoreScene::SceneInterface::Path path;
	IECoreScene::SceneInterface::stringToPath( m_rootPath, path );
	if ( path.empty() )
	{
		return IECoreScene::SceneInterface::rootName;
	}

	return *path.rbegin();
}

void LiveScene::path( Path &p ) const
{
	p.clear();
	IECoreScene::SceneInterface::stringToPath( m_rootPath, p );
}

Imath::Box3d LiveScene::readBound( double time ) const
{
	Imath::Box3d bound;
	IECoreScene::SceneInterface::Path rootPath, currentPath;
	for( unsigned i=0; i < geometryList( &time ).objects(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( ( result != IECore::PathMatcher::AncestorMatch ) && ( result != IECore::PathMatcher::ExactMatch ) )
		{
			continue;
		}
		IECoreScene::SceneInterface::stringToPath( m_rootPath, rootPath );
		IECoreScene::SceneInterface::stringToPath( nameValue, currentPath );

		GeoInfo info = geometryList( &time ).object( i );
		Box3 objectBound;
		if ( ( currentPath.size() > 1 ) && ( ( currentPath.size() == rootPath.size() + 1 ) || ( nameValue == m_rootPath ) ) )
		{
			// object space bound
			objectBound = info.bbox();
		}
		else
		{
			objectBound = info.getTransformedBBox();
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
	
	for( unsigned i=0; i < geometryList().objects(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( result == IECore::PathMatcher::ExactMatch )
		{
			auto geoInfo = geometryList( &time ).object( i );
			auto from = geoInfo.matrix;
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
	for( unsigned i=0; i < geometryList().objects(); ++i )
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
	for( unsigned i=0; i < geometryList().objects(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( result == IECore::PathMatcher::ExactMatch )
		{
			auto geoInfo = geometryList( &time ).object( i );
			MeshFromNukePtr converter = new IECoreNuke::MeshFromNuke( &geoInfo );
			return converter->convert();
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

	for( unsigned i=0; i < geometryList().objects(); ++i )
	{
		auto nameValue = geoInfoPath( i );
		auto result = m_pathMatcher.match( nameValue );
		if ( ( result == IECore::PathMatcher::AncestorMatch ) || ( result == IECore::PathMatcher::ExactMatch ) )
		{
			allPaths.push_back( nameValue );
		}
	}

	// filter only children
	IECoreScene::SceneInterface::Path allPath, rootPath;
	IECoreScene::SceneInterface::stringToPath( m_rootPath, rootPath );
	for ( auto& path : allPaths )
	{
		allPath.clear();
		IECoreScene::SceneInterface::stringToPath( path, allPath );
		if ( rootPath.size() < allPath.size() )
		{
			childNames.push_back( allPath[rootPath.size()] );
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
	return new LiveScene( m_op, m_rootPath + "/" + name.string() );
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
	return new LiveScene( m_op, m_rootPath + "/" + name.string() );
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

	std::string pathStr;
	IECoreScene::SceneInterface::pathToString( path, pathStr );
	return new LiveScene( m_op, pathStr );
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

	std::string pathStr;
	IECoreScene::SceneInterface::pathToString( path, pathStr );
	return new LiveScene( m_op, pathStr );
}

void LiveScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	Path p;
	path( p );
	h.append( &( p[0] ), p.size() );
}
