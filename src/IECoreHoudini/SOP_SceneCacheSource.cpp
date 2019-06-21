//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/SOP_SceneCacheSource.h"

#include "IECoreHoudini/GEO_CortexPrimitive.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

#include "IECoreScene/CoordinateSystem.h"
#include "IECoreScene/Group.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/TransformOp.h"
#include "IECoreScene/VisibleRenderable.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include "GA/GA_Names.h"
#include "OP/OP_NodeInfoParms.h"
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_WorkArgs.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

static InternedString pName( "P" );

const char *SOP_SceneCacheSource::typeName = "ieSceneCacheSource";

PRM_Name SOP_SceneCacheSource::pObjectOnly( "objectOnly", "Object Only" );
PRM_Name SOP_SceneCacheSource::pVisibilityFilter( "visibilityFilter", "Visibility Filter" );

OP_TemplatePair *SOP_SceneCacheSource::buildParameters()
{
	static OP_TemplatePair *templatePair = 0;
	if ( templatePair )
	{
		return templatePair;
	}

	PRM_Template *mainTemplate = SOP_SceneCacheSource::buildMainParameters()->myTemplate;
	PRM_Template *optionTemplate = SOP_SceneCacheSource::buildOptionParameters()->myTemplate;

	unsigned numMainParms = PRM_Template::countTemplates( mainTemplate );
	unsigned numOptionParms = PRM_Template::countTemplates( optionTemplate );

	static PRM_Template *thisTemplate = new PRM_Template[ numMainParms + numOptionParms + 2 ];

	// add the generic SceneCacheNode parms
	unsigned totalParms = 0;
	for ( unsigned i = 0; i < numMainParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = mainTemplate[i];
	}

	// add the generic SceneCacheNode option parms
	for ( unsigned i = 0; i < numOptionParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = optionTemplate[i];
	}

	// add the parms for this node
	thisTemplate[totalParms] = PRM_Template(
		PRM_TOGGLE, 1, &pObjectOnly, 0, 0, 0, &sceneParmChangedCallback, 0, 0,
		"Determines whether this SOP cooks the current object only, or traverses down through the hierarchy."
	);
	totalParms++;

	// add the visiblity filter parm
	thisTemplate[totalParms] = PRM_Template(
		PRM_TOGGLE, 1, &pVisibilityFilter, 0, 0, 0, &sceneParmChangedCallback, 0, 0,
		"Determines whether this SOP cull out hidden location or not."
	);
	totalParms++;

	templatePair = new OP_TemplatePair( thisTemplate );
	return templatePair;
}

SOP_SceneCacheSource::SOP_SceneCacheSource( OP_Network *net, const char *name, OP_Operator *op ) : SceneCacheNode<SOP_Node>( net, name, op )
{
}

SOP_SceneCacheSource::~SOP_SceneCacheSource()
{
}

OP_Node *SOP_SceneCacheSource::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_SceneCacheSource( net, name, op );
}

bool SOP_SceneCacheSource::getObjectOnly() const
{
	return evalInt( pObjectOnly.getToken(), 0, 0 );
}

void SOP_SceneCacheSource::setObjectOnly( bool objectOnly )
{
	setInt( pObjectOnly.getToken(), 0, 0, objectOnly );
	sceneChanged();
}

void SOP_SceneCacheSource::sceneChanged()
{
	SceneCacheNode<SOP_Node>::sceneChanged();

	std::string file;
	if ( !ensureFile( file ) )
	{
		m_static = boost::indeterminate;
		return;
	}

	m_static = false;

	ConstSceneInterfacePtr scene = this->scene( file, getPath() );
	const SampledSceneInterface *sampledScene = IECore::runTimeCast<const SampledSceneInterface>( scene.get() );
	if ( sampledScene )
	{
		bool objectOnly = this->evalInt( pObjectOnly.getToken(), 0, 0 );
		m_static = ( objectOnly && sampledScene->hasObject() ) ? ( sampledScene->numObjectSamples() < 2 ) : ( sampledScene->numBoundSamples() < 2 );
	}

	flags().setTimeDep( bool( !m_static ) );
}

