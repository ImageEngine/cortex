//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Exception.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Camera.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/Primitive.h"
#include "IECore/NullObject.h"

#include "IECoreMaya/LiveScene.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/FromMayaTransformConverter.h"
#include "IECoreMaya/FromMayaShapeConverter.h"
#include "IECoreMaya/FromMayaDagNodeConverter.h"
#include "IECoreMaya/FromMayaCameraConverter.h"
#include "IECoreMaya/Convert.h"
#include "IECoreMaya/SceneShape.h"

#include "maya/MFnDagNode.h"
#include "maya/MFnTransform.h"
#include "maya/MPxTransform.h"
#include "maya/MPxSurfaceShape.h"
#include "maya/MFnCamera.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnNumericData.h"
#include "maya/MFnAttribute.h"
#include "maya/MSelectionList.h"
#include "maya/MAnimControl.h"
#include "maya/MString.h"
#include "maya/MTime.h"
#include "maya/MItDag.h"
#include "maya/MPlug.h"
#include "maya/MTransformationMatrix.h"
#include "maya/MDagPathArray.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace IECore;
using namespace IECoreMaya;

IE_CORE_DEFINERUNTIMETYPED( LiveScene );

// this stuff requires a mutex, as all them maya DG functions aint thread safe!
LiveScene::Mutex LiveScene::s_mutex;

LiveScene::LiveScene() : m_isRoot( true )
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	// initialize to the root path:
	MItDag it;
	it.getPath( m_dagPath );
}

LiveScene::LiveScene( const MDagPath& p, bool isRoot ) : m_isRoot( isRoot )
{
	// this constructor is expected to be called when s_mutex is locked!
	m_dagPath = p;
}

LiveScene::~LiveScene()
{
}

std::string LiveScene::fileName() const
{
	throw Exception( "IECoreMaya::LiveScene does not support fileName()." );
}

LiveScenePtr LiveScene::duplicate( const MDagPath& p, bool isRoot ) const
{
	return new LiveScene( p, isRoot );
}

SceneInterface::Name LiveScene::name() const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::name: Dag path no longer exists!" );
	}
	
	std::string nameStr = m_dagPath.fullPathName().asChar();
	
	if( nameStr.size() <= 1 )
	{
		return SceneInterface::rootName;
	}
	
	size_t pipePos = nameStr.rfind("|");
	if( pipePos != std::string::npos )
	{
		return std::string( nameStr.begin() + pipePos + 1, nameStr.end() );
	}
	else
	{
		return nameStr;
	}
}

void LiveScene::path( Path &p ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::path: Dag path no longer exists!" );
	}
	
	std::string pathStr( m_dagPath.fullPathName().asChar() );
	boost::tokenizer<boost::char_separator<char> > t( pathStr, boost::char_separator<char>( "|" ) );
	
	p.clear();
	
	for (
		boost::tokenizer<boost::char_separator<char> >::iterator it = t.begin();
		it != t.end();
		++it
	)
	{
		p.push_back( Name( *it ) );
	}
	
}

Imath::Box3d LiveScene::readBound( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "IECoreMaya::LiveScene::readBound: time must be the same as on the maya timeline!" );
	}
	
	if( m_isRoot )
	{
		MDagPathArray paths;
		getChildDags( m_dagPath, paths );
		
		Imath::Box3d bound;
		
		for( unsigned i=0; i < paths.length(); ++i )
		{
			MFnDagNode dagFn( paths[i] );
			Imath::Box3d b = IECore::convert<Imath::Box3d, MBoundingBox>( dagFn.boundingBox() );
			
			if( b.hasVolume() )
			{
				bound.extendBy( b );
			}
		}
		
		return bound;
	}
	else if( m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::readBound: Dag path no longer exists!" );
	}
	else
	{
		MFnDagNode dagFn( m_dagPath );
		Imath::Box3d ret = IECore::convert<Imath::Box3d, MBoundingBox>( dagFn.boundingBox() );
		Imath::M44d invTransform = IECore::convert<Imath::M44d, MMatrix>( dagFn.transformationMatrix() ).inverse();
		return Imath::transform( ret, invTransform );
	}
}

void LiveScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw Exception( "IECoreMaya::LiveScene::writeBound: write operations not supported!" );
}

