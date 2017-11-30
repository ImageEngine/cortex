//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathBoxAlgo.h"

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Renderable.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/Primitive.h"

#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/ProceduralHolderComponentBoundIterator.h"
#include "IECoreMaya/Convert.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECore/VectorOps.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/AngleConversion.h"

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MSelectionList.h"
#include "maya/MAttributeSpec.h"
#include "maya/MAttributeIndex.h"
#include "maya/MAttributeSpecArray.h"
#include "maya/MDagPath.h"
#include "maya/MFnStringData.h"
#include "maya/MPlugArray.h"
#include "maya/MObjectArray.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;
using namespace boost;

MTypeId ProceduralHolder::id = ProceduralHolderId;
MObject ProceduralHolder::aGLPreview;
MObject ProceduralHolder::aTransparent;
MObject ProceduralHolder::aDrawBound;
MObject ProceduralHolder::aDrawCoordinateSystems;
MObject ProceduralHolder::aProceduralComponents;
MObject ProceduralHolder::aComponentQueries;
MObject ProceduralHolder::aComponentTransform;
MObject ProceduralHolder::aComponentTranslate;
MObject ProceduralHolder::aComponentTranslateX;
MObject ProceduralHolder::aComponentTranslateY;
MObject ProceduralHolder::aComponentTranslateZ;
MObject ProceduralHolder::aComponentRotate;
MObject ProceduralHolder::aComponentRotateX;
MObject ProceduralHolder::aComponentRotateY;
MObject ProceduralHolder::aComponentRotateZ;
MObject ProceduralHolder::aComponentScale;
MObject ProceduralHolder::aComponentScaleX;
MObject ProceduralHolder::aComponentScaleY;
MObject ProceduralHolder::aComponentScaleZ;
MObject ProceduralHolder::aComponentBound;
MObject ProceduralHolder::aComponentBoundMin;
MObject ProceduralHolder::aComponentBoundMinX;
MObject ProceduralHolder::aComponentBoundMinY;
MObject ProceduralHolder::aComponentBoundMinZ;
MObject ProceduralHolder::aComponentBoundMax;
MObject ProceduralHolder::aComponentBoundMaxX;
MObject ProceduralHolder::aComponentBoundMaxY;
MObject ProceduralHolder::aComponentBoundMaxZ;
MObject ProceduralHolder::aComponentBoundCenter;
MObject ProceduralHolder::aComponentBoundCenterX;
MObject ProceduralHolder::aComponentBoundCenterY;
MObject ProceduralHolder::aComponentBoundCenterZ;


ProceduralHolder::ProceduralHolder()
	:	m_boundDirty( true ), m_sceneDirty( true ), m_lastRenderer( 0 )
{
}

ProceduralHolder::~ProceduralHolder()
{
}

void ProceduralHolder::postConstructor()
{
	ParameterisedHolderComponentShape::postConstructor();
	setRenderable( true );
}

void *ProceduralHolder::creator()
{
	return new ProceduralHolder;
}