OP_ERROR SOP_SceneCacheSource::cookMySop( OP_Context &context )
{
	// make sure the state is valid
	if ( boost::indeterminate( m_static ) )
	{
		sceneChanged();
	}

	flags().setTimeDep( bool( !m_static ) );

	std::string file;
	if ( !ensureFile( file ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, ( file + " is not a valid .scc" ).c_str() );
		gdp->clearAndDestroy();
		return error();
	}

	std::string path = getPath();
	Space space = getSpace();
	SOP_SceneCacheSource::GeometryType geometryType = (SOP_SceneCacheSource::GeometryType)this->evalInt( pGeometryType.getToken(), 0, 0 );

	UT_String tagFilterStr;
	getTagFilter( tagFilterStr );
	UT_StringMMPattern tagFilter;
	tagFilter.compile( tagFilterStr );

	UT_String shapeFilterStr;
	getShapeFilter( shapeFilterStr );
	UT_StringMMPattern shapeFilter;
	shapeFilter.compile( shapeFilterStr );

	UT_String p( "P" );
	UT_String attributeFilter;
	getAttributeFilter( attributeFilter );
	if ( !p.match( attributeFilter ) )
	{
		attributeFilter += " P";
	}

	UT_String attributeCopy;
	getAttributeCopy( attributeCopy );

	UT_String fullPathName;
	getFullPathName( fullPathName );

	ConstSceneInterfacePtr scene = this->scene( file, path );
	if ( !scene )
	{
		addError( SOP_ATTRIBUTE_INVALID, ( path + " is not a valid location in " + file ).c_str() );
		gdp->clearAndDestroy();
		return error();
	}

	MurmurHash hash;
	hash.append( file );
	hash.append( path );
	hash.append( space );
	hash.append( tagFilterStr );
	hash.append( getTagGroups() );
	hash.append( shapeFilterStr );
	hash.append( attributeFilter );
	hash.append( attributeCopy );
	hash.append( fullPathName );
	hash.append( geometryType );
	hash.append( getObjectOnly() );
	hash.append( evalInt( pVisibilityFilter.getToken(), 0, 0 ) );

	if ( !m_loaded || m_hash != hash )
	{
		gdp->clearAndDestroy();
	}

	double readTime = time( context );
	Imath::M44d transform = ( space == World ) ? worldTransform( file, path, readTime ) : Imath::M44d();

	SceneInterface::Path rootPath;
	scene->path( rootPath );

	UT_Interrupt *progress = UTgetInterrupt();
	if ( !progress->opStart( ( "Cooking objects for " + getPath() ).c_str() ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, "Cooking interrupted before it started" );
		gdp->clearAndDestroy();
		return error();
	}

	Parameters params;
	UT_String attribFilter;
	getAttributeFilter( attribFilter );
	params.attributeFilter = attribFilter.toStdString();
	params.attributeCopy = attributeCopy.toStdString();
	params.fullPathName = fullPathName.toStdString();
	params.geometryType = getGeometryType();
	params.tagGroups = getTagGroups();
	getShapeFilter( params.shapeFilter );
	getTagFilter( params.tagFilter );

	// Building a map from shape name to primitive range, which will be used during
	// convertObject() to do a lazy update of animated primvars where possible, and
	// to destroy changing topology shapes when necessary.
	GA_ROHandleS nameAttrib( gdp, GA_ATTRIB_PRIMITIVE, GA_Names::name );
	if( nameAttrib.isValid() )
	{
		std::map<std::string, GA_OffsetList> offsets;

		GA_Offset start, end;
		for( GA_Iterator it( gdp->getPrimitiveRange() ); it.blockAdvance( start, end ); )
		{
			for( GA_Offset offset = start; offset < end; ++offset )
			{
				std::string current = "";
				if( const char *value = nameAttrib.get( offset ) )
				{
					current = value;
				}
				auto oIt = offsets.insert( { current, GA_OffsetList() } ).first;
				oIt->second.append( offset );
			}
		}

		for( const auto &kv : offsets )
		{
			params.namedRanges[kv.first] = GA_Range( gdp->getPrimitiveMap(), kv.second );
		}
	}

	loadObjects( scene.get(), transform, readTime, space, params, rootPath.size(), SceneInterface::rootName.string() );

	if ( progress->opInterrupt( 100 ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, "Cooking interrupted" );
		gdp->clearAndDestroy();
		m_loaded = false;
		m_hash = MurmurHash();
	}
	else
	{
		m_loaded = true;
		m_hash = hash;
	}

	progress->opEnd();

	return error();
}

