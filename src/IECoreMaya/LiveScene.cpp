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
#include "IECore/TransformationMatrixData.h"
#include "IECore/NullObject.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/MatrixTransform.h"
#include "IECoreScene/Camera.h"
#include "IECoreScene/Primitive.h"
#include "IECoreScene/CurvesMergeOp.h"
#include "IECoreScene/MeshMergeOp.h"

#include "IECoreMaya/LiveScene.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/FromMayaTransformConverter.h"
#include "IECoreMaya/FromMayaShapeConverter.h"
#include "IECoreMaya/FromMayaDagNodeConverter.h"
#include "IECoreMaya/FromMayaCameraConverter.h"
#include "IECoreMaya/Convert.h"
#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/FromMayaCurveConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/FromMayaInstancerConverter.h"

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
#include "maya/MFnNurbsCurve.h"
#include "maya/MGlobal.h"
#include "maya/MFnSet.h"
#include "maya/MFnInstancer.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include "boost/algorithm/string.hpp"
#include "boost/tokenizer.hpp"
#include "boost/format.hpp"

#include "tbb/recursive_mutex.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;

IE_CORE_DEFINERUNTIMETYPED( LiveScene );

namespace
{

SceneInterface::Name g_userAttrPrefix{ "user:" };
SceneInterface::Name g_mayaUserAttrPrefix{ "ieAttr_" };

void readExportableSets( std::set<SceneInterface::Name> &exportableSets, const MDagPath &dagPath )
{
	// convert maya sets to tags
	MSelectionList selectionList;
	selectionList.add( dagPath );

	MObjectArray sets;
	if( !MGlobal::getAssociatedSets( selectionList, sets ) )
	{
		return;
	}

	for( unsigned int i = 0, numSets = sets.length(); i < numSets; ++i )
	{
		MStatus s;
		MFnSet set( sets[i], &s );

		if( !s )
		{
			continue;
		}

		MPlug exportPlug = set.findPlug( "ieExport", false, &s );

		if( !s )
		{
			continue;
		}

		if( exportPlug.asBool() )
		{
			exportableSets.insert( SceneInterface::Name( set.name().asChar() ) );
		}

	}
}

}

IECoreScene::SceneInterface::Name LiveScene::visibilityOverrideName{ "ieVisibility" };

// this stuff requires a mutex, as all them maya DG functions aint thread safe!
LiveScene::Mutex LiveScene::s_mutex;
tbb::recursive_mutex g_mutex;

LiveScene::LiveScene() : m_isRoot( true )
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

	// initialize to the root path:
	MItDag it;
	it.getPath( m_dagPath );
}

LiveScene::LiveScene( const MDagPath& p, bool isRoot ) : m_isRoot( isRoot )
{
	// this constructor is expected to be called when g_mutex is locked!
	m_dagPath = p;
}

std::string LiveScene::fileName() const
{
	throw Exception( "IECoreMaya::LiveScene does not support fileName()." );
}

MDagPath LiveScene::dagPath() const
{
	return m_dagPath;
}

LiveScenePtr LiveScene::duplicate( const MDagPath& p, bool isRoot ) const
{
	return new LiveScene( p, isRoot );
}