ConstDataPtr LiveScene::readTransform( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::readTransform: Dag path no longer exists!" );
	}
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "IECoreMaya::LiveScene::readTransform: time must be the same as on the maya timeline!" );
	}
	
	if( m_dagPath.hasFn( MFn::kTransform ) )
	{
		MFnDagNode dagFn( m_dagPath );
		return new IECore::TransformationMatrixdData( IECore::convert<IECore::TransformationMatrixd, MTransformationMatrix>( dagFn.transformationMatrix() ) );
	}
	else
	{
		return new TransformationMatrixdData( IECore::TransformationMatrixd() );
	}
}

Imath::M44d LiveScene::readTransformAsMatrix( double time ) const
{
	return runTimeCast< const TransformationMatrixdData >( readTransform( time ) )->readable().transform();
}

void LiveScene::writeTransform( const Data *transform, double time )
{
	throw Exception( "IECoreMaya::LiveScene::writeTransform: write operations not supported!" );
}

bool LiveScene::hasAttribute( const Name &name ) const
{
	if ( !m_isRoot && m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::hasAttribute: Dag path no longer exists!" );
	}
	
	if( name == SceneInterface::visibilityName )
	{
		return true;
	}
	
	std::vector< CustomAttributeReader > &attributeReaders = customAttributeReaders();
	for ( std::vector< CustomAttributeReader >::const_iterator it = attributeReaders.begin(); it != attributeReaders.end(); ++it )
	{
		if ( it->m_mightHave )
		{
			if ( ! it->m_mightHave( m_dagPath, name ) )
			{
				continue;
			}
		}

		NameList names;
		{
			// call it->m_names under a mutex, as it could be reading plug values,
			// which isn't thread safe:
			tbb::mutex::scoped_lock l( s_mutex );
			it->m_names( m_dagPath, names );
		}
		
		if ( std::find(names.begin(), names.end(), name) != names.end() )
		{
			return true;
		}
	}
	return false;
}

void LiveScene::attributeNames( NameList &attrs ) const
{
	if( !m_isRoot && m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::attributeNames: Dag path no longer exists!" );
	}

	tbb::mutex::scoped_lock l( s_mutex );
	attrs.clear();
	attrs.push_back( SceneInterface::visibilityName );

	// translate attributes with names starting with "ieAttr_":
	MFnDependencyNode fnNode( m_dagPath.node() );
	unsigned int n = fnNode.attributeCount();
	for( unsigned int i=0; i<n; i++ )
	{
		MObject attr = fnNode.attribute( i );
		MFnAttribute fnAttr( attr );
		MString attrName = fnAttr.name();
		if( attrName.length() > 7 && ( strstr( attrName.asChar(),"ieAttr_" ) == attrName.asChar() ) )
		{
			attrs.push_back( ( "user:" + attrName.substring( 7, attrName.length()-1 ) ).asChar() );
		}
	}

	// add attributes from custom readers:
	for ( std::vector< CustomAttributeReader >::const_iterator it = customAttributeReaders().begin(); it != customAttributeReaders().end(); it++ )
	{
		it->m_names( m_dagPath, attrs );
	}

	// remove duplicates:
	std::sort( attrs.begin(), attrs.end() );
	attrs.erase( std::unique( attrs.begin(), attrs.end() ), attrs.end() );
}

