//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

// Houdini
#include "UT/UT_Interrupt.h"
#include "CH/CH_LocalVariable.h"
#include "PRM/PRM_Include.h"
#include "PRM/PRM_Parm.h"

// Cortex
#include "IECore/ParameterisedProcedural.h"
#include "IECore/SimpleTypedData.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECoreGL/Renderer.h"

// IECoreHoudini
#include "CoreHoudini.h"
#include "NodePassData.h"
#include "SOP_ProceduralHolder.h"

using namespace boost::python;
using namespace IECoreHoudini;

/// Parameter names for non-dynamic SOP parameters
PRM_Name SOP_ProceduralHolder::opTypeParm( "__opType", "Procedural:" );
PRM_Name SOP_ProceduralHolder::opVersionParm( "__opVersion", "  Version:" );
PRM_Name SOP_ProceduralHolder::opParmEval( "__opParmEval", "ParameterEval" );
PRM_Name SOP_ProceduralHolder::opMatchString( "__opMatchString", "MatchString" );
PRM_Default SOP_ProceduralHolder::opMatchStringDefault( 0.f, "*" );
PRM_Name SOP_ProceduralHolder::opReloadBtn( "__opReloadBtn", "Reload" );
PRM_Name SOP_ProceduralHolder::switcherName( "__switcher", "Switcher" );

PRM_ChoiceList SOP_ProceduralHolder::typeMenu( PRM_CHOICELIST_SINGLE, &SOP_ProceduralHolder::buildTypeMenu );
PRM_ChoiceList SOP_ProceduralHolder::versionMenu( PRM_CHOICELIST_SINGLE, &SOP_ProceduralHolder::buildVersionMenu );

/// Detail for our switcher
PRM_Default SOP_ProceduralHolder::switcherDefaults[] = {
	PRM_Default( 0, "Parameters" ),
};

/// Add parameters to SOP
PRM_Template SOP_ProceduralHolder::myParameters[] = {
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT, 1, &opTypeParm, 0, &typeMenu, 0, &SOP_ProceduralHolder::reloadClassCallback ),
	PRM_Template( PRM_STRING|PRM_TYPE_JOIN_NEXT , 1, &opVersionParm, 0, &versionMenu, 0, &SOP_ProceduralHolder::reloadClassCallback ),
	PRM_Template( PRM_CALLBACK, 1, &opReloadBtn, 0, 0, 0, &SOP_ProceduralHolder::reloadButtonCallback ),
	PRM_Template( PRM_INT|PRM_TYPE_INVISIBLE, 1, &opParmEval ),
	PRM_Template( PRM_STRING|PRM_TYPE_INVISIBLE, 1, &opMatchString, &opMatchStringDefault ),
	PRM_Template( PRM_SWITCHER, 1, &switcherName, switcherDefaults ),
	PRM_Template()
};

/// Don't worry about variables today
CH_LocalVariable SOP_ProceduralHolder::myVariables[] = {
	{ 0, 0, 0 },
};

/// Houdini's static creator method
OP_Node *SOP_ProceduralHolder::myConstructor( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_ProceduralHolder( net, name, op );
}

/// Ctor
SOP_ProceduralHolder::SOP_ProceduralHolder(OP_Network *net, const char *name, OP_Operator *op )
	: SOP_ParameterisedHolder( net, name, op ), m_scene( 0 ), m_renderDirty( true )
{
	getParm("__opParmEval").setExpression( 0, "val = 0\nreturn val", CH_PYTHON, 0 );
	getParm("__opParmEval").setLockedFlag( 0, 1 );
}

/// Dtor
SOP_ProceduralHolder::~SOP_ProceduralHolder()
{
}

/// Build type menu
void SOP_ProceduralHolder::buildTypeMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, PRM_Parm * )
{
	SOP_ProceduralHolder *me = reinterpret_cast<SOP_ProceduralHolder*>( data );
	if ( !me )
	{
		return;
	}

	menu[0].setToken( "" );
	menu[0].setLabel( "< No Procedural >" );
	unsigned int pos=1;

	// refresh class list
	me->refreshClassNames();
	const std::vector<std::string> &class_names = me->classNames();
	std::vector<std::string>::const_iterator it;
	for ( it=class_names.begin(); it!=class_names.end(); ++it )
	{
		menu[pos].setToken( (*it).c_str() );
		menu[pos].setLabel( (*it).c_str() );
		++pos;
	}

	// mark the end of our menu
	menu[pos].setToken( 0 );
}