SceneInterface::Name LiveScene::name() const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
		MFnTransform dagFn( m_dagPath );
		return new IECore::TransformationMatrixdData( IECore::convert<IECore::TransformationMatrixd, MTransformationMatrix>( dagFn.transformation() ) );
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

	// Visibility is always defined
	if( name == SceneInterface::visibilityName )
	{
		return true;
	}

	// Check the maya transform for the attribute
	Name mayaAttributeName = toMayaAttributeName( name );
	if( !mayaAttributeName.string().empty() )
	{
		MFnDependencyNode fnNode( m_dagPath.node() );
		MFnAttribute fnAttr;
		unsigned int n = fnNode.attributeCount();
		for( unsigned int i = 0; i < n; ++i )
		{
			MObject attr = fnNode.attribute( i );
			fnAttr.setObject( attr );
			Name attrName = fnAttr.name().asChar();
			if( attrName == mayaAttributeName )
			{
				return true;
			}
		}
	}

	// Check custom registered readers for the attribute name
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
			tbb::recursive_mutex::scoped_lock l( g_mutex );
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

	tbb::recursive_mutex::scoped_lock l( g_mutex );
	attrs.clear();
	attrs.push_back( SceneInterface::visibilityName );

	// Get the attributes exposed on the maya transform
	MFnDependencyNode fnNode( m_dagPath.node() );
	unsigned int n = fnNode.attributeCount();
	for( unsigned int i=0; i<n; i++ )
	{
		MObject attr = fnNode.attribute( i );
		MFnAttribute fnAttr( attr );
		Name attributeName = fromMayaAttributeName( fnAttr.name().asChar() );
		if( !attributeName.string().empty() )
		{
			attrs.push_back( attributeName );
		}
	}

	// Get any extra attributes registered with a custom reader
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

	tbb::recursive_mutex::scoped_lock l( g_mutex );

	// Check the maya transform for the attribute
	// It's important to read the transform attributes before the custom attributes so that they will
	// be found before custom attributes (giving them the opportunity to override)
	Name mayaAttributeName = toMayaAttributeName( name );
	if( !mayaAttributeName.string().empty() )
	{
		MStatus st;
		MFnDependencyNode fnNode( m_dagPath.node() );
		MPlug attrPlug = fnNode.findPlug( mayaAttributeName.c_str(), false, &st );
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

	// Read custom attributes
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

	// Special Case - Visibility
	// Let the maya transform's "visibility" attribute set scene:visible when:
	//    1) A maya override attribute doesn't exist on the transform (via ieVisibility) and
	//    2) The visibility is not set by a custom attribute reader (like a SceneShape)
	if( name == SceneInterface::visibilityName )
	{
		// The root is always visible
		if( m_isRoot )
		{
			return new BoolData( true );
		}

		// Return the transform's visibility
		MFnTransform transformFn( m_dagPath );
		bool visible = transformFn.findPlug( MPxTransform::visibility, false).asBool();
		return new BoolData( visible );
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

	std::set<Name> sets;
	readExportableSets(sets, m_dagPath);

	if( sets.find( name ) != sets.end() )
	{
		return true;
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
	readExportableSets(uniqueTags, m_dagPath);

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

SceneInterface::NameList LiveScene::setNames( bool includeDescendantSets ) const
{
	return SceneInterface::NameList();
}

IECore::PathMatcher LiveScene::readSet( const Name &name, bool includeDescendantSets, const IECore::Canceller *canceller ) const
{
	throw Exception( "IECoreMaya::LiveScene::readSet not supported" );
}

void LiveScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	throw Exception( "IECoreMaya::LiveScene::writeSet not supported" );
}

void LiveScene::hashSet( const Name& setName, IECore::MurmurHash &h ) const
{
	SceneInterface::hashSet( setName, h );
}

namespace
{

template<int MFnType>
struct PrimMergerTraits;

template<>
struct PrimMergerTraits<MFn::kNurbsCurve>
{
	typedef FromMayaCurveConverter ConverterType;
	typedef CurvesMergeOp MergeOpType;
	static CurvesPrimitiveParameter * primParameter( MergeOpType* mop )
	{
		return mop->curvesParameter();
	}
};

template<>
struct PrimMergerTraits<MFn::kMesh>
{
	typedef FromMayaMeshConverter ConverterType;
	typedef MeshMergeOp MergeOpType;
	static MeshPrimitiveParameter * primParameter( MergeOpType* mop )
	{
		return mop->meshParameter();
	}
};

template<int MFnType>
class PrimMerger
{
		typedef typename PrimMergerTraits<MFnType>::ConverterType ConverterType;
		typedef typename PrimMergerTraits<MFnType>::MergeOpType MergeOpType;
		typedef typename MergeOpType::PrimitiveType PrimitiveType;

	public:

		static void createMergeOp( MDagPath &childDag, IECore::ModifyOpPtr &op )
		{
			typename ConverterType::Ptr converter = runTimeCast<ConverterType>( ConverterType::create( childDag ) );
			if( ! converter )
			{
				throw Exception( ( boost::format( "Creating merge op failed! " ) % childDag.fullPathName().asChar() ).str() );
			}
			typename PrimitiveType::Ptr cprim = runTimeCast<PrimitiveType>( converter->convert() );
			op = new MergeOpType;
			op->copyParameter()->setTypedValue( false );
			op->inputParameter()->setValue( cprim );
		}

		static void mergePrim( MDagPath &childDag, IECore::ModifyOpPtr &op )
		{
			typename ConverterType::Ptr converter = runTimeCast<ConverterType>( ConverterType::create( childDag ) );
			if( ! converter )
			{
				throw Exception( ( boost::format( "Merging primitive failed! " ) % childDag.fullPathName().asChar() ).str() );
			}
			typename PrimitiveType::Ptr prim = runTimeCast<PrimitiveType>( converter->convert() );
			MergeOpType *mop = runTimeCast<MergeOpType>( op.get() );
			PrimMergerTraits<MFnType>::primParameter( mop )->setValue( const_cast<PrimitiveType*>( prim.get() ) );
			op->operate();
		}

};

bool hasMergeableObjects( const MDagPath &p )
{
	// When there are multiple child shapes that can be merged, readMergedObject() returns an object that has all the shapes merged in it.
	// This is because multiple Maya shapes can be converted to one IECore primitive eg. nurbs curves -> IECore::CurvesPrimitive.
	// We want to have multiple shape nodes in Maya, and want it to be one primitive if viewed through IECoreMaya::LiveScene.
	unsigned int childCount = p.childCount();

	// At least two shapes need to exist to merge.
	if( childCount < 2 )
	{
		return false;
	}

	bool isMergeable = false;
	MFnNurbsCurve::Form acceptableCurveForm = MFnNurbsCurve::kInvalid;
	int acceptableCurveDegree = -1;
	MFn::Type foundType = MFn::kInvalid;
	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MObject childObject = p.child( c );
		MFn::Type type = childObject.apiType();

		if( type == MFn::kNurbsCurve )
		{
			MFnNurbsCurve fnNCurve( childObject );

			if( acceptableCurveForm == MFnNurbsCurve::kInvalid )
			{
				acceptableCurveForm = fnNCurve.form();
			}
			else if ( fnNCurve.form() != acceptableCurveForm )
			{
				msg( Msg::Warning, p.fullPathName().asChar(), "Found curves with different kind of forms under the same transform!" );
				return false;
			}

			int degree = fnNCurve.degree();
			if( degree == 0 )
			{
				msg( Msg::Warning, p.fullPathName().asChar(), "Could not get a curve degree!" );
				return false;
			}
			if( acceptableCurveDegree == -1 )
			{
				acceptableCurveDegree = degree;
			}
			else if ( degree != acceptableCurveDegree )
			{
				msg( Msg::Warning, p.fullPathName().asChar(), "Found curves with different degrees under the same transform!" );
				return false;
			}
		}

		if( type == MFn::kMesh || type == MFn::kNurbsCurve )
		{
			if( MFnDagNode( childObject ).isIntermediateObject() )
			{
				continue;
			}

			if( foundType == MFn::kInvalid )
			{
				foundType = type;
			}
			else if( foundType == type )
			{
				isMergeable = true;
			}
			else
			{
				msg( Msg::Warning, p.fullPathName().asChar(), "Found multiple shape types under the same transform!" );
				return false;
			}
		}
	}

	return isMergeable;

}

ConstObjectPtr readMergedObject( const MDagPath &p )
{
	unsigned int childCount = p.childCount();

	IECore::ModifyOpPtr op = NULL;

	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MObject childObject = p.child( c );
		MFn::Type type = childObject.apiType();

		if( type != MFn::kNurbsCurve && type != MFn::kMesh )
		{
			continue;
		}

		MFnDagNode fnChildDag( childObject );
		if( fnChildDag.isIntermediateObject() )
		{
			continue;
		}

		MDagPath childDag;
		fnChildDag.getPath( childDag );

		if( ! op )
		{
			if( type == MFn::kNurbsCurve )
			{
				PrimMerger<MFn::kNurbsCurve>::createMergeOp( childDag, op );
			}
			else
			{
				PrimMerger<MFn::kMesh>::createMergeOp( childDag, op );
			}
		}
		else
		{
			if( type == MFn::kNurbsCurve )
			{
				PrimMerger<MFn::kNurbsCurve>::mergePrim( childDag, op );
			}
			else
			{
				PrimMerger<MFn::kMesh>::mergePrim( childDag, op );
			}
		}

	}

	assert( op );
	return op->inputParameter()->getValue();
}

} // anonymous namespace.

