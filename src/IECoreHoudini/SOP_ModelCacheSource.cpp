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

#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "UT/UT_StringMMPattern.h"

#include "IECore/CoordinateSystem.h"
#include "IECore/Group.h"
#include "IECore/TransformOp.h"
#include "IECore/VisibleRenderable.h"

#include "IECoreHoudini/SOP_ModelCacheSource.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

PRM_Name SOP_ModelCacheSource::pShapeFilter( "shapeFilter", "Shape Filter" );
PRM_Name SOP_ModelCacheSource::pAttributeFilter( "attributeFilter", "Attribute Filter" );

PRM_Default SOP_ModelCacheSource::shapeFilterDefault( 0, "*" );
PRM_Default SOP_ModelCacheSource::attributeFilterDefault( 0, "*" );

PRM_ChoiceList SOP_ModelCacheSource::shapeFilterMenu( PRM_CHOICELIST_TOGGLE, &SOP_ModelCacheSource::buildShapeFilterMenu );

OP_TemplatePair *SOP_ModelCacheSource::buildParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		unsigned numMDCParms = PRM_Template::countTemplates( ModelCacheNode<SOP_Node>::parameters );
		thisTemplate = new PRM_Template[ numMDCParms + 3 ];
		thisTemplate[0] = ModelCacheNode<SOP_Node>::parameters[0];
		thisTemplate[1] = ModelCacheNode<SOP_Node>::parameters[1];
		thisTemplate[2] = PRM_Template(
			PRM_STRING, 1, &pShapeFilter, &shapeFilterDefault, &shapeFilterMenu, 0, 0, 0, 0,
			"A list of filters to decide which shapes to load. Uses Houdini matching syntax"
		);
		thisTemplate[3] = PRM_Template(
			PRM_STRING, 1, &pAttributeFilter, &attributeFilterDefault, 0, 0, 0, 0, 0,
			"A list of attribute names to load, if they exist on each shape. Uses Houdini matching syntax. "
			"P will always be loaded."
		);
		for ( unsigned i = 2; i < numMDCParms; ++i )
		{
			thisTemplate[2+i] = ModelCacheNode<SOP_Node>::parameters[i];
		}
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

SOP_ModelCacheSource::SOP_ModelCacheSource( OP_Network *net, const char *name, OP_Operator *op ) : ModelCacheNode<SOP_Node>( net, name, op )
{
}

SOP_ModelCacheSource::~SOP_ModelCacheSource()
{
}

OP_Node *SOP_ModelCacheSource::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_ModelCacheSource( net, name, op );
}

void SOP_ModelCacheSource::buildShapeFilterMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	SOP_ModelCacheSource *node = reinterpret_cast<SOP_ModelCacheSource*>( data );
	if ( !node )
	{
		return;
	}
	
	menu[0].setToken( "*" );
	menu[0].setLabel( "*" );
	
	std::string file;
	if ( !node->ensureFile( file ) )
	{
		// mark the end of our menu
		menu[1].setToken( 0 );
		return;
	}
	
	std::string path = node->getPath();
	
	std::vector<std::string> objects;
	ModelCacheUtil::Cache::EntryPtr entry = cache().entry( file, path );
	node->objectNames( entry->modelCache(), objects );
	node->createMenu( menu, objects );
}

OP_ERROR SOP_ModelCacheSource::cookMySop( OP_Context &context )
{
	gdp->stashAll();
	
	std::string file;
	if ( !ensureFile( file ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, ( file + " is not a valid .mdc" ).c_str() );
		gdp->destroyStashed();
		return error();
	}
	
	std::string path = getPath();
	
	UT_String value;
	evalString( value, pShapeFilter.getToken(), 0, 0 );
	UT_StringMMPattern shapeFilter;
	shapeFilter.compile( value );
	
	UT_String p( "P" );
	evalString( value, pAttributeFilter.getToken(), 0, 0 );
	UT_StringMMPattern attributeFilter;
	if ( !p.match( value ) )
	{
		value += " P";
	}
	attributeFilter.compile( value );
	
	Space space = (Space)evalInt( pSpace.getToken(), 0, 0 );
	Imath::M44d transform = ( space == World ) ? cache().worldTransform( file, path ) : Imath::M44d();
	
	ModelCacheUtil::Cache::EntryPtr entry = cache().entry( file, path );
	loadObjects( entry->modelCache(), transform, space, shapeFilter, attributeFilter );
	
	gdp->destroyStashed();
	return error();
}

