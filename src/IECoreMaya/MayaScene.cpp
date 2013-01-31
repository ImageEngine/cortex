#include "IECore/Exception.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Camera.h"
#include "IECore/TransformationMatrixData.h"

#include "IECoreMaya/MayaScene.h"
#include "IECoreMaya/FromMayaTransformConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/Convert.h"

#include "maya/MFnDagNode.h"
#include "maya/MFnTransform.h"
#include "maya/MFnCamera.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnNumericData.h"
#include "maya/MDGContext.h"
#include "maya/MSelectionList.h"
#include "maya/MString.h"
#include "maya/MTime.h"
#include "maya/MItDag.h"
#include "maya/MPlug.h"
#include "maya/MTransformationMatrix.h"
#include "maya/MSelectionList.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace IECore;
using namespace IECoreMaya;

MayaScene::MayaScene() : m_isRoot( true ), m_name( "/" )
{
	// initialize to the root path:
	MItDag it;
	it.getPath( m_dagPath );
}

MayaScene::MayaScene( const MDagPath& p ) : m_isRoot( false ), m_name( "" )
{
	m_dagPath = p;
	
	std::string nameStr = m_dagPath.fullPathName().asChar();
	boost::replace_all( nameStr, "|", "/" );
	
	m_name = nameStr;
}

MayaScene::~MayaScene()
{
}

const SceneInterface::Name& MayaScene::name() const
{
	return m_name;
}

void MayaScene::path( Path &p ) const
{
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::path: Dag path no longer exists!" );
	}
	
	std::string pathStr( name() );
	boost::tokenizer<boost::char_separator<char> > t( pathStr, boost::char_separator<char>( "/" ) );
	
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
	if( m_isRoot )
	{
		return Imath::Box3d( Imath::V3d( -10000000, -10000000, -10000000 ), Imath::V3d( 10000000, 10000000, 10000000 ) );
	}
	else if( m_dagPath.length() == 0 )
	{
		throw Exception( "MayaScene::readBound: Dag path no longer exists!" );
	}
	else
	{
		MFnDagNode dagFn( m_dagPath );
		
		MPlug plug;
		MDGContext context( MTime( time, MTime::kSeconds ) );
		MStatus st;
		
		plug = dagFn.findPlug( "boundingBoxMin", &st );
		if( !st )
		{
			return Imath::Box3d();
		}
		
		MObject min;
		plug.getValue( min, context );
		MFnNumericData fnMin( min, &st );
		if( !st )
		{
			return Imath::Box3d();
		}
		
		double minX, minY, minZ;
		fnMin.getData( minX, minY, minZ );
		
		plug = dagFn.findPlug( "boundingBoxMax", &st );
		if( !st )
		{
			return Imath::Box3d();
		}
		
		MObject max;
		plug.getValue( max, context );
		MFnNumericData fnMax( max, &st );
		if( !st )
		{
			return Imath::Box3d();
		}
		
		double maxX, maxY, maxZ;
		fnMax.getData( maxX, maxY, maxZ );
		
		Imath::Box3d ret( Imath::V3d( minX, minY, minZ ), Imath::V3d( maxX, maxY, maxZ ) );
		
		Imath::M44d invTransform = readTransformAsMatrix( time ).inverse();
		
		return Imath::transform( ret, invTransform );
	}
}

void MayaScene::writeBound( const Imath::Box3d &bound, double time )
{
}

DataPtr MayaScene::readTransform( double time ) const
{
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::readTransform: Dag path no longer exists!" );
	}
	
	if( m_dagPath.hasFn( MFn::kTransform ) )
	{
		
		MObject dagNode = m_dagPath.node();
		MFnDependencyNode fnN( dagNode );
		
		MStatus st;
		
		MPlug plug = fnN.findPlug( "matrix", &st );
		
		MDGContext context( MTime( time, MTime::kSeconds ) );
		MObject matrix;
		plug.getValue( matrix, context );
		
		MFnMatrixData fnM( matrix );
		MTransformationMatrix transform = fnM.transformation();
		
		return new IECore::TransformationMatrixdData( IECore::convert<IECore::TransformationMatrixd, MTransformationMatrix>( transform ) );
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
}

bool MayaScene::hasAttribute( const Name &name ) const
{
	return false;
}