bool LiveScene::hasObject() const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

	if( m_isRoot )
	{
		return false;
	}
	else if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::hasObject: Dag path no longer exists!" );
	}

	if( hasMergeableObjects( m_dagPath ) )
	{
		return true;
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

	if( runTimeCast<FromMayaInstancerConverter> (FromMayaDagNodeConverter::create( m_dagPath ) ) )
	{
		return true;
	}

	return false;
}

ConstObjectPtr LiveScene::readObject( double time, const IECore::Canceller *canceller ) const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

	if( m_dagPath.length() == 0 && !m_isRoot )
	{
		throw Exception( "IECoreMaya::LiveScene::readObject: Dag path no longer exists!" );
	}

	double currentMayaTime = MAnimControl::currentTime().as( MTime::kSeconds );
	if( fabs( currentMayaTime  - time )  > 1.e-4 )
	{

		throw Exception(
			boost::str(
				boost::format( "IECoreMaya::LiveScene::readObject: time : %1% must be the same as on the maya timeline : %2% " ) %
					time %
					currentMayaTime
			)
		);
	}

	if ( hasMergeableObjects( m_dagPath ) )
	{
		return readMergedObject( m_dagPath );
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
				return dagConverter->convert();
			}
		}
	}

	 if( FromMayaInstancerConverterPtr converter = runTimeCast<FromMayaInstancerConverter> (FromMayaDagNodeConverter::create( m_dagPath ) ) )
	{
		return converter->convert();
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

		// Remove top level nodes which are not serializable
		// examples include ground plane, manipulators, hypershade cameras & geometry.
		// Perhaps there are cases where non serializable objects need to be exported but
		// it might be easier to special case add them then special case remove all unwanted objects
		if( dagPath.length() == 0 )
		{
			MStatus r;
			MFnDependencyNode depNode( childPath.node(), &r );
			if( !r )
			{
				continue;
			}

			if( !depNode.canBeWritten() )
			{
				continue;
			}
		}

		paths.append( childPath );
	}
}

