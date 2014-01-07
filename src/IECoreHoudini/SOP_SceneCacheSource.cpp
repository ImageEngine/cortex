//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "OP/OP_NodeInfoParms.h"
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_WorkArgs.h"

#include "IECore/CoordinateSystem.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Group.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/TransformOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/VisibleRenderable.h"

#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/SOP_SceneCacheSource.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

static InternedString pName( "P" );

const char *SOP_SceneCacheSource::typeName = "ieSceneCacheSource";

PRM_Name SOP_SceneCacheSource::pObjectOnly( "objectOnly", "Object Only" );

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
	const SampledSceneInterface *sampledScene = IECore::runTimeCast<const SampledSceneInterface>( scene );
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
	GeometryType geometryType = (GeometryType)this->evalInt( pGeometryType.getToken(), 0, 0 );
	
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
	hash.append( shapeFilterStr );
	hash.append( attributeFilter );
	hash.append( attributeCopy );
	hash.append( geometryType );
	hash.append( getObjectOnly() );
	
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
	params.geometryType = getGeometryType();
	getShapeFilter( params.shapeFilter );
	getTagFilter( params.tagFilter );
	
	// Building a map from shape name to primitive range, which will be used during
	// convertObject() to do a lazy update of animated primvars where possible, and
	// to destroy changing topology shapes when necessary.
	GA_ROAttributeRef nameAttrRef = gdp->findStringTuple( GA_ATTRIB_PRIMITIVE, "name" );
	if ( nameAttrRef.isValid() )
	{
		const GA_Attribute *attr = nameAttrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
		
		std::map<std::string, GA_OffsetList> offsets;
		GA_Range primRange = gdp->getPrimitiveRange();
		for ( GA_Iterator it = primRange.begin(); !it.atEnd(); ++it )
		{
			std::string current = "";
			if ( const char *value = tuple->getString( attr, it.getOffset() ) )
			{
				current = value;
			}
			
			std::map<std::string, GA_OffsetList>::iterator oIt = offsets.find( current );
			if ( oIt == offsets.end() )
			{
				oIt = offsets.insert( std::pair<std::string, GA_OffsetList>( current, GA_OffsetList() ) ).first;
			}
			
			oIt->second.append( it.getOffset() );
		}
		
		for ( std::map<std::string, GA_OffsetList>::iterator oIt = offsets.begin(); oIt != offsets.end(); ++oIt )
		{
			params.namedRanges[oIt->first] = GA_Range( gdp->getPrimitiveMap(), oIt->second );
		}
	}
	
	loadObjects( scene, transform, readTime, space, params, rootPath.size() );
	
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

void SOP_SceneCacheSource::loadObjects( const IECore::SceneInterface *scene, Imath::M44d transform, double time, Space space, Parameters &params, size_t rootSize )
{
	UT_Interrupt *progress = UTgetInterrupt();
	progress->setLongOpText( ( "Loading " + scene->name().string() ).c_str() );
	if ( progress->opInterrupt() )
	{
		return;
	}
	
	if ( scene->hasObject() && UT_String( scene->name() ).multiMatch( params.shapeFilter ) && tagged( scene, params.tagFilter ) )
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
				const InternedStringVectorData *animatedPrimVarData = IECore::runTimeCast<const InternedStringVectorData>( animatedPrimVarObj );
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
		object = modifyObject( object, params );
		
		// transform the object unless its an identity
		if ( currentTransform != Imath::M44d() )
		{
			object = transformObject( object, currentTransform, params );
		}
		
		// convert the object to Houdini
		if ( !convertObject( object, name, params ) )
		{
			std::string fullName;
			SceneInterface::Path path;
			scene->path( path );
			SceneInterface::pathToString( path, fullName );
			addWarning( SOP_MESSAGE, ( "Could not convert " + fullName + " to houdini" ).c_str() );
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
		if ( tagged( child, params.tagFilter ) )
		{
			loadObjects( child, child->readTransformAsMatrix( time ) * transform, time, space, params, rootSize );
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
					
					modified->variables[values[1]] = modified->variables[values[0]];
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
SOP_SceneCacheSource::TransformGeometricData::ReturnType SOP_SceneCacheSource::TransformGeometricData::operator()( typename T::ConstPtr data ) const
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
		
		// add all Point and Normal prim vars to the transformation list
		const PrimitiveVariableMap &variables = primitive->variables;
		std::vector<std::string> &primVars = transformer->primVarsParameter()->getTypedValue();
		primVars.clear();
		for ( PrimitiveVariableMap::const_iterator it = variables.begin(); it != variables.end(); ++it )
		{
			if ( despatchTypedData<TransformGeometricData, IECore::TypeTraits::IsGeometricTypedData, DespatchTypedDataIgnoreError>( it->second.data ) )
			{
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

bool SOP_SceneCacheSource::convertObject( const IECore::Object *object, const std::string &name, Parameters &params )
{
	ToHoudiniGeometryConverterPtr converter = 0;
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
			converter->transferAttribs( gdp, pointRange, primRange );
			
			return true;
		}
		else
		{
			// topology is changing, so destroy the exisiting primitives
			gdp->destroyPrimitives( primRange, true );
		}
	}
	
	// fallback to full conversion
	converter->nameParameter()->setTypedValue( name );
	converter->attributeFilterParameter()->setTypedValue( params.attributeFilter );
	return converter->convert( myGdpHandle );
}

void SOP_SceneCacheSource::getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms )
{
	SceneCacheNode<SOP_Node>::getNodeSpecificInfoText( context, parms );
	
	// add type descriptions for the Cortex Objects
	GeometryType geometryType = (GeometryType)this->evalInt( pGeometryType.getToken(), 0, 0 );
	if ( geometryType == Cortex )
	{
		GU_CortexPrimitive::infoText( getCookedGeo( context ), context, parms );
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

std::string SOP_SceneCacheSource::relativePath( const IECore::SceneInterface *scene, size_t rootSize )
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