void SOP_SceneCacheSource::loadObjects( const IECoreScene::SceneInterface *scene, Imath::M44d transform, double time, Space space, Parameters &params, size_t rootSize, std::string currentPath, bool inheritedVisibility )
{
	UT_Interrupt *progress = UTgetInterrupt();
	progress->setLongOpText( ( "Loading " + scene->name().string() ).c_str() );
	if ( progress->opInterrupt() )
	{
		return;
	}

	if ( currentPath != scene->name().string() )
	{
		if ( currentPath.back() != '/' )
		{
			currentPath += "/";
		}
		currentPath += scene->name().string();
	}

	if ( UT_String( currentPath ).multiMatch( params.shapeFilter ) && tagged( scene, params.tagFilter ) )
	{
		// filter out visibility or inherited visibility
		if ( evalInt( pVisibilityFilter.getToken(), 0, 0 ) )
		{
			if ( inheritedVisibility && scene->hasAttribute( IECoreScene::SceneInterface::visibilityName ) )
			{
				inheritedVisibility = IECore::runTimeCast< const IECore::BoolData >( scene->readAttribute( IECoreScene::SceneInterface::visibilityName, time ) )->readable();
			}

			if ( !inheritedVisibility )
			{
				// check the primitve range map to see if this shape exists already
				std::map<std::string, GA_Range>::iterator rIt = params.namedRanges.find( currentPath );
				if ( rIt != params.namedRanges.end() && !rIt->second.isEmpty() )
				{
					GA_Range primRange = rIt->second;
					gdp->destroyPrimitives( primRange, true );
				}

				// recurse to delete child primitive if necessary
				SceneInterface::NameList children;
				scene->childNames( children );
				for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
				{
					ConstSceneInterfacePtr child = scene->child( *it );
					if ( tagged( child.get(), params.tagFilter ) )
					{
						loadObjects( child.get(), child->readTransformAsMatrix( time ) * transform, time, space, params, rootSize, currentPath, false /* visibility is off if we reach this point */ );
					}
				}
				return;
			}
			
		}

		if ( scene->hasObject() )
		{
			std::string name = relativePath( scene, rootSize );

			Imath::M44d currentTransform;
			if ( space == Local )
			{
				currentTransform = scene->readTransformAsMatrix( time );
			}
			else if ( space != Object )
			{
				currentTransform = transform;
			}

			ConstObjectPtr object = 0;
			if ( params.geometryType == BoundingBox )
			{
				Imath::Box3d bound = scene->readBound( time );
				object = MeshPrimitive::createBox( Imath::Box3f( bound.min, bound.max ) );

				params.hasAnimatedTopology = false;
				params.hasAnimatedPrimVars = true;
				params.animatedPrimVars.clear();
				params.animatedPrimVars.push_back( "P" );
			}
			else if ( params.geometryType == PointCloud )
			{
				std::vector<Imath::V3f> point( 1, scene->readBound( time ).center() );
				PointsPrimitivePtr points = new PointsPrimitive( new V3fVectorData( point ) );
				std::vector<Imath::V3f> basis1( 1, Imath::V3f( currentTransform[0][0], currentTransform[0][1], currentTransform[0][2] ) );
				std::vector<Imath::V3f> basis2( 1, Imath::V3f( currentTransform[1][0], currentTransform[1][1], currentTransform[1][2] ) );
				std::vector<Imath::V3f> basis3( 1, Imath::V3f( currentTransform[2][0], currentTransform[2][1], currentTransform[2][2] ) );
				points->variables["basis1"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( basis1 ) );
				points->variables["basis2"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( basis2 ) );
				points->variables["basis3"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( basis3 ) );

				params.hasAnimatedTopology = false;
				params.hasAnimatedPrimVars = true;
				params.animatedPrimVars.clear();
				params.animatedPrimVars.push_back( "P" );
				params.animatedPrimVars.push_back( "basis1" );
				params.animatedPrimVars.push_back( "basis2" );
				params.animatedPrimVars.push_back( "basis3" );

				object = points;
			}
			else
			{
				object = scene->readObject( time );

				params.hasAnimatedTopology = scene->hasAttribute( SceneCache::animatedObjectTopologyAttribute );
				params.hasAnimatedPrimVars = scene->hasAttribute( SceneCache::animatedObjectPrimVarsAttribute );
				if ( params.hasAnimatedPrimVars )
				{
					const ConstObjectPtr animatedPrimVarObj = scene->readAttribute( SceneCache::animatedObjectPrimVarsAttribute, 0 );
					const InternedStringVectorData *animatedPrimVarData = IECore::runTimeCast<const InternedStringVectorData>( animatedPrimVarObj.get() );
					if ( animatedPrimVarData )
					{
						const std::vector<InternedString> &values = animatedPrimVarData->readable();
						params.animatedPrimVars.clear();
						params.animatedPrimVars.resize( values.size() );
						std::copy( values.begin(), values.end(), params.animatedPrimVars.begin() );
					}
				}

			}

			// modify the object if necessary
			object = modifyObject( object.get(), params );

			// transform the object unless its an identity
			if ( currentTransform != Imath::M44d() )
			{
				object = transformObject( object.get(), currentTransform, params );
			}

			// convert the object to Houdini
			if ( !convertObject( object.get(), name, scene, params ) )
			{
				addWarning( SOP_MESSAGE, ( "Could not convert " + currentPath + " to Houdini" ).c_str() );
			}
		}
	}

	if ( evalInt( pObjectOnly.getToken(), 0, 0 ) )
	{
		return;
	}

	SceneInterface::NameList children;
	scene->childNames( children );
	std::sort( children.begin(), children.end(), InternedStringSort() );
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr child = scene->child( *it );
		if ( tagged( child.get(), params.tagFilter ) )
		{
			loadObjects( child.get(), child->readTransformAsMatrix( time ) * transform, time, space, params, rootSize, currentPath, inheritedVisibility );
		}
	}
}

