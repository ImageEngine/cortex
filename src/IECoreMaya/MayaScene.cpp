#include "IECore/Exception.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Camera.h"
#include "IECore/TransformationMatrixData.h"

#include "IECoreMaya/MayaScene.h"
#include "IECoreMaya/FromMayaTransformConverter.h"
#include "IECoreMaya/FromMayaShapeConverter.h"
#include "IECoreMaya/FromMayaDagNodeConverter.h"
#include "IECoreMaya/FromMayaCameraConverter.h"
#include "IECoreMaya/Convert.h"

#include "maya/MFnDagNode.h"
#include "maya/MFnTransform.h"
#include "maya/MFnCamera.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnNumericData.h"
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

IE_CORE_DEFINERUNTIMETYPED( MayaScene );

// this stuff requires a mutex, as all them maya DG functions aint thread safe!
MayaScene::Mutex MayaScene::s_mutex;

MayaScene::MayaScene() : m_isRoot( true )
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	// initialize to the root path:
	MItDag it;
	it.getPath( m_dagPath );
}

MayaScene::MayaScene( const MDagPath& p, bool isRoot ) : m_isRoot( isRoot )
{
	// this constructor is expected to be called when s_mutex is locked!
	m_dagPath = p;
}

MayaScene::~MayaScene()
{
}

MayaScenePtr MayaScene::duplicate( const MDagPath& p, bool isRoot ) const
{
	return new MayaScene( p, isRoot );
}

SceneInterface::Name MayaScene::name() const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::name: Dag path no longer exists!" );
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

void MayaScene::path( Path &p ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::path: Dag path no longer exists!" );
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

Imath::Box3d MayaScene::readBound( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "MayaScene::readBound: time must be the same as on the maya timeline!" );
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
		throw Exception( "MayaScene::readBound: Dag path no longer exists!" );
	}
	else
	{
		MFnDagNode dagFn( m_dagPath );
		Imath::Box3d ret = IECore::convert<Imath::Box3d, MBoundingBox>( dagFn.boundingBox() );
		Imath::M44d invTransform = IECore::convert<Imath::M44d, MMatrix>( dagFn.transformationMatrix() ).inverse();
		return Imath::transform( ret, invTransform );
	}
}

void MayaScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw Exception( "MayaScene::writeBound: write operations not supported!" );
}

DataPtr MayaScene::readTransform( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::readTransform: Dag path no longer exists!" );
	}
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "MayaScene::readTransform: time must be the same as on the maya timeline!" );
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

Imath::M44d MayaScene::readTransformAsMatrix( double time ) const
{
	return runTimeCast< TransformationMatrixdData >( readTransform( time ) )->readable().transform();
}

void MayaScene::writeTransform( const Data *transform, double time )
{
	throw Exception( "MayaScene::writeTransform: write operations not supported!" );
}

bool MayaScene::hasAttribute( const Name &name ) const
{
	return false;
}

void MayaScene::readAttributeNames( NameList &attrs ) const
{
}

ObjectPtr MayaScene::readAttribute( const Name &name, double time ) const
{
	return 0;
}

void MayaScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "MayaScene::writeAttribute: write operations not supported!" );
}

bool MayaScene::hasObject() const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_isRoot )
	{
		return false;
	}
	else if( m_dagPath.length() == 0 )
	{
		throw Exception( "MayaScene::hasObject: Dag path no longer exists!" );
	}
	
	MDagPath childDag = m_dagPath;
	if( childDag.extendToShapeDirectlyBelow( 0 ) )
	{
		return true;
	}
	
	return false;
}

ObjectPtr MayaScene::readObject( double time ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::readObject: Dag path no longer exists!" );
	}
	
	if( fabs( MAnimControl::currentTime().as( MTime::kSeconds ) - time ) > 1.e-4 )
	{
		throw Exception( "MayaScene::readObject: time must be the same as on the maya timeline!" );
	}
	
	MDagPath childDag = m_dagPath;
	if( childDag.extendToShapeDirectlyBelow( 0 ) )
	{
		
		if( childDag.hasFn( MFn::kCamera ) )
		{
			FromMayaCameraConverter converter( childDag );
			CameraPtr cam = runTimeCast< Camera >( converter.convert() );
			cam->setTransform( new MatrixTransform( Imath::M44f() ) );
			return cam;
		}
		
		FromMayaShapeConverterPtr shapeConverter = FromMayaShapeConverter::create( childDag );
		if( shapeConverter )
		{
			return shapeConverter->convert();
		}
		
		FromMayaDagNodeConverterPtr dagConverter = FromMayaDagNodeConverter::create( childDag );
		if( dagConverter )
		{
			return dagConverter->convert();
		}
		
	}
	
	return 0;
}

void MayaScene::writeObject( const Object *object, double time )
{
	throw Exception( "MayaScene::writeObject: write operations not supported!" );
}

void MayaScene::getChildDags( const MDagPath& dagPath, MDagPathArray& paths ) const
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

void MayaScene::childNames( NameList &childNames ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::childNames: Dag path no longer exists!" );
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

bool MayaScene::hasChild( const Name &name ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::childNames: Dag path no longer exists!" );
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

IECore::SceneInterfacePtr MayaScene::retrieveChild( const Name &name, MissingBehaviour missingBehaviour ) const
{
	tbb::mutex::scoped_lock l( s_mutex );
	
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::retrieveChild: Dag path no longer exists!" );
	}
	
	MSelectionList sel;
	sel.add( m_dagPath.fullPathName() + "|" + std::string( name ).c_str() );
	
	MDagPath path;
	MStatus st = sel.getDagPath( 0, path );
	
	if( !path.hasFn( MFn::kTransform ) )
	{
		if( missingBehaviour == SceneInterface::ThrowIfMissing )
		{
			throw Exception( "MayaScene::retrieveChild: Couldn't find transform at specified path " + std::string( path.fullPathName().asChar() ) );
		}
		return 0;
	}
	
	return duplicate( path );
}

SceneInterfacePtr MayaScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	return retrieveChild( name, missingBehaviour );
}

ConstSceneInterfacePtr MayaScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	return retrieveChild( name, missingBehaviour );
}

SceneInterfacePtr MayaScene::createChild( const Name &name )
{
	return 0;
}

SceneInterfacePtr MayaScene::retrieveScene( const Path &path, MissingBehaviour missingBehaviour ) const
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
			
			throw Exception( "MayaScene::retrieveScene: Couldn't find transform at specified path " + pathName );
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
			
			throw Exception( "MayaScene::retrieveScene: Couldn't find transform at specified path " + pathName );
		}
		return 0;
	}
	
}


ConstSceneInterfacePtr MayaScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	return retrieveScene( path, missingBehaviour );
}

SceneInterfacePtr MayaScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	return retrieveScene( path, missingBehaviour );
}