void LiveScene::childNames( NameList &childNames ) const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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

IECoreScene::SceneInterfacePtr LiveScene::retrieveChild( const Name &name, MissingBehaviour missingBehaviour ) const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
		return nullptr;
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
	return nullptr;
}

SceneInterfacePtr LiveScene::retrieveScene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	tbb::recursive_mutex::scoped_lock l( g_mutex );

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
		return nullptr;
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
		return nullptr;
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

// If useful, this could be extended to register arbitrary mappings from cortex attr names to maya attr names
// If the mapping exists, and the attribute value is convertible to a maya plug, then the user has the ability
// to override the attribute value (from the perspective of Live Scene)
LiveScene::Name LiveScene::toMayaAttributeName( const LiveScene::Name &name )
{
	// Scene Visibility
	if( name == SceneInterface::visibilityName )
	{
		return visibilityOverrideName;
	}

	// User Attributes
	if( name.string().length() > 5 && boost::starts_with( name.string(), g_userAttrPrefix.string() ) )
	{
		std::string attrName = name;
		attrName = attrName.substr( 5, attrName.length() - 1 );
		boost::replace_all( attrName, ":", "__" );
		return g_mayaUserAttrPrefix.string() + attrName;
	}

	// No corresponding attribute name
	return Name();
}

LiveScene::Name LiveScene::fromMayaAttributeName( const LiveScene::Name &name )
{
	// Scene Visibility
	if( name == visibilityOverrideName )
	{
		return SceneInterface::visibilityName;
	}

	// User Attributes
	if( name.string().length() > 7 && boost::starts_with( name.string(), g_mayaUserAttrPrefix.string() ) )
	{
		std::string attrName = name;
		attrName = attrName.substr( 7, attrName.length() - 1 );
		boost::replace_all( attrName, "__", ":" );
		return g_userAttrPrefix.string() + attrName;
	}

	// No corresponding attribute name, so return empty string
	return Name();
}