ConstObjectPtr LiveScene::readAttribute( const Name &name, double time ) const
{
	if ( !m_isRoot && m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::readAttribute: Dag path no longer exists!" );
	}
	
	tbb::mutex::scoped_lock l( s_mutex );
	if ( !m_isRoot )
	{

		if( name == SceneInterface::visibilityName )
		{
			bool visible = true;

			MStatus st;
			MFnDagNode dagFn( m_dagPath );
			MPlug visibilityPlug = dagFn.findPlug( MPxTransform::visibility, &st );
			if( st )
			{
				visible = visibilityPlug.asBool();
			}

			if( visible )
			{
				MDagPath childDag;

				// find an object that's either a SceneShape, or has a cortex converter and check its visibility:
				unsigned int childCount = 0;
				m_dagPath.numberOfShapesDirectlyBelow(childCount);

				for ( unsigned int c = 0; c < childCount; c++ )
				{
					MDagPath d = m_dagPath;
					if( d.extendToShapeDirectlyBelow( c ) )
					{
						MFnDagNode fnChildDag(d);
						if( fnChildDag.typeId() == SceneShape::id )
						{
							childDag = d;
							break;
						}

						if ( fnChildDag.isIntermediateObject() )
						{
							continue;
						}

						FromMayaShapeConverterPtr shapeConverter = FromMayaShapeConverter::create( d );
						if( shapeConverter )
						{
							childDag = d;
							break;
						}

						FromMayaDagNodeConverterPtr dagConverter = FromMayaDagNodeConverter::create( d );
						if( dagConverter )
						{
							childDag = d;
							break;
						}
					}
				}

				if( childDag.isValid() )
				{
					MFnDagNode dagFn( childDag );
					MPlug visibilityPlug = dagFn.findPlug( MPxSurfaceShape::visibility, &st );
					if( st )
					{
						visible = visibilityPlug.asBool();
					}
				}

			}

			return new BoolData( visible );
		}

	}
	else if( name == SceneInterface::visibilityName )
	{
		return new BoolData( true );
	}

	std::vector< CustomAttributeReader > &attributeReaders = customAttributeReaders();
	for ( std::vector< CustomAttributeReader >::const_reverse_iterator it = attributeReaders.rbegin(); it != attributeReaders.rend(); ++it )
	{
		ConstObjectPtr attr = it->m_read( m_dagPath, name );
		if( !attr )
		{
			continue;
		}
		return attr;
	}

	if( strstr( name.c_str(), "user:" ) == name.c_str() )
	{

		MStatus st;
		MFnDependencyNode fnNode( m_dagPath.node() );
		MPlug attrPlug = fnNode.findPlug( ( "ieAttr_" + name.string().substr(5) ).c_str(), false, &st );
		if( st )
		{
			FromMayaConverterPtr plugConverter = FromMayaPlugConverter::create( attrPlug );
			if( !plugConverter )
			{
				return IECore::NullObject::defaultNullObject();
			}
			return plugConverter->convert();
		}
	}

	return IECore::NullObject::defaultNullObject();
}

void LiveScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreMaya::LiveScene::writeAttribute: write operations not supported!" );
}

bool LiveScene::hasTag( const Name &name, int filter ) const
{
	if ( m_isRoot )
	{
		return false;
	}

	if( m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::hasTag: Dag path no longer exists!" );
	}
	
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		if ( it->m_has( m_dagPath, name, filter ) )
		{
			return true;
		}
	}
	
	return false;
}

void LiveScene::readTags( NameList &tags, int filter ) const
{
	tags.clear();

	if ( m_isRoot )
	{
		return;
	}

	if( m_dagPath.length() == 0 )
	{
		throw Exception( "IECoreMaya::LiveScene::attributeNames: Dag path no longer exists!" );
	}
	
	std::set<Name> uniqueTags;

	// read tags from ieTags attribute:
	MStatus st;
	MFnDependencyNode fnNode( m_dagPath.node() );
	MPlug tagsPlug = fnNode.findPlug( "ieTags", false, &st );
	if( st )
	{
		std::string tagsStr( tagsPlug.asString().asChar() );
		boost::tokenizer<boost::char_separator<char> > t( tagsStr, boost::char_separator<char>( " " ) );
		for (
			boost::tokenizer<boost::char_separator<char> >::iterator it = t.begin();
			it != t.end();
			++it
		)
		{
			uniqueTags.insert( Name( *it ) );
		}
	}

	// read tags from custom readers:
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		NameList values;
		it->m_read( m_dagPath, values, filter );
		uniqueTags.insert( values.begin(), values.end() );
	}

	tags.insert( tags.end(), uniqueTags.begin(), uniqueTags.end() );
}

void LiveScene::writeTags( const NameList &tags )
{
	throw Exception( "IECoreMaya::LiveScene::writeTags not supported" );
}

bool LiveScene::hasObject() const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_isRoot )
	{
		return false;
	}
	else if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::hasObject: Dag path no longer exists!" );
	}

	for ( std::vector< CustomReader >::const_reverse_iterator it = customObjectReaders().rbegin(); it != customObjectReaders().rend(); it++ )
	{
		if ( it->m_has( m_dagPath ) )
		{
			return true;
		}
	}

	// if no custom object was detected, we try the general cortex converter
	unsigned int childCount = 0;
	m_dagPath.numberOfShapesDirectlyBelow(childCount);

	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MDagPath childDag = m_dagPath;
		if( childDag.extendToShapeDirectlyBelow( c ) )
		{
			MFnDagNode fnChildDag(childDag);
			if ( fnChildDag.isIntermediateObject() )
			{
				continue;
			}

			FromMayaShapeConverterPtr shapeConverter = FromMayaShapeConverter::create( childDag );
			if( shapeConverter )
			{
				return true;
			}
		
			FromMayaDagNodeConverterPtr dagConverter = FromMayaDagNodeConverter::create( childDag );
			if( dagConverter )
			{
				return true;
			}
		}
	}
	
	return false;
}