void MayaScene::readAttributeNames( NameList &attrs ) const
{
	attrs.push_back( "renderer:immediate" );
}

ObjectPtr MayaScene::readAttribute( const Name &name, double time )
{
	if( name == "renderer:immediate" )
	{
		return new IECore::BoolData( true );
	}
	
	return 0;
}

void MayaScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
}

bool MayaScene::hasObject() const
{
	if( m_isRoot )
	{
		return false;
	}
	else if( m_dagPath.length() == 0 )
	{
		throw Exception( "MayaScene::hasObject: Dag path no longer exists!" );
	}
	
	return !m_dagPath.hasFn( MFn::kTransform );
}

ObjectPtr MayaScene::readObject( double time ) const
{
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::readObject: Dag path no longer exists!" );
	}
	
	if( m_dagPath.hasFn( MFn::kMesh ) )
	{
		MObject dagNode = m_dagPath.node();
		MFnDependencyNode fnN( dagNode );
		
		MStatus st;
		
		MPlug plug = fnN.findPlug( "outMesh", &st );
		
		MDGContext context( MTime( time, MTime::kSeconds ) );
		MObject mesh;
		plug.getValue( mesh, context );
		
		FromMayaMeshConverter converter( mesh );
		converter.spaceParameter()->setValue( new IECore::IntData( FromMayaShapeConverter::Object ) );
		return converter.convert();
	}
	else if( m_dagPath.hasFn( MFn::kCamera ) )
	{
		MFnCamera camFn( m_dagPath );
		IECore::Camera::Ptr result = new IECore::Camera("asss", new IECore::MatrixTransform );
		//result->parameters()["resolution"] = new IECore::V2iData( resolutionPlug()->getValue() );
		result->parameters()["projection"] = new IECore::StringData( camFn.isOrtho() ? "orthographic" : "perspective" );
		//result->parameters()["projection:fov"] = new IECore::FloatData( fieldOfViewPlug()->getValue() );
		result->parameters()["clippingPlanes"] = new IECore::V2fData( Imath::V2f( camFn.nearClippingPlane(), camFn.farClippingPlane() ) );
		result->addStandardParameters();
		
		return result;
	}
	
	return 0;
}

void MayaScene::writeObject( const Object *object, double time )
{
}

void MayaScene::childNames( NameList &childNames ) const
{
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::childNames: Dag path no longer exists!" );
	}
	
	unsigned currentPathLength = m_dagPath.fullPathName().length();

	for( unsigned i=0; i < m_dagPath.childCount(); ++i )
	{
		MDagPath childPath = m_dagPath;
		childPath.push( m_dagPath.child( i ) );

		std::string childName = childPath.fullPathName().asChar();

		childName = std::string( childName.begin() + currentPathLength + 1, childName.end() );

		childNames.push_back( Name( childName ) );
	}
}

IECore::SceneInterfacePtr MayaScene::retrieveChild( const Name &name, MissingBehaviour missingBehaviour ) const
{
	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "MayaScene::retrieveChild: Dag path no longer exists!" );
	}
	
	unsigned currentPathLength = m_dagPath.fullPathName().length();
	
	for( unsigned i=0; i < m_dagPath.childCount(); ++i )
	{
		MDagPath childPath = m_dagPath;
		childPath.push( m_dagPath.child( i ) );
		
		std::string childName = childPath.fullPathName().asChar();
		
		childName = std::string( childName.begin() + currentPathLength + 1, childName.end() );
		
		if( name == childName )
		{
			return new MayaScene( childPath );
		}
	}
	
	if( missingBehaviour == SceneInterface::ThrowIfMissing )
	{
		throw Exception( "MayaScene::retrieveChild: Dag path has no child named " + std::string( name ) );
	}
	
	return 0;
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
	if( path.size() == 0 )
	{
		return new MayaScene;
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
			throw Exception( "MayaScene::retrieveScene: Couldn't find location at specified path" );
		}
		return 0;
	}
	
	MDagPath dagPath;
	sel.getDagPath( 0, dagPath );
	
	return new MayaScene( dagPath );
	
}


ConstSceneInterfacePtr MayaScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	return retrieveScene( path, missingBehaviour );
}

SceneInterfacePtr MayaScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	return retrieveScene( path, missingBehaviour );
}