ConstObjectPtr SOP_SceneCacheSource::modifyObject( const IECore::Object *object, Parameters &params )
{
	ConstObjectPtr result = object;

	if ( params.attributeCopy != "" )
	{
		if ( const Primitive *primitive = IECore::runTimeCast<const Primitive>( object ) )
		{
			PrimitivePtr modified = 0;
			const PrimitiveVariableMap &variables = primitive->variables;

			UT_WorkArgs pairs;
			UT_String args( params.attributeCopy );
			args.tokenize( pairs, " " );
			for ( int i = 0; i < pairs.entries(); ++i )
			{
				UT_WorkArgs values;
				UT_String pair( pairs[i] );
				pair.tokenize( values, ":" );
				if ( values.entries() != 2 )
				{
					continue;
				}

				PrimitiveVariableMap::const_iterator it = variables.find( values[0] );
				if ( it != variables.end() )
				{
					if ( !modified )
					{
						modified = primitive->copy();
					}

					// we need to copy the data in case either copy will be modified later on
					const PrimitiveVariable &orig = modified->variables[values[0]];
					modified->variables[values[1]] = PrimitiveVariable( orig.interpolation, orig.data->copy() );
				}
			}

			if ( modified )
			{
				result = modified;
			}
		}
	}

	return result;
}

template<typename T>
SOP_SceneCacheSource::TransformGeometricData::ReturnType SOP_SceneCacheSource::TransformGeometricData::operator()( const T *data ) const
{
	GeometricData::Interpretation interp = data->getInterpretation();
	return ( interp == GeometricData::Point || interp == GeometricData::Normal || interp == GeometricData::Vector );
}