ConstObjectPtr LiveScene::readObject( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::readObject: Dag path no longer exists!" );
	}
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "IECoreMaya::LiveScene::readObject: time must be the same as on the maya timeline!" );
	}

	for ( std::vector< CustomReader >::const_reverse_iterator it = customObjectReaders().rbegin(); it != customObjectReaders().rend(); it++ )
	{
		if ( it->m_has( m_dagPath ) )
		{
			return it->m_read( m_dagPath );
		}
	}

	// if no custom object was detected, we try the general cortex converter
	unsigned int childCount = 0;
	m_dagPath.numberOfShapesDirectlyBelow(childCount);

	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MDagPath childDag = m_dagPath;
		if( childDag.extendToShapeDirectlyBelow( c ) )
		{
			MFnDagNode fnChildDag(childDag);
			if ( fnChildDag.isIntermediateObject() )
			{
				continue;
			}

			FromMayaShapeConverterPtr shapeConverter = FromMayaShapeConverter::create( childDag );
			if( shapeConverter )
			{
				return shapeConverter->convert();
			}
		
			FromMayaDagNodeConverterPtr dagConverter = FromMayaDagNodeConverter::create( childDag );
			if( dagConverter )
			{
				ObjectPtr result = dagConverter->convert();
				Camera *cam = runTimeCast< Camera >( result.get() );
				if( cam )
				{
					// Cameras still carry the transform when converted from maya,
					// so we have to remove them after conversion.
					cam->setTransform( new MatrixTransform( Imath::M44f() ) );
				}
				return result;
			}
		}
	}
	return IECore::NullObject::defaultNullObject();
}

PrimitiveVariableMap LiveScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	// \todo Optimize this function, adding special cases such as for Meshes.
	ConstPrimitivePtr prim = runTimeCast< const Primitive >( readObject( time ) );
	if ( !prim )
	{
		throw Exception( "Object does not have primitive variables!" );
	}
	return prim->variables;
}

void LiveScene::writeObject( const Object *object, double time )
{
	throw Exception( "IECoreMaya::LiveScene::writeObject: write operations not supported!" );
}

void LiveScene::getChildDags( const MDagPath& dagPath, MDagPathArray& paths ) const
{
	for( unsigned i=0; i < dagPath.childCount(); ++i )
	{
		MDagPath childPath = dagPath;
		childPath.push( dagPath.child( i ) );
		
		if( dagPath.length() == 0 )
		{
			// bizarrely, this iterates through things like the translate manipulator and
			// the view cube too, so lets skip them so they don't show up:
			if( childPath.node().hasFn( MFn::kManipulator3D ) )
			{
				continue;
			}

			// looks like it also gives us the ground plane, so again, lets skip that:
			if( childPath.fullPathName() == "|groundPlane_transform" )
			{
				continue;
			}
		}
		
		paths.append( childPath );
	}
}

void LiveScene::childNames( NameList &childNames ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::childNames: Dag path no longer exists!" );
	}
	
	unsigned currentPathLength = m_dagPath.fullPathName().length();
	MDagPathArray paths;
	getChildDags( m_dagPath, paths );
	
	for( unsigned i=0; i < paths.length(); ++i )
	{
		if( paths[i].hasFn( MFn::kTransform ) )
		{
			std::string childName( paths[i].fullPathName().asChar() + currentPathLength + 1 );
			childNames.push_back( Name( childName ) );
		}
	}
}

bool LiveScene::hasChild( const Name &name ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::childNames: Dag path no longer exists!" );
	}
	
	unsigned currentPathLength = m_dagPath.fullPathName().length();
	MDagPathArray paths;
	getChildDags( m_dagPath, paths );
	
	for( unsigned i=0; i < paths.length(); ++i )
	{
		if( paths[i].hasFn( MFn::kTransform ) )
		{
			std::string childName( paths[i].fullPathName().asChar() + currentPathLength + 1 );
			if( Name( childName ) == name )
			{
				return true;
			}
		}
	}
	return false;
}