void SOP_ModelCacheSource::loadObjects( const IECore::ModelCache *cache, Imath::M44d transform, Space space, const UT_StringMMPattern &shapeFilter, const UT_StringMMPattern &attributeFilter )
{
	if ( cache->hasObject() && UT_String( cache->name() ).multiMatch( shapeFilter ) )
	{
		ObjectPtr object = cache->readObject();
		
		std::string fullName = cache->path();
		/// \todo: we either need a reserved character to reconstruct the hierarchy from group names
		/// or we need Cortex to start using a name attribute instead of groups
		boost::algorithm::replace_all( fullName, "/", "_" );
		boost::algorithm::replace_all( fullName, ":", "" );
		
		modifyObject( object, fullName, attributeFilter );
		
		Imath::M44d currentTransform;
		if ( space == Leaf )
		{
			currentTransform = cache->readTransform();
		}
		else if ( space != Object )
		{
			currentTransform = transform;
		}
		
		transformObject( object, currentTransform );
		
		VisibleRenderable *renderable = IECore::runTimeCast<VisibleRenderable>( object );
		if ( renderable )
		{
			ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( renderable );
			if ( !converter || !converter->convert( myGdpHandle ) )
			{
				addError( SOP_LOAD_UNKNOWN_BINARY_FLAG, ( "Could not convert " + cache->path() + " to houdini" ).c_str() );
			}
		}
	}
	
	IndexedIO::EntryIDList children;
	cache->childNames( children );
	for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstModelCachePtr child = cache->readableChild( *it );
		loadObjects( child, child->readTransform() * transform, space, shapeFilter, attributeFilter );
	}
}

IECore::ObjectPtr SOP_ModelCacheSource::modifyObject( IECore::Object *object, std::string &name, const UT_StringMMPattern &attributeFilter )
{
	VisibleRenderable *renderable = IECore::runTimeCast<VisibleRenderable>( object );
	if ( !renderable )
	{
		return object;
	}
	
	renderable->blindData()->member<StringData>( "name", false, true )->writable() = name;
	
	Primitive *primitive = IECore::runTimeCast<Primitive>( renderable );
	if ( primitive )
	{
		PrimitiveVariableMap &variables = primitive->variables;
		PrimitiveVariableMap::iterator it = variables.begin();
		PrimitiveVariableMap::iterator toErase = variables.begin();
		while ( it != variables.end() )
		{
			UT_String attributeName( it->first );
			if ( !attributeName.multiMatch( attributeFilter ) )
			{
				variables.erase( it++ );
			}
			else
			{
				it++;
			}
		}
	}
	
	return object;
};

IECore::ObjectPtr SOP_ModelCacheSource::transformObject( IECore::Object *object, Imath::M44d transform )
{
	Primitive *primitive = IECore::runTimeCast<Primitive>( object );
	if ( primitive )
	{
		TransformOpPtr transformer = new TransformOp();
		transformer->inputParameter()->setValue( primitive );
		transformer->copyParameter()->setTypedValue( false );
		transformer->matrixParameter()->setValue( new M44dData( transform ) );
		return transformer->operate();
	}
	
	Group *group = IECore::runTimeCast<Group>( object );
	if ( group )
	{
		group->setTransform( matrixTransform( transform ) );
		return group;
	}
	
	CoordinateSystem *coord = IECore::runTimeCast<CoordinateSystem>( object );
	if ( coord )
	{
		coord->setTransform( matrixTransform( transform ) );
		return coord;
	}
	
	return object;
}

MatrixTransformPtr SOP_ModelCacheSource::matrixTransform( Imath::M44d t )
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