/// Build version menu
void SOP_ProceduralHolder::buildVersionMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, PRM_Parm * )
{
	SOP_ProceduralHolder *me = reinterpret_cast<SOP_ProceduralHolder*>(data);
	if ( !me )
	{
		return;
	}

	unsigned int pos=0;
	if ( me->m_className!="" )
	{
		std::vector<int> class_versions = classVersions( SOP_ParameterisedHolder::PROCEDURAL_LOADER, me->m_className );
		std::vector<int>::iterator it;
		for ( it=class_versions.begin(); it!=class_versions.end(); ++it )
		{
	        std::stringstream ss;
	        ss << (*it);
			menu[pos].setToken( ss.str().c_str() );
			menu[pos].setLabel( ss.str().c_str() );
			++pos;
		}
	}

	if ( pos==0 )
	{
		menu[0].setToken( "" );
		menu[0].setLabel( "< No Version >" );
		++pos;
	}

	// mark the end of our menu
	menu[pos].setToken( 0 );
}

// This refresh the list of class names
void SOP_ProceduralHolder::refreshClassNames()
{
	UT_String matchString;
	evalString( matchString, "__opMatchString", 0, 0 );
	if ( std::string( matchString.buffer() )!=m_matchString )
	{
		m_matchString = std::string( matchString.buffer() );
		m_cachedNames = classNames( SOP_ParameterisedHolder::PROCEDURAL_LOADER, m_matchString );
	}
}

void SOP_ProceduralHolder::setParameterised( IECore::RunTimeTypedPtr p, const std::string &type, int version )
{
	// disable gui update
	disableParameterisedUpdate();

	// set type & version
	m_className = type;
	setString( type.c_str(), CH_STRING_LITERAL, "__opType", 0, 0 );
	m_classVersion = version;
	setString( boost::lexical_cast<std::string>(version).c_str(), CH_STRING_LITERAL, "__opVersion", 0, 0 );

	// set parameterised
	setParameterisedDirectly( p );

	// enable gui update
	enableParameterisedUpdate();
}

/// Callback executed whenever the type/version menus change
int SOP_ProceduralHolder::reloadClassCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	SOP_ProceduralHolder *sop = reinterpret_cast<SOP_ProceduralHolder*>(data);
	if ( !sop )
	{
		return 0;
	}

	UT_String type_str, ver_str;
	sop->evalString( type_str, "__opType", 0, 0 );
	std::string type( type_str.buffer() );
	sop->evalString( ver_str, "__opVersion", 0, 0 );
	int version = -1;
	if ( ver_str!="" )
	{
		version = boost::lexical_cast<int>( ver_str.buffer() );
	}

	// has our type changed?
	if ( type!=sop->m_className )
	{
		sop->m_className = type;
		version = -1;
	}

	// has the version changed?
	if ( version!=sop->m_classVersion )
	{
		sop->m_classVersion = version;
	}

	// if necessary reload and update the interface
	if ( sop->doParameterisedUpdate() )
	{
		sop->m_renderDirty = true; // dirty the scene

		// should we just clear the procedural?
		if ( sop->m_className=="" )
		{
			sop->m_classVersion = -1;
			sop->setParameterised(0, "", -1);
		}
		else
		{
			// if we don't have a version, use the default
			if ( sop->m_classVersion==-1 )
			{
				sop->m_classVersion = defaultClassVersion( SOP_ParameterisedHolder::PROCEDURAL_LOADER, sop->m_className );
				sop->setString( boost::lexical_cast<std::string>(sop->m_classVersion).c_str(), CH_STRING_LITERAL, "__opVersion", 0, 0 );
			}
		}

		// set class/version & refresh gui
		sop->loadProcedural( sop->m_className, sop->m_classVersion );
	}
	return 1;
}

void SOP_ProceduralHolder::loadProcedural( const std::string &type, int version, bool update_gui )
{
	// do we have an existsing procedural?
	IECore::ParameterisedProceduralPtr old_procedural;

	// get our current procedural and save it
	if ( hasParameterised() )
	{
		IECore::ParameterisedProceduralPtr procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( getParameterised() );
		if ( procedural )
		{
			old_procedural = procedural;
		}
	}

	// load & set the procedural
	IECore::RunTimeTypedPtr proc;
	if ( type!="" && version!=-1 )
	{
		proc = loadParameterised( type, version, "IECORE_PROCEDURAL_PATHS" );
	}

	// check our procedural
	if ( proc )
	{
		setParameterised( proc, type, version );
	}
	else
	{
		UT_String msg( "Procedural Holder has no parameterised class to operate on!" );
		addError( SOP_MESSAGE, msg );
	}

	// if required, update the parameter interface on the SOP
	if ( update_gui )
	{
		// update the parameter interface using the FnProceduralHolder.addRemoveParameters()
		// python method from the IECoreHoudini module.
		UT_String my_path;
		getFullPath( my_path );
		std::string cmd = "IECoreHoudini.FnProceduralHolder( hou.node( \"";
		cmd += my_path.buffer();
		cmd += "\") )";
		{
			IECorePython::ScopedGILLock lock;
			try
			{
				handle<> resultHandle( PyRun_String( cmd.c_str(), Py_eval_input,
					CoreHoudini::globalContext().ptr(), CoreHoudini::globalContext().ptr() ) );
				object fn( resultHandle );
				fn.attr("updateParameters")( proc, old_procedural );
			}
			catch( ... )
			{
				PyErr_Print();
			}
		}
	}
}