IECore::SceneInterfacePtr LiveScene::retrieveChild( const Name &name, MissingBehaviour missingBehaviour ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::retrieveChild: Dag path no longer exists!" );
	}
	
	MSelectionList sel;
	sel.add( m_dagPath.fullPathName() + "|" + std::string( name ).c_str() );
	
	MDagPath path;
	sel.getDagPath( 0, path );
	
	if( !path.hasFn( MFn::kTransform ) )
	{
		if( missingBehaviour == SceneInterface::ThrowIfMissing )
		{
			throw Exception( "IECoreMaya::LiveScene::retrieveChild: Couldn't find transform at specified path " + std::string( path.fullPathName().asChar() ) );
		}
		return 0;
	}
	
	return duplicate( path );
}

SceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	return retrieveChild( name, missingBehaviour );
}

ConstSceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	return retrieveChild( name, missingBehaviour );
}

SceneInterfacePtr LiveScene::createChild( const Name &name )
{
	return 0;
}

SceneInterfacePtr LiveScene::retrieveScene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( path.size() == 0 )
	{
		MItDag it;
		MDagPath rootPath;
		it.getPath( rootPath );
		return duplicate( rootPath, true );
	}
	
	MString pathName;
	
	for( Path::const_iterator it=path.begin(); it != path.end(); ++it )
	{
		pathName += "|";
		pathName += std::string( *it ).c_str();
	}
	
	MSelectionList sel;
	MStatus st = sel.add( pathName );
	if( !st )
	{
		if( missingBehaviour == SceneInterface::ThrowIfMissing )
		{
			std::string pathName;
			for( size_t i = 0; i < path.size(); ++i )
			{
				pathName += std::string( path[i] ) + "/";
			}
			
			throw Exception( "IECoreMaya::LiveScene::retrieveScene: Couldn't find transform at specified path " + pathName );
		}
		return 0;
	}
	
	MDagPath dagPath;
	sel.getDagPath( 0, dagPath );
	
	if( dagPath.hasFn( MFn::kTransform ) )
	{
		return duplicate( dagPath );
	}
	else
	{
		if( missingBehaviour == SceneInterface::ThrowIfMissing )
		{
			std::string pathName;
			for( size_t i = 0; i < path.size(); ++i )
			{
				pathName += std::string( path[i] ) + "/";
			}
			
			throw Exception( "IECoreMaya::LiveScene::retrieveScene: Couldn't find transform at specified path " + pathName );
		}
		return 0;
	}
	
}

ConstSceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	return retrieveScene( path, missingBehaviour );
}

SceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	return retrieveScene( path, missingBehaviour );
}

void LiveScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	throw Exception( "Hashes currently not supported in IECoreMaya::LiveScene objects." );
}

void LiveScene::registerCustomObject( HasFn hasFn, ReadFn readFn )
{
	CustomReader r;
	r.m_has = hasFn;
	r.m_read = readFn;
	customObjectReaders().push_back(r);
}

void LiveScene::registerCustomAttributes( NamesFn namesFn, ReadAttrFn readFn )
{
	registerCustomAttributes( namesFn, readFn, 0 );
}

void LiveScene::registerCustomAttributes( NamesFn namesFn, ReadAttrFn readFn, MightHaveFn mightHaveFn )
{
	CustomAttributeReader r;
	r.m_names = namesFn;
	r.m_read = readFn;
	r.m_mightHave = mightHaveFn;
	customAttributeReaders().push_back(r);
}

void LiveScene::registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn )
{
	CustomTagReader r;
	r.m_has = hasFn;
	r.m_read = readFn;
	customTagReaders().push_back( r );
}

std::vector<LiveScene::CustomReader> &LiveScene::customObjectReaders()
{
	static std::vector<LiveScene::CustomReader> readers;
	return readers;
}

std::vector<LiveScene::CustomAttributeReader> &LiveScene::customAttributeReaders()
{
	static std::vector<LiveScene::CustomAttributeReader> readers;
	return readers;
}

std::vector<LiveScene::CustomTagReader> &LiveScene::customTagReaders()
{
	static std::vector<LiveScene::CustomTagReader> readers;
	return readers;
}