ConstObjectPtr SOP_SceneCacheSource::transformObject( const IECore::Object *object, const Imath::M44d &transform, Parameters &params )
{
	if ( const Primitive *primitive = IECore::runTimeCast<const Primitive>( object ) )
	{
		TransformOpPtr transformer = new TransformOp();
		transformer->inputParameter()->setValue( const_cast<Primitive*>( primitive ) ); // safe because we set the copy parameter
		transformer->copyParameter()->setTypedValue( true );
		transformer->matrixParameter()->setValue( new M44dData( transform ) );

		// add all Point and Normal prim vars to the transformation list, except for rest/Pref
		const PrimitiveVariableMap &variables = primitive->variables;
		std::vector<std::string> &primVars = transformer->primVarsParameter()->getTypedValue();
		primVars.clear();
		for ( PrimitiveVariableMap::const_iterator it = variables.begin(); it != variables.end(); ++it )
		{
			if ( despatchTypedData<TransformGeometricData, IECore::TypeTraits::IsGeometricTypedData, DespatchTypedDataIgnoreError>( it->second.data.get() ) )
			{
				// we don't want to alter rest/Pref because Houdini excepts these to be non-transforming prim vars
				if ( it->first == "rest" || it->first == "Pref" )
				{
					continue;
				}

				primVars.push_back( it->first );

				// add the transforming prim vars to the animated list
				if ( std::find( params.animatedPrimVars.begin(), params.animatedPrimVars.end(), it->first ) == params.animatedPrimVars.end() )
				{
					params.animatedPrimVars.push_back( it->first );
					params.hasAnimatedPrimVars = true;
				}
			}
		}

		return transformer->operate();
	}
	else if ( const Group *group = IECore::runTimeCast<const Group>( object ) )
	{
		GroupPtr result = group->copy();
		MatrixTransformPtr matTransform = matrixTransform( transform );
		if ( const Transform *transform = group->getTransform() )
		{
			matTransform->matrix *= transform->transform();
		}
		result->setTransform( matTransform );
		return result;
	}
	else if ( const CoordinateSystem *coord = IECore::runTimeCast<const CoordinateSystem>( object ) )
	{
		CoordinateSystemPtr result = coord->copy();
		MatrixTransformPtr matTransform = matrixTransform( transform );
		if ( const Transform *transform = coord->getTransform() )
		{
			matTransform->matrix *= transform->transform();
		}
		result->setTransform( matTransform );
		return result;
	}

	return object;
}

bool SOP_SceneCacheSource::convertObject( const IECore::Object *object, const std::string &name, const SceneInterface *scene, Parameters &params )
{
	ToHoudiniGeometryConverterPtr converter = nullptr;
	if ( params.geometryType == Cortex )
	{
		converter = new ToHoudiniCortexObjectConverter( object );
	}
	else
	{
		const VisibleRenderable *renderable = IECore::runTimeCast<const VisibleRenderable>( object );
		if ( !renderable )
		{
			return false;
		}

		converter = ToHoudiniGeometryConverter::create( renderable );
	}

	if ( !converter )
	{
		return false;
	}

	// we need to set the name regardless of whether
	// we're reusing prims or doing the full conversion
	// because this parameter can have an affect in
	// transferAttribs() as well as convert()
	converter->nameParameter()->setTypedValue( name );

	// check the primitve range map to see if this shape exists already
	std::map<std::string, GA_Range>::iterator rIt = params.namedRanges.find( name );
	if ( rIt != params.namedRanges.end() && !rIt->second.isEmpty() )
	{
		GA_Range primRange = rIt->second;
		const Primitive *primitive = IECore::runTimeCast<const Primitive>( object );
		if ( primitive && !params.hasAnimatedTopology && params.hasAnimatedPrimVars )
		{
			// this means constant topology and primitive variables, even though multiple samples were written
			if ( params.animatedPrimVars.empty() )
			{
				return true;
			}

			GA_Range pointRange( *gdp, primRange, GA_ATTRIB_POINT, GA_Range::primitiveref(), false );

			// update the animated primitive variables only
			std::string animatedPrimVarStr = "";
			for ( std::vector<InternedString>::const_iterator it = params.animatedPrimVars.begin(); it != params.animatedPrimVars.end(); ++it )
			{
				animatedPrimVarStr += it->string() + " ";
			}

			converter->attributeFilterParameter()->setTypedValue( animatedPrimVarStr );

			try
			{
				converter->transferAttribs( gdp, pointRange, primRange );
				return true;
			}
			catch ( std::exception &e )
			{
				addWarning( SOP_MESSAGE, e.what() );
				return false;
			}
			catch ( ... )
			{
				addWarning( SOP_MESSAGE, "Attribute transfer failed for unknown reasons" );
				return false;
			}
		}
		else
		{
			// topology is changing, so destroy the exisiting primitives
			gdp->destroyPrimitives( primRange, true );
		}
	}

	// fallback to full conversion
	converter->attributeFilterParameter()->setTypedValue( params.attributeFilter );

	try
	{
		GA_Offset firstNewPrim = gdp->getPrimitiveMap().lastOffset() + 1;

		bool status = converter->convert( myGdpHandle );

		// adds the full path in addition to the relative name
		const GA_IndexMap &primMap = gdp->getPrimitiveMap();
		GA_Range newPrims( primMap, firstNewPrim, primMap.lastOffset() + 1 );

		if ( params.fullPathName != "" )
		{
			if ( newPrims.isValid() )
			{
				std::string fullName;
				SceneInterface::Path path;
				scene->path( path );
				SceneInterface::pathToString( path, fullName );

				GA_RWAttributeRef pathAttribRef = ToHoudiniStringVectorAttribConverter::convertString( params.fullPathName, fullName, gdp, newPrims );
				status = status && pathAttribRef.isValid();
			}
		}

		if ( params.tagGroups )
		{
			static UT_StringMMPattern convertTagFilter;
			if( convertTagFilter.isEmpty() )
			{
				convertTagFilter.compile( "ObjectType:*" );
			}
			SceneInterface::NameList tags;
			scene->readTags( tags, IECoreScene::SceneInterface::LocalTag );
			for ( SceneInterface::NameList::const_iterator it=tags.begin(); it != tags.end(); ++it )
			{
				UT_String tag( *it );
				// skip this tag because it's used behind the scenes
				if ( tag.multiMatch( convertTagFilter ) )
				{
					continue;
				}

				// replace this special character found in SCC tags that will prevent the group from being created
				tag.substitute(":", "_");

				tag.prepend("ieTag_");

				GA_PrimitiveGroup *group = gdp->findPrimitiveGroup(tag);
				if ( !group )
				{
					group = gdp->newPrimitiveGroup(tag);
				}
				group->addRange(newPrims);
			}
		}

		return status;
	}
	catch ( std::exception &e )
	{
		addWarning( SOP_MESSAGE, e.what() );
		return false;
	}
	catch ( ... )
	{
		addWarning( SOP_MESSAGE, "Conversion failed for unknown reasons" );
		return false;
	}
}