/// Callback executed whenever the reload button is clicked
int SOP_ProceduralHolder::reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	SOP_ProceduralHolder *sop = reinterpret_cast<SOP_ProceduralHolder*>( data );
	if ( !sop )
	{
		return 0;
	}

	CoreHoudini::evalPython( "IECore.ClassLoader.defaultProceduralLoader().refresh()" );
	sop->loadProcedural( sop->m_className, sop->m_classVersion );
	
	return 1;
}

/// Redraws the OpenGL Scene if the procedural is marked as having changed
/// (aka dirty).
IECoreGL::ConstScenePtr SOP_ProceduralHolder::scene()
{
	IECore::ParameterisedProceduralPtr procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( getParameterised() );
	if ( !procedural )
	{
		return 0;
	}

	if ( m_renderDirty || !m_scene )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
			renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
			renderer->worldBegin();
			procedural->render( renderer );
			renderer->worldEnd();
			m_scene = renderer->scene();
		}
		catch( const std::exception &e )
		{
			std::cerr << e.what() << std::endl;
		}
		catch( ... )
		{
			std::cerr << "Unknown!" << std::endl;
		}

		m_renderDirty = false;
	}
	return m_scene;
}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_ProceduralHolder::cookMySop( OP_Context &context )
{
	// some defaults and useful variables
	float now = context.myTime;

	// force eval of our nodes parameters with our hidden parameter expression
	evalInt( "__opParmEval", 0, now );

	// update parameters on procedural from our Houdini parameters
	IECore::ParameterisedProceduralPtr procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( getParameterised() );

	// check for a valid parameterised on this SOP
	if ( !procedural )
	{
		UT_String msg( "Procedural Holder has no parameterised class to operate on!" );
		addError( SOP_MESSAGE, msg );
		return error();
	}

	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building OpHolder Geometry...");
	gdp->clearAndDestroy();

	// do we need to redraw?
	bool do_update = updateParameters( procedural, now );
	if ( do_update )
	{
		dirty();
	}

	// calculate our bounding box
	IECorePython::ScopedGILLock gilLock;
	try
	{
		// put our cortex passdata on our gdp as a detail attribute
		IECoreHoudini::NodePassData data( this, IECoreHoudini::NodePassData::CORTEX_PROCEDURALHOLDER );
		gdp->addAttrib( "IECoreHoudini::NodePassData", sizeof(IECoreHoudini::NodePassData), GB_ATTRIB_MIXED, &data );

		// calculate our bounding box
		Imath::Box3f bbox = procedural->bound();
		gdp->cube( bbox.min.x, bbox.max.x, bbox.min.y, bbox.max.y, bbox.min.z, bbox.max.z, 0, 0, 0, 1, 1 );
	}
	catch( boost::python::error_already_set )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		std::cerr << "Procedural::bound() " << e.what() << std::endl;
	}
	catch( ... )
	{
		std::cerr << "Procedural::bound() Caught unknown exception!" << std::endl;
	}

	// tidy up & go home!
	boss->opEnd();

	return error();
}

/// This gets called when this SOP is loaded from Disk
/// It checks for type/version values on the node and attempts to reload
/// the procedural from disk
/// \todo: not entirely certain this is returning the correct thing...
bool SOP_ProceduralHolder::load( UT_IStream &is, const char *ext, const char *path )
{
	bool loaded = OP_Node::load( is, ext, path );

	// look at type/version parameters
	UT_String type_str, ver_str;
	evalString( type_str, "__opType", 0, 0 );
	std::string type( type_str.buffer() );
	evalString( ver_str, "__opVersion", 0, 0 );
	m_className = type_str.buffer();
	m_classVersion = -1;
	if ( ver_str!="" )
	{
		m_classVersion = boost::lexical_cast<int>(ver_str.buffer());
	}

	// if we can, set our class & version
	if ( m_className!="" && m_classVersion!=-1 )
	{
		loadProcedural( m_className, m_classVersion, false );
	}
	
	return loaded;
}