MStatus ProceduralHolder::initialize()
{
	MStatus s = inheritAttributesFrom( ParameterisedHolderComponentShape::typeName );
	assert( s );

	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnCompoundAttribute cAttr;

	// drawing attributes

	aGLPreview = nAttr.create( "glPreview", "glpr", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aGLPreview );
	assert( s );

	aTransparent = nAttr.create( "transparent", "trans", MFnNumericData::kBoolean, 0, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aTransparent );
	assert( s );

	aDrawBound = nAttr.create( "drawBound", "dbnd", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aDrawBound );
	assert( s );

	aDrawCoordinateSystems = nAttr.create( "drawCoordinateSystems", "dcs", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aDrawCoordinateSystems );
	assert( s );

	// component attributes

	IECoreGL::ConstStatePtr defaultState = IECoreGL::State::defaultState();
	assert( defaultState );
	assert( defaultState->isComplete() );
	MFnStringData fnSData;
	MObject defaultValue = fnSData.create( defaultState->get<const IECoreGL::NameStateComponent>()->name().c_str(), &s );
	assert( s );

	aProceduralComponents = tAttr.create( "proceduralComponents", "prcm", MFnData::kString, defaultValue, &s );
	assert( s );
	tAttr.setReadable( true );
	tAttr.setWritable( false );
	tAttr.setStorable( false );
	tAttr.setConnectable( true );
	tAttr.setHidden( true );
	tAttr.setArray( true );
	tAttr.setUsesArrayDataBuilder( true );

	s = addAttribute( aProceduralComponents );
	assert( s );

	// component queries attribute

	aComponentQueries = tAttr.create( "componentQueries", "cmqy", MFnData::kString, fnSData.create( "" ), &s );
	assert( s );
	tAttr.setReadable( true );
	tAttr.setWritable( true );
	tAttr.setStorable( true );
	tAttr.setConnectable( true );
	tAttr.setHidden( false );
	tAttr.setArray( true );
	tAttr.setIndexMatters( true );

	s = addAttribute( aComponentQueries );
	assert( s );

	// component transform attributes

	aComponentTranslateX = nAttr.create( "componentTranslateX", "ctx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentTranslateY = nAttr.create( "componentTranslateY", "cty", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentTranslateZ = nAttr.create( "componentTranslateZ", "ctz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentTranslate = nAttr.create( "componentTranslate", "ct", aComponentTranslateX, aComponentTranslateY, aComponentTranslateZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentRotateX = nAttr.create( "componentRotateX", "crx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentRotateY = nAttr.create( "componentRotateY", "cry", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentRotateZ = nAttr.create( "componentRotateZ", "crz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentRotate = nAttr.create( "componentRotate", "cr", aComponentRotateX, aComponentRotateY, aComponentRotateZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentScaleX = nAttr.create( "componentScaleX", "csx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentScaleY = nAttr.create( "componentScaleY", "csy", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentScaleZ = nAttr.create( "componentScaleZ", "csz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentScale = nAttr.create( "componentScale", "cs", aComponentScaleX, aComponentScaleY, aComponentScaleZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentTransform = cAttr.create( "componentTransform", "ctr" );
	cAttr.addChild( aComponentTranslate );
	cAttr.addChild( aComponentRotate );
	cAttr.addChild( aComponentScale );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );

	s = addAttribute( aComponentTransform );
	assert( s );

	// component bounding box attributes

	aComponentBoundMinX = nAttr.create( "componentBoundMinX", "cbminx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMinY = nAttr.create( "componentBoundMinY", "cbminy", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMinZ = nAttr.create( "componentBoundMinZ", "cbminz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMin = nAttr.create( "componentBoundMin", "cbmin", aComponentBoundMinX, aComponentBoundMinY, aComponentBoundMinZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMaxX = nAttr.create( "componentBoundMaxX", "cbmaxx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMaxY = nAttr.create( "componentBoundMaxY", "cbmaxy", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMaxZ = nAttr.create( "componentBoundMaxZ", "cbmaxz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundMax = nAttr.create( "componentBoundMax", "cbmax", aComponentBoundMaxX, aComponentBoundMaxY, aComponentBoundMaxZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundCenterX = nAttr.create( "componentBoundCenterX", "cbcx", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundCenterY = nAttr.create( "componentBoundCenterY", "cbcy", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundCenterZ = nAttr.create( "componentBoundCenterZ", "cbcz", MFnNumericData::kFloat, 0, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBoundCenter = nAttr.create( "componentBoundCenter", "cbc", aComponentBoundCenterX, aComponentBoundCenterY, aComponentBoundCenterZ, &s );
	assert( s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aComponentBound = cAttr.create( "componentBound", "cb" );
	cAttr.addChild( aComponentBoundMin );
	cAttr.addChild( aComponentBoundMax );
	cAttr.addChild( aComponentBoundCenter );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );

	s = addAttribute( aComponentBound );
	assert( s );

	// dependencies

	attributeAffects( aComponentQueries, aComponentTransform );
	attributeAffects( aComponentQueries, aComponentBound );

	return MS::kSuccess;
}

bool ProceduralHolder::isBounded() const
{
	return true;
}

MBoundingBox ProceduralHolder::boundingBox() const
{
	if( !m_boundDirty )
	{
		return m_bound;
	}

	m_bound = MBoundingBox( MPoint( -1, -1, -1 ), MPoint( 1, 1, 1 ) );

	ParameterisedProceduralPtr p = const_cast<ProceduralHolder*>(this)->getProcedural();
	if( p )
	{
		const_cast<ProceduralHolder*>(this)->setParameterisedValues( true /* lazy */ );
		try
		{
			Box3f b = p->bound();
			if( !b.isEmpty() )
			{
				m_bound = convert<MBoundingBox>( b );
			}
		}
		catch( boost::python::error_already_set )
		{
			IECorePython::ScopedGILLock gilLock;
			PyErr_Print();
		}
		catch( std::exception &e )
		{
			msg( Msg::Error, "ProceduralHolder::boundingBox", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "ProceduralHolder::boundingBox", "Exception thrown in Procedural::bound" );
		}
	}

	m_boundDirty = false;
	return m_bound;
}

/// This method is overridden to supply a geometry iterator, which maya uses to work out
/// the bounding boxes of the components you've selected in the viewport
MPxGeometryIterator* ProceduralHolder::geometryIteratorSetup( MObjectArray& componentList, MObject& components, bool forReadOnly )
{
	if ( components.isNull() )
	{
		return new ProceduralHolderComponentBoundIterator( this, componentList );
	}
	else
	{
		return new ProceduralHolderComponentBoundIterator( this, components );
	}
}

MStatus ProceduralHolder::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if(
		std::string( plug.partialName().substring( 0, 4 ).asChar() ) == g_attributeNamePrefix ||
		plug == aDrawCoordinateSystems
	)
	{
		// it's an input to the procedural
		m_boundDirty = m_sceneDirty = true;
		m_componentToBoundMap.clear();
		childChanged( kBoundingBoxChanged ); // this is necessary to cause maya to redraw

		MPlug pComponentTransform( thisMObject(), aComponentTransform );
		for( unsigned i=0; i<pComponentTransform.numElements(); i++ )
		{
			MPlug p = pComponentTransform[i];
			for( unsigned j=0; j<p.numChildren(); j++ )
			{
				plugArray.append( p.child( j ) );
				plugArray.append( p.child( j ).child( 0 ) );
				plugArray.append( p.child( j ).child( 1 ) );
				plugArray.append( p.child( j ).child( 2 ) );
			}
		}

		MPlug pComponentBound( thisMObject(), aComponentBound );
		for( unsigned i=0; i<pComponentBound.numElements(); i++ )
		{
			MPlug p = pComponentBound[i];
			for( unsigned j=0; j<p.numChildren(); j++ )
			{
				plugArray.append( p.child( j ) );
				plugArray.append( p.child( j ).child( 0 ) );
				plugArray.append( p.child( j ).child( 1 ) );
				plugArray.append( p.child( j ).child( 2 ) );
			}
		}
	}

	return ParameterisedHolderComponentShape::setDependentsDirty( plug, plugArray );
}


Imath::Box3f ProceduralHolder::componentBound( int idx ) const
{
	ComponentToBoundMap::const_iterator boundIt = m_componentToBoundMap.find( idx );
	if( boundIt != m_componentToBoundMap.end() )
	{
		return boundIt->second;
	}

	Imath::Box3f componentBound;

	ComponentToGroupMap::const_iterator it = m_componentToGroupMap.find( idx );

	if( it == m_componentToGroupMap.end() )
	{
		return componentBound;
	}

	const std::set< std::pair< std::string, IECoreGL::GroupPtr > >& groups = it->second;
	std::set< std::pair< std::string, IECoreGL::GroupPtr > >::const_iterator groupIt = groups.begin();
	for( ; groupIt != groups.end(); ++groupIt )
	{
		IECoreGL::ConstGroupPtr group = groupIt->second;
		Imath::Box3f bound;
		for( IECoreGL::Group::ChildContainer::const_iterator it=group->children().begin(); it!=group->children().end(); it++ )
		{
			bound.extendBy( (*it)->bound() );
		}

		ComponentTransformsMap::const_iterator tIt = m_componentTransforms.find( groupIt->first );

		bound = Imath::transform( bound, tIt->second );

		componentBound.extendBy( bound );
	}

	m_componentToBoundMap[ idx ] = componentBound;

	return componentBound;
}

MStatus ProceduralHolder::compute( const MPlug &plug, MDataBlock &dataBlock )
{
	MPlug topLevelPlug = plug;
	while( topLevelPlug.isChild() || topLevelPlug.isElement() )
	{
		if( topLevelPlug.isChild() )
		{
			topLevelPlug = topLevelPlug.parent();
		}
		if( topLevelPlug.isElement() )
		{
			topLevelPlug = topLevelPlug.array();
		}
	}

	if( topLevelPlug==aComponentBound || topLevelPlug==aComponentTransform )
	{

		scene(); // to get the component maps up to date

		MArrayDataHandle queryHandle = dataBlock.inputArrayValue( aComponentQueries );

		MArrayDataHandle transformHandle = dataBlock.outputArrayValue( aComponentTransform );
		MArrayDataBuilder transformBuilder = transformHandle.builder();

		MArrayDataHandle boundHandle = dataBlock.outputArrayValue( aComponentBound );
		MArrayDataBuilder boundBuilder = boundHandle.builder();

		for( unsigned i=0; i<queryHandle.elementCount(); i++, queryHandle.next() )
		{

			MString name = queryHandle.inputValue().asString();

			ComponentTransformsMap::const_iterator tIt = m_componentTransforms.find( name.asChar() );
			ComponentsMap::const_iterator cIt = m_componentsMap.find( name.asChar() );
			if( tIt!=m_componentTransforms.end() && cIt!=m_componentsMap.end() )
			{
				Imath::V3f translate( 0 ), shear( 0 ), rotate( 0 ), scale( 1 );
				Imath::extractSHRT( convert<Imath::M44f>( tIt->second ), scale, shear, rotate, translate );
				rotate = radiansToDegrees( rotate );

				MDataHandle transformElementHandle = transformBuilder.addElement( queryHandle.elementIndex() );
				transformElementHandle.child( aComponentTranslate ).set3Float( translate[0], translate[1], translate[2] );
				transformElementHandle.child( aComponentRotate ).set3Float( rotate[0], rotate[1], rotate[2] );
				transformElementHandle.child( aComponentScale ).set3Float( scale[0], scale[1], scale[2] );

				IECoreGL::GroupPtr group = cIt->second.second;
				Imath::Box3f bound;
				for( IECoreGL::Group::ChildContainer::const_iterator it=group->children().begin(); it!=group->children().end(); it++ )
				{
					bound.extendBy( (*it)->bound() );
				}
				bound = Imath::transform( bound, tIt->second );

				MDataHandle boundElementHandle = boundBuilder.addElement( queryHandle.elementIndex() );
				boundElementHandle.child( aComponentBoundMin ).set3Float( bound.min[0],  bound.min[1],  bound.min[2] );
				boundElementHandle.child( aComponentBoundMax ).set3Float( bound.max[0],  bound.max[1],  bound.max[2] );
				Imath::V3f boundCenter = bound.center();
				boundElementHandle.child( aComponentBoundCenter ).set3Float( boundCenter[0], boundCenter[1], boundCenter[2] );
			}

		}

		transformHandle.set( transformBuilder );
		boundHandle.set( boundBuilder );

		dataBlock.setClean( aComponentTransform );
		dataBlock.setClean( aComponentTranslate );
		dataBlock.setClean( aComponentRotate );
		dataBlock.setClean( aComponentScale );
		dataBlock.setClean( aComponentBound );
		dataBlock.setClean( aComponentBoundMin );
		dataBlock.setClean( aComponentBoundMax );
		dataBlock.setClean( aComponentBoundCenter );

		return MS::kSuccess;
	}

	return ParameterisedHolderComponentShape::compute( plug, dataBlock );
}

MStatus ProceduralHolder::setProcedural( const std::string &className, int classVersion )
{
	return setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" );
}

IECoreScene::ParameterisedProceduralPtr ProceduralHolder::getProcedural( std::string *className, int *classVersion )
{
	return runTimeCast<IECoreScene::ParameterisedProcedural>( getParameterised( className, classVersion ) );
}

IECoreGL::ConstScenePtr ProceduralHolder::scene()
{
	if( !m_sceneDirty  )
	{
		return m_scene;
	}

	m_scene = 0;
	ParameterisedProceduralPtr p = ((ProceduralHolder*)this)->getProcedural();
	if( p )
	{
		setParameterisedValues( true /* lazy */ );
		try
		{
			IECoreGL::RendererPtr rendererToReuse = 0;
			{
				IECorePython::ScopedGILLock gilLock;
				boost::python::object pythonProcedural( p );
				if( m_lastRenderer && PyObject_HasAttrString( pythonProcedural.ptr(), "willRerender" ) )
				{
					/// \todo Consider how we might modify the ParameterisedProcedural (and possibly Renderer::Procedural?) interface
					/// to properly support rerendering. Do this in conjunction with the todo in IECoreGL::Renderer::command() (about formalising a
					/// proper interface for specifying scene edits to a Renderer).
					bool rerender = boost::python::extract<bool>( pythonProcedural.attr( "willRerender" )( m_lastRenderer, IECore::ObjectPtr( p->parameters()->getValue() ) ) );
					if( rerender )
					{
						rendererToReuse = m_lastRenderer;
					}
				}
			}

			if( rendererToReuse )
			{
				rendererToReuse->command( "editBegin", CompoundDataMap() );

				p->render( rendererToReuse.get() );
				m_scene = rendererToReuse->scene();

				rendererToReuse->command( "editEnd", CompoundDataMap() );
			}
			else
			{
				IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
				renderer->setOption( "gl:mode", new StringData( "deferred" ) );
				renderer->setOption( "gl:drawCoordinateSystems", new BoolData( MPlug( thisMObject(), aDrawCoordinateSystems ).asBool() ) );
				renderer->worldBegin();

					// using the form with many arguments so that we can customise
					// rendering. in particular it's very important that the geometry
					// is rendered immediately and not deferred in a procedural. if it
					// were deferred then that procedural might end up being called on
					// another thread, and then try to get the GIL - this results in deadlock
					// as sometimes (but not always) maya holds the GIL on the thread calling
					// scene(). it's ok if the procedural spawns more procedurals because at
					// that point we release the GIL ourselves in the bindings.
					p->render(
						renderer.get(),
						false, // we don't need an attribute block
						true, // we do want doRenderState() called
						true, // we do want geometry (doRender)
						true  // we want geometry rendered immediately (not deferred in a procedural call)
					);

				renderer->worldEnd();

				m_scene = renderer->scene();
				m_scene->setCamera( 0 );

				m_lastRenderer = renderer;
			}

			buildComponents();
		}
		catch( boost::python::error_already_set )
		{
			IECorePython::ScopedGILLock gilLock;
			PyErr_Print();
		}
		catch( const std::exception &e )
		{
			msg( Msg::Error, "ProceduralHolder::scene", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "ProceduralHolder::scene", "Caught unknown exception" );
		}
	}

	m_sceneDirty = false;
	return m_scene;
}

void ProceduralHolder::componentToPlugs( MObject &component, MSelectionList &selectionList ) const
{
	MStatus s;

	if ( component.hasFn( MFn::kSingleIndexedComponent ) )
	{
		MFnSingleIndexedComponent fnComp( component, &s );
		assert( s );
		MObject thisNode = thisMObject();
		MPlug plug( thisNode, aProceduralComponents );
		assert( !plug.isNull() );

		int len = fnComp.elementCount( &s );
		assert( s );
		for ( int i = 0; i < len; i++ )
		{
			MPlug compPlug = plug.elementByLogicalIndex( fnComp.element(i), &s );
			assert( s );
			assert( !compPlug.isNull() );

			selectionList.add( compPlug );
		}
	}
}

MPxSurfaceShape::MatchResult ProceduralHolder::matchComponent( const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list )
{
	if( spec.length() == 1 )
	{
		MAttributeSpec attrSpec = spec[0];
		MStatus s;

		int dim = attrSpec.dimensions();

		if ( (dim > 0) && (attrSpec.name() == "proceduralComponents" || attrSpec.name() == "prcm" || attrSpec.name() == "f" ) )
		{
			int numComponents = m_componentToGroupMap.size();

			MAttributeIndex attrIndex = attrSpec[0];

			if ( attrIndex.type() != MAttributeIndex::kInteger )
			{
				return MPxSurfaceShape::kMatchInvalidAttributeRange;
			}

			int upper = numComponents - 1;
			int lower = 0;
			if ( attrIndex.hasLowerBound() )
			{
				attrIndex.getLower( lower );
			}
			if ( attrIndex.hasUpperBound() )
			{
				attrIndex.getUpper( upper );
			}

			// Check the attribute index range is valid
			if ( (attrIndex.hasRange() && !attrIndex.hasValidRange() ) || (upper >= numComponents) || (lower < 0 ) )
			{
				return MPxSurfaceShape::kMatchInvalidAttributeRange;
			}

			MDagPath path;
			item.getDagPath( 0, path );
			MFnSingleIndexedComponent fnComp;
			MObject comp = fnComp.create( MFn::kMeshPolygonComponent, &s );
			assert( s );

			for ( int i=lower; i<=upper; i++ )
			{
				fnComp.addElement( i );
			}

			list.add( path, comp );

			return MPxSurfaceShape::kMatchOk;
		}
	}

	return MPxSurfaceShape::matchComponent( item, spec, list );
}

void ProceduralHolder::buildComponents( IECoreGL::ConstNameStateComponentPtr nameState, IECoreGL::GroupPtr group, const Imath::M44f &parentTransform )
{
	assert( nameState );
	assert( group );
	assert( group->getState() );

	MStatus s;

	if (  group->getState()->get< IECoreGL::NameStateComponent >() )
	{
		nameState = group->getState()->get< IECoreGL::NameStateComponent >();
	}

	Imath::M44f groupTransform = group->getTransform() * parentTransform;

	const std::string &name = nameState->name();

	ComponentsMap::const_iterator it = m_componentsMap.find( name );
	if( it == m_componentsMap.end() )
	{
		/// Reserve slot in the componentsMap. The exact component ID gets generated later, once all components have been
		/// traversed. IDs are then assigned in ascending order whilst iterating over the component map, which is sorted by name. This
		/// ensures a consistent ordering of components from frame to frame, which we'd not otherwise get due to IECore::Group using
		/// a regular set (sorted by pointer) to store its children.
		m_componentsMap[name] = ComponentsMap::mapped_type( 0, group );
		m_componentTransforms[name] = groupTransform;
	}

	const IECoreGL::Group::ChildContainer &children = group->children();

	for ( IECoreGL::Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
	{
		assert( *it );

		group = runTimeCast< IECoreGL::Group >( *it );
		if ( group )
		{
			buildComponents( nameState, group, groupTransform );
		}
	}
}

/// Blank implementation of this method. This is to avoid a crash when you try and use the rotation manipulator maya gives
/// you when you've selected procedural components in rotation mode (maya 2013)
void ProceduralHolder::transformUsing( const MMatrix &mat, const MObjectArray &componentList, MPxSurfaceShape::MVertexCachingMode cachingMode, MPointArray *pointCache )
{
}

void ProceduralHolder::buildComponents()
{
	MStatus s;
	MDataBlock block = forceCache();

	MArrayDataHandle cH = block.outputArrayValue( aProceduralComponents, &s );
	assert( s );

	m_componentsMap.clear();

	m_componentToGroupMap.clear();

	IECoreGL::ConstStatePtr defaultState = IECoreGL::State::defaultState();
	assert( defaultState );
	assert( defaultState->isComplete() );
	assert( defaultState->get<const IECoreGL::NameStateComponent>() );

	assert( m_scene );
	assert( m_scene->root() );

	buildComponents( defaultState->get<const IECoreGL::NameStateComponent>(), m_scene->root(), Imath::M44f() );


	MArrayDataBuilder builder = cH.builder( &s );
	assert( s );

	int compId = 0;
	ComponentsMap::iterator it = m_componentsMap.begin();
	for ( ; it != m_componentsMap.end(); it ++ )
	{
		/// Build the mapping that goes from ID -> ( name, group )
		m_componentToGroupMap[compId].insert( ComponentToGroupMap::mapped_type::value_type( it->first.value(), it->second.second ) );

		it->second.first = compId ++;

		MFnStringData fnData;
		MObject data = fnData.create( MString( it->first.value().c_str() ) );
		MDataHandle h = builder.addElement( it->second.first, &s );
		assert( s );

		s = h.set( data );
		assert( s );
	}

	s = cH.set( builder );
	assert( s );

#ifndef NDEBUG
	MPlug plug( thisMObject(), aProceduralComponents );
	for ( ComponentsMap::const_iterator it = m_componentsMap.begin(); it != m_componentsMap.end(); ++it )
	{
		MPlug child = plug.elementByLogicalIndex( it->second.first, &s );
		assert( s );
		MObject obj;
		s = child.getValue( obj );
		assert( s );
		MFnStringData fnData( obj, &s );
		assert( s );

		assert( fnData.string() == MString( it->first.value().c_str() ) );
	}

#endif
}