void SOP_SceneCacheSource::getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms )
{
	SceneCacheNode<SOP_Node>::getNodeSpecificInfoText( context, parms );

	// add type descriptions for the Cortex Objects
	GeometryType geometryType = (GeometryType)this->evalInt( pGeometryType.getToken(), 0, 0 );
	if ( geometryType == Cortex )
	{
		GEO_CortexPrimitive::infoText( getCookedGeo( context ), context, parms );
		return;
	}

	// add conversion details for Houdini geo
	UT_String p( "P" );
	UT_String filter;
	evalString( filter, pAttributeFilter.getToken(), 0, 0 );
	if ( !p.match( filter ) )
	{
		filter += " P";
	}
	UT_StringMMPattern attributeFilter;
	attributeFilter.compile( filter );

	/// \todo: this text could come from a static method on a class that manages these name relations (once that exists)
	parms.append( "Converting standard Cortex PrimitiveVariables:\n" );
	if ( UT_String( "s" ).multiMatch( attributeFilter ) && UT_String( "t" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  s,t -> uv\n" );
	}

	if ( UT_String( "Cs" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Cs -> Cd\n" );
	}

	if ( UT_String( "Pref" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Pref -> rest\n" );
	}

	if ( UT_String( "width" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  width -> pscale\n" );
	}

	if ( UT_String( "Os" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Os -> Alpha\n" );
	}
}

MatrixTransformPtr SOP_SceneCacheSource::matrixTransform( Imath::M44d t )
{
	return new MatrixTransform(
		Imath::M44f(
			t[0][0], t[0][1], t[0][2], t[0][3],
			t[1][0], t[1][1], t[1][2], t[1][3],
			t[2][0], t[2][1], t[2][2], t[2][3],
			t[3][0], t[3][1], t[3][2], t[3][3]
		)
	);
}

std::string SOP_SceneCacheSource::relativePath( const IECoreScene::SceneInterface *scene, size_t rootSize )
{
	SceneInterface::Path path, relative;
	scene->path( path );

	SceneInterface::Path::iterator start = path.begin() + rootSize;
	if ( start != path.end() )
	{
		relative.resize( path.end() - start );
		std::copy( start, path.end(), relative.begin() );
	}

	std::string result;
	SceneInterface::pathToString( relative, result );

	return result;
}

bool SOP_SceneCacheSource::InternedStringSort::operator() ( const SceneInterface::Name &i, const SceneInterface::Name &j )
{
	return ( i.string() < j.string() );
}
