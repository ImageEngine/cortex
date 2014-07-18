//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "OBJ/OBJ_Geometry.h"
#include "OBJ/OBJ_SubNet.h"
#include "PRM/PRM_Include.h"
#include "PRM/PRM_Parm.h"
#include "PRM/PRM_SpareData.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/OBJ_SceneCacheNode.h"

using namespace IECore;
using namespace IECoreHoudini;

template<typename BaseType>
OBJ_SceneCacheNode<BaseType>::OBJ_SceneCacheNode( OP_Network *net, const char *name, OP_Operator *op )
	: SceneCacheNode<BaseType>( net, name, op )
{
}

template<typename BaseType>
OBJ_SceneCacheNode<BaseType>::~OBJ_SceneCacheNode()
{
}

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pMainSwitcher( "mainSwitcher", "Main Switcher" );
	
template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pExpand( "expand", "Expand" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pPush( "push", "Push Options" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pCollapse( "collapse", "Collapse" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pExpanded( "expanded", "Expanded" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pOverrideTransform( "overrideTransform", "Override Transform" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pOutTranslate( "outT", "Out Translate" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pOutRotate( "outR", "Out Rotate" );

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pOutScale( "outS", "Out Scale" );

static PRM_Default outTranslateDefault[] = {
	PRM_Default( 0, "hou.pwd().parmTransform().extractTranslates()[0]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractTranslates()[1]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractTranslates()[2]", CH_PYTHON_EXPRESSION )
};

static PRM_Default outRotateDefault[] = {
	PRM_Default( 0, "hou.pwd().parmTransform().extractRotates()[0]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractRotates()[1]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractRotates()[2]", CH_PYTHON_EXPRESSION )
};

static PRM_Default outScaleDefault[] = {
	PRM_Default( 0, "hou.pwd().parmTransform().extractScales()[0]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractScales()[1]", CH_PYTHON_EXPRESSION ),
	PRM_Default( 0, "hou.pwd().parmTransform().extractScales()[2]", CH_PYTHON_EXPRESSION )
};

static void copyParm( PRM_Template &src, PRM_Template &dest, bool visible = true )
{
	PRM_Name *name = new PRM_Name( src.getToken(), src.getLabel(), src.getExpressionFlag() );
	name->harden();
	
	PRM_Type type = ( visible ) ? src.getType() : (PRM_Type) (src.getType() | PRM_TYPE_INVISIBLE);
	
	dest.initialize(
		type,
		src.getTypeExtended(),
		src.exportLevel(),
		src.getVectorSize(),
		name,
		src.getFactoryDefaults(),
		src.getChoiceListPtr(),
		src.getRangePtr(),
		src.getCallback(),
		src.getSparePtr(),
		src.getParmGroup(),
		(const char *)src.getHelpText(),
		src.getConditionalBasePtr()
	);
}

template<typename BaseType>
PRM_Template *OBJ_SceneCacheNode<BaseType>::buildParameters( OP_TemplatePair *extraParameters )
{
	static PRM_Template *thisTemplate = 0;
	if ( thisTemplate )
	{
		return thisTemplate;
	}
	
	PRM_Template *objTemplate = BaseType::getTemplateList( OBJ_PARMS_PLAIN );
	PRM_Template *mainTemplate = OBJ_SceneCacheNode<BaseType>::buildMainParameters()->myTemplate;
	PRM_Template *optionTemplate = OBJ_SceneCacheNode<BaseType>::buildOptionParameters()->myTemplate;
	PRM_Template *extraTemplate = ( extraParameters ) ? extraParameters->myTemplate : 0;
	PRM_Template *expansionTemplate = buildExpansionParameters()->myTemplate;
	PRM_Template *outputTemplate = buildOutputParameters()->myTemplate;
	
	unsigned numObjParms = PRM_Template::countTemplates( objTemplate );
	unsigned numMainParms = PRM_Template::countTemplates( mainTemplate );
	unsigned numOptionParms = PRM_Template::countTemplates( optionTemplate );
	unsigned numExtraParms = ( extraTemplate ) ? PRM_Template::countTemplates( extraTemplate ) : 0;
	unsigned numExpansionParms = PRM_Template::countTemplates( expansionTemplate );
	unsigned numOutputParms = PRM_Template::countTemplates( outputTemplate );
	
	thisTemplate = new PRM_Template[ numObjParms + numMainParms + numOptionParms + numExtraParms + numExpansionParms + numOutputParms + 3 ];
	
	// add the SceneCacheNode folders to the stdswitcher
	unsigned switcherIndex = PRM_Template::getTemplateIndexByToken( objTemplate, "stdswitcher" );
	PRM_Template &stdswitcher = objTemplate[switcherIndex];
	unsigned numFolders = stdswitcher.getVectorSize();
	static PRM_Default *folders = new PRM_Default[ numFolders + 3 ];
	folders[0] = PRM_Default( numMainParms + numExtraParms + numExpansionParms, "Main" );
	folders[1] = PRM_Default( numOptionParms + 1, "Options" );
	folders[2] = PRM_Default( numOutputParms, "Output" );
	
	// add the normal folders
	PRM_Default *defaults = stdswitcher.getFactoryDefaults();
	for ( unsigned j = 0; j < numFolders; ++j )
	{
		// add an extra parm to the transform folder
		if ( !strcmp( defaults[j].getString(), "Transform" ) )
		{
			folders[j+3] = PRM_Default( defaults[j].getFloat() + 1, defaults[j].getString() );
		}
		else
		{
			folders[j+3] = defaults[j];
		}
	}
	
	// re-init the stdswitcher so we get our new folders
	thisTemplate[0] = stdswitcher;
	thisTemplate[0].initialize(
		stdswitcher.getType(),
		stdswitcher.getTypeExtended(),
		stdswitcher.exportLevel(),
		numFolders + 3,
		stdswitcher.getNamePtr(),
		folders,
		stdswitcher.getChoiceListPtr(),
		stdswitcher.getRangePtr(),
		stdswitcher.getCallback(),
		stdswitcher.getSparePtr(),
		stdswitcher.getParmGroup(),
		(const char *)stdswitcher.getHelpText(),
		stdswitcher.getConditionalBasePtr()
	);
	
	// add the generic SceneCacheNode parms
	unsigned totalParms = 1;
	for ( unsigned i = 0; i < numMainParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = mainTemplate[i];
	}
	
	// add the extra parms for this node
	for ( unsigned i = 0; i < numExtraParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = extraTemplate[i];
	}
	
	// add the generic OBJ_SceneCacheNode expansion parms
	for ( unsigned i = 0; i < numExpansionParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = expansionTemplate[i];
	}
	
	// add the push options button
	thisTemplate[totalParms] = PRM_Template(
		PRM_CALLBACK, 1, &pPush, 0, 0, 0, &OBJ_SceneCacheNode<BaseType>::pushButtonCallback, 0, 0,
		"Push the relevant parameter values to the hierarchy below.\n"
	);
	totalParms++;	
	
	// add the generic SceneCacheNode option parms
	for ( unsigned i = 0; i < numOptionParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = optionTemplate[i];
	}
	
	// add the OBJ_SceneCacheNode output parms
	for ( unsigned i = 0; i < numOutputParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = outputTemplate[i];
	}
	
	// add the override parm
	thisTemplate[totalParms] = PRM_Template(
		PRM_TOGGLE, 1, &pOverrideTransform, 0, 0, 0, &OBJ_SceneCacheNode<BaseType>::sceneParmChangedCallback, 0, 0,
		"Determines whether this OBJ reads from file or from the user parms."
	);
	totalParms++;
	
	// add the generic OBJ_Node parms
	int transformIndex = thisTemplate[0].findSwitcherFolderWithLabel( "Transform" );
	for ( unsigned i = 0; i < numObjParms; ++i, ++totalParms )
	{
		// this was added above
		if ( i == switcherIndex )
		{
			totalParms--;
			continue;
		}
		
		thisTemplate[totalParms] = objTemplate[i];
		
		int switcher, folder;
		bool visible = true;
		bool inFolder = PRM_Template::getEnclosingSwitcherFolder( thisTemplate, totalParms, switcher, folder );
		if ( inFolder && ( folder == transformIndex ) )
		{
			visible = (
				!strcmp( thisTemplate[totalParms].getToken(), "t" ) ||
				!strcmp( thisTemplate[totalParms].getToken(), "r" ) ||
				!strcmp( thisTemplate[totalParms].getToken(), "s" )
			);
		}
		
		copyParm( objTemplate[i], thisTemplate[totalParms], visible );
	}
	
	return thisTemplate;
}

template<typename BaseType>
OP_TemplatePair *OBJ_SceneCacheNode<BaseType>::buildExpansionParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		thisTemplate = new PRM_Template[4];
		
		thisTemplate[0] = PRM_Template(
			PRM_CALLBACK, 1, &pExpand, 0, 0, 0, &OBJ_SceneCacheNode<BaseType>::expandButtonCallback, 0, 0,
			"Expand the hierarchy below the specified root path.\n"
			"Some nodes may define additional options that are used during the expansion process."
		);
		
		thisTemplate[1] = PRM_Template(
			PRM_CALLBACK, 1, &pCollapse, 0, 0, 0, &OBJ_SceneCacheNode<BaseType>::collapseButtonCallback, 0, 0,
			"Clean the hierarchy below the specified root path. Note that this is a destructive operation. "
			"All nodes contained within will be deleted."
		);
		
		thisTemplate[2] = PRM_Template(
			PRM_TOGGLE, 1, &pExpanded, 0, 0, 0, 0, 0, 0,
			"A toggle to indicate whether this level is expanded or not. This does not affect cooking, "
			"and the value may be changed by automated scripts. Expansion will be blocked when this is on."
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

template<typename BaseType>
OP_TemplatePair *OBJ_SceneCacheNode<BaseType>::buildOutputParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		thisTemplate = new PRM_Template[5];
		
		thisTemplate[0] = PRM_Template(
			PRM_XYZ | PRM_TYPE_NOCOOK, 3, &pOutTranslate, outTranslateDefault, 0, 0, 0, 0, 0,
			"Output translation calculated by this node. This is for user clarity only and is not editable."
		);
		
		thisTemplate[1] = PRM_Template(
			PRM_XYZ | PRM_TYPE_NOCOOK, 3, &pOutRotate, outRotateDefault, 0, 0, 0, 0, 0,
			"Output rotation calculated by this node. This is for user clarity only and is not editable."
		);
		
		thisTemplate[2] = PRM_Template(
			PRM_XYZ | PRM_TYPE_NOCOOK, 3, &pOutScale, outScaleDefault, 0, 0, 0, 0, 0,
			"Output scale calculated by this node. This is for user clarity only and is not editable."
		);
		
		thisTemplate[3] = PRM_Template(
			PRM_STRING | PRM_TYPE_NOCOOK, 1, &HoudiniScene::pTags, 0, 0, 0, 0, 0, 0,
			"A space separated list of tags to add when caching with the SceneCache ROP."
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

template<typename BaseType>
int OBJ_SceneCacheNode<BaseType>::expandButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	std::string file;
	OBJ_SceneCacheNode<BaseType> *node = reinterpret_cast<OBJ_SceneCacheNode<BaseType>*>( data );
	if ( !node || !node->ensureFile( file ) || node->evalInt( pExpanded.getToken(), 0, 0 ) )
	{
		return 0;
	}
	
	node->setDisplay( false );
	node->expandHierarchy( node->scene( file, node->getPath() ).get() );
	node->setDisplay( true );
	
	return 1;
}

template<typename BaseType>
int OBJ_SceneCacheNode<BaseType>::pushButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	std::string file;
	OBJ_SceneCacheNode<BaseType> *node = reinterpret_cast<OBJ_SceneCacheNode<BaseType>*>( data );
	if ( !node || !node->ensureFile( file ) || !node->evalInt( pExpanded.getToken(), 0, 0 ) )
	{
		return 0;
	}
	
	node->pushToHierarchy();
	
	return 1;
}

template<typename BaseType>
int OBJ_SceneCacheNode<BaseType>::collapseButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	OBJ_SceneCacheNode<BaseType> *node = reinterpret_cast<OBJ_SceneCacheNode<BaseType>*>( data );
	if ( !node )
	{
		return 0;
	}
	
	node->collapseHierarchy();
	
	return 1;
}

template<typename BaseType>
void OBJ_SceneCacheNode<BaseType>::collapseHierarchy()
{
	OP_NodeList childNodes;
	for ( int i=0; i < this->getNchildren(); ++i )
	{
		childNodes.append( this->getChild( i ) );
	}
	
	this->destroyNodes( childNodes );
	this->setInt( pExpanded.getToken(), 0, 0, 0 );
}

template<typename BaseType>
void OBJ_SceneCacheNode<BaseType>::sceneChanged()
{
	SceneCacheNode<BaseType>::sceneChanged();
	this->m_static = boost::indeterminate;
}

template<typename BaseType>
void OBJ_SceneCacheNode<BaseType>::updateState()
{
	// do not read from file if overriding
	if ( this->evalInt( pOverrideTransform.getToken(), 0, 0 ) )
	{
		this->m_static = boost::indeterminate;
		return;
	}
	
	std::string file;
	if ( !OBJ_SceneCacheNode<BaseType>::ensureFile( file ) )
	{
		this->m_static = boost::indeterminate;
		return;
	}
	
	std::string path = this->getPath();
	
	ConstSceneInterfacePtr scene = this->scene( file, path );
	const SampledSceneInterface *sampledScene = IECore::runTimeCast<const SampledSceneInterface>( scene.get() );
	
	this->m_static = ( sampledScene ) ? ( sampledScene->numTransformSamples() < 2 ) : false;
	
	// only update time dependency if Houdini thinks its static
	if ( !BaseType::flags().getTimeDep() && !BaseType::getParmList()->getCookTimeDependent() )
	{
		BaseType::flags().setTimeDep( bool( !this->m_static ) );
		BaseType::getParmList()->setCookTimeDependent(  bool( !this->m_static ) );
	}
}

template<typename BaseType>
bool OBJ_SceneCacheNode<BaseType>::getParmTransform( OP_Context &context, UT_DMatrix4 &xform )
{
	std::string file = this->getFile();
	std::string path = this->getPath();
	OBJ_SceneCacheNode<OP_Node>::Space space = (OBJ_SceneCacheNode<OP_Node>::Space)this->getSpace();
	
	MurmurHash hash;
	hash.append( file );
	hash.append( path );
	hash.append( space );
	
	// make sure the state is valid
	if ( boost::indeterminate( this->m_static ) )
	{
		updateState();
	}
	
	// only update time dependency if Houdini thinks its static
	if ( !BaseType::flags().getTimeDep() && !BaseType::getParmList()->getCookTimeDependent() )
	{
		BaseType::flags().setTimeDep( bool( !this->m_static ) );
		BaseType::getParmList()->setCookTimeDependent( bool( !this->m_static ) );	
	}
	
	// do not read from file if overriding
	if ( this->evalInt( pOverrideTransform.getToken(), 0, 0 ) )
	{
		BaseType::getParmTransform( context, xform );
		return true;
	}
	
	if ( this->m_static == true && this->m_loaded && this->m_hash == hash )
	{
		xform = m_xform;
		return true;
	}
	
	if ( !SceneCacheNode<BaseType>::ensureFile( file ) )
	{
		SceneCacheNode<BaseType>::addError( OBJ_ERR_CANT_FIND_OBJ, ( file + " is not a valid .scc" ).c_str() );
		return false;
	}
	
	ConstSceneInterfacePtr scene = this->scene( file, path );
	if ( !scene )
	{
		SceneCacheNode<BaseType>::addError( OBJ_ERR_CANT_FIND_OBJ, ( path + " is not a valid location in " + file ).c_str() );
		return false;
	}
	
	Imath::M44d transform;
	if ( space == SceneCacheNode<OP_Node>::World )
	{
		transform = SceneCacheNode<BaseType>::worldTransform( file, path, this->time( context ) );
	}
	else if ( space == SceneCacheNode<OP_Node>::Local )
	{
		transform = scene->readTransformAsMatrix( this->time( context ) );
	}
	
	xform = IECore::convert<UT_Matrix4D>( transform );
	m_xform = xform;
	this->m_hash = hash;
	this->m_loaded = true;
	
	return true;
}

template<typename BaseType>
OP_ERROR OBJ_SceneCacheNode<BaseType>::cookMyObj( OP_Context &context )
{
	OP_ERROR status = BaseType::cookMyObj( context );
	
	// only update time dependency if Houdini thinks its static
	if ( !BaseType::flags().getTimeDep() && !BaseType::getParmList()->getCookTimeDependent() )
	{
		BaseType::flags().setTimeDep( bool( !this->m_static ) );
		BaseType::getParmList()->setCookTimeDependent( bool( !this->m_static ) );	
	}
	
	return status;
}

template<typename BaseType>
bool OBJ_SceneCacheNode<BaseType>::updateParmsFlags()
{
	bool expanded = this->evalInt( pExpanded.getToken(), 0, 0 );
	this->enableParm( pExpand.getToken(), !expanded );
	this->enableParm( pExpanded.getToken(), !expanded );
	this->enableParm( pCollapse.getToken(), expanded );
	bool override = this->evalInt( pOverrideTransform.getToken(), 0, 0 );
	this->enableParm( "t", override );
	this->enableParm( "r", override );
	this->enableParm( "s", override );
	this->enableParm( pOutTranslate.getToken(), false );
	this->enableParm( pOutRotate.getToken(), false );
	this->enableParm( pOutScale.getToken(), false );
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Known Specializations
//////////////////////////////////////////////////////////////////////////////////////////

template class OBJ_SceneCacheNode<OBJ_Geometry>;
template class OBJ_SceneCacheNode<OBJ_SubNet>;
