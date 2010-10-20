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
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Operator.h>
#include <UT/UT_Math.h>
#include <UT/UT_Interrupt.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <CH/CH_LocalVariable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_Parm.h>
#include <OP/OP_Operator.h>
#include <GU/GU_PrimPart.h>
#include <CH/CH_ExprLanguage.h>
#include <GEO/GEO_PrimPart.h>
#include <GB/GB_AttributeRef.h>


// Cortex
#include <IECore/MessageHandler.h>
#include <IECore/CompoundParameter.h>
#include <IECore/Object.h>
#include <IECore/NullObject.h>
#include "IECore/ObjectParameter.h"
#include <IECore/Op.h>
#include <IECore/TypedParameter.h>
#include <IECore/Parameterised.h>
#include <IECore/CompoundParameter.h>
#include <IECore/ParameterisedProcedural.h>
#include <IECore/SimpleTypedData.h>
#include <IECore/MeshPrimitive.h>
#include <IECorePython/ScopedGILLock.h>

// OpenEXR
#include <OpenEXR/ImathBox.h>

// C++
#include <sstream>

// Boost
#include "boost/python.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"
using namespace boost::python;
using namespace boost;

// IECoreHoudini
#include "CoreHoudini.h"
#include "SOP_OpHolder.h"
#include "NodePassData.h"
#include "FnOpHolder.h"
#include "FromHoudiniGeometryConverter.h"
using namespace IECoreHoudini;

/// Parameter names for non-dynamic SOP parameters
PRM_Name SOP_OpHolder::opTypeParm( "__opType", "Op:" );
PRM_Name SOP_OpHolder::opVersionParm( "__opVersion", "  Version:" );
PRM_Name SOP_OpHolder::opParmEval( "__opParmEval", "ParameterEval" );
PRM_Name SOP_OpHolder::opMatchString( "__opMatchString", "MatchString" );
PRM_Default SOP_OpHolder::opMatchStringDefault( 0.f, "*" );
PRM_Name SOP_OpHolder::opReloadBtn( "__opReloadBtn", "Reload" );
PRM_Name SOP_OpHolder::switcherName( "__switcher", "Switcher" );

PRM_ChoiceList SOP_OpHolder::typeMenu( PRM_CHOICELIST_SINGLE,
		&SOP_OpHolder::buildTypeMenu );
PRM_ChoiceList SOP_OpHolder::versionMenu( PRM_CHOICELIST_SINGLE,
		&SOP_OpHolder::buildVersionMenu );

/// Detail for our switcher
PRM_Default SOP_OpHolder::switcherDefaults[] = {
		PRM_Default(0, "Parameters"),
};

/// Add parameters to SOP
PRM_Template SOP_OpHolder::myParameters[] = {
		PRM_Template(PRM_STRING|PRM_TYPE_JOIN_NEXT, 1, &opTypeParm, 0, &typeMenu, 0, &SOP_OpHolder::reloadClassCallback ),
		PRM_Template(PRM_STRING|PRM_TYPE_JOIN_NEXT , 1, &opVersionParm, 0, &versionMenu, 0, &SOP_OpHolder::reloadClassCallback ),
		PRM_Template(PRM_CALLBACK, 1, &opReloadBtn, 0, 0, 0, &SOP_OpHolder::reloadButtonCallback ),
		PRM_Template(PRM_INT|PRM_TYPE_INVISIBLE, 1, &opParmEval),
		PRM_Template(PRM_STRING|PRM_TYPE_INVISIBLE, 1, &opMatchString, &opMatchStringDefault ),
		PRM_Template(PRM_SWITCHER, 1, &switcherName, switcherDefaults ),
		PRM_Template()
};

/// Don't worry about variables today
CH_LocalVariable SOP_OpHolder::myVariables[] = {
		{ 0, 0, 0 },
};

/// Houdini's static creator method
OP_Node *SOP_OpHolder::myConstructor( OP_Network *net,
										const char *name,
										OP_Operator *op )
{
    return new SOP_OpHolder(net, name, op);
}

/// Ctor
SOP_OpHolder::SOP_OpHolder(OP_Network *net,
		const char *name,
		OP_Operator *op ) :
	SOP_ParameterisedHolder(net, name, op),
    m_renderDirty(true),
    m_parameters(0),
    m_haveParameterList(false)
{
	// evaluation expression parameter
	getParm("__opParmEval").setExpression( 0, "val = 0\nreturn val", CH_PYTHON, 0 );
	getParm("__opParmEval").setLockedFlag( 0, 1 );

	// clear inputs
	m_inputs.clear();
}

/// Dtor
SOP_OpHolder::~SOP_OpHolder()
{
}

/// TODO: move this code into parameterised holder
/// Build type menu
void SOP_OpHolder::buildTypeMenu( void *data, PRM_Name *menu, int maxSize,
		const PRM_SpareData *, PRM_Parm * )
{
	SOP_OpHolder *me = reinterpret_cast<SOP_OpHolder*>(data);
	if ( !me )
		return;

	menu[0].setToken( "" );
	menu[0].setLabel( "< No Op >" );
	unsigned int pos=1;

	// refresh class names
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
	menu[pos].setToken(0);
}

/// TODO: move this code into parameterised holder
/// Build version menu
void SOP_OpHolder::buildVersionMenu( void *data, PRM_Name *menu, int maxSize,
		const PRM_SpareData *, PRM_Parm *)
{
	SOP_OpHolder *me = reinterpret_cast<SOP_OpHolder*>(data);
	if ( !me )
		return;

	unsigned int pos=0;
	if ( me->m_className!="" )
	{
		std::vector<int> class_versions = classVersions( SOP_ParameterisedHolder::OP_LOADER, me->m_className );
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
	menu[pos].setToken(0);
}

/// TODO: Move this code into parameterised holder
void SOP_OpHolder::setParameterised( IECore::RunTimeTypedPtr p, const std::string &type, int version )
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

	// refresh input parameters
	refreshInputConnections();
}

// This refresh the list of class names
void SOP_OpHolder::refreshClassNames()
{
	UT_String matchString;
	evalString( matchString, "__opMatchString", 0, 0 );
	if ( std::string( matchString.buffer() )!=m_matchString )
	{
		m_matchString = std::string( matchString.buffer() );
		m_cachedNames = classNames( SOP_ParameterisedHolder::OP_LOADER, m_matchString );
	}
}

/// TODO: move this code into parameterised holder
/// Callback executed whenever the type/version menus change
int SOP_OpHolder::reloadClassCallback( void *data, int index, float time,
		const PRM_Template *tplate)
{
	SOP_OpHolder *sop = reinterpret_cast<SOP_OpHolder*>(data);
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
		version = boost::lexical_cast<int>( ver_str.buffer() );

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
		sop->m_renderDirty = true; // dirty the op

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
				sop->m_classVersion = defaultClassVersion( SOP_ParameterisedHolder::OP_LOADER, sop->m_className );
				sop->setString( boost::lexical_cast<std::string>(sop->m_classVersion).c_str(), CH_STRING_LITERAL, "__opVersion", 0, 0 );
			}
		}

		// set class/version & refresh gui
		sop->loadOp( sop->m_className, sop->m_classVersion );
	}
	return 1;
}

void SOP_OpHolder::refreshInputConnections()
{
	// clear our internal cache of input parameters
	m_parameters = 0;
	m_inputs.clear();

	IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( getParameterised() );
	if ( !op )
		return;

	// work out which parameters need inputs & make them so :)
	m_parameters = op->parameters();
	for ( IECore::CompoundParameter::ParameterVector::const_iterator it=m_parameters->orderedParameters().begin();
			it!=m_parameters->orderedParameters().end(); ++it )
	{
		switch( (*it)->typeId() )
		{
			case IECore::ObjectParameterTypeId:
			case IECore::PrimitiveParameterTypeId:
			case IECore::PointsPrimitiveParameterTypeId:
			case IECore::MeshPrimitiveParameterTypeId:
				m_inputs.push_back( (*it) );
				break;
			default:
				break;
		}

		// \todo: get proper warning in here...
		if ( m_inputs.size()>4 )
		{
			std::cerr << "Cortex Op Holder cannot support more than 4 input parameter connections." << std::endl;
		}
	}
	m_haveParameterList = true;

	//=====
	// \todo: Geesh, is this really the only we can get the gui to update the input connections?!?
	setXY( getX()+.0001, getY()+.0001 );
	setXY( getX()-.0001, getY()-.0001 );
	//=====
}

/// TODO: move this code into parameterised holder
void SOP_OpHolder::loadOp( const std::string &type, int version, bool update_gui )
{
	// do we have an existsing procedural?
	IECore::OpPtr old_op;

	// get our current procedural and save it
	if ( hasParameterised() )
	{
		IECore::OpPtr op =
				IECore::runTimeCast<IECore::Op>(
						getParameterised() );
		if ( op )
		{
			old_op = op;
		}
	}

	// load & set the procedural
	IECore::RunTimeTypedPtr proc;
	if ( type!="" && version!=-1 )
	{
		proc = loadParameterised( type, version, "IECORE_OP_PATHS" );
	}

	// check our procedural
	if ( proc )
	{
		// set out parameterised on our opholder
		setParameterised( proc, type, version );
	}
	else
	{
		m_parameters = 0;
		m_inputs.clear();
		m_haveParameterList = false;
		UT_String msg( "Op Holder has no parameterised class to operate on!" );
    	addError( SOP_MESSAGE, msg );
	}

	// if required, update the parameter interface on the SOP
	if ( update_gui )
	{
		// update the parameter interface using the FnProceduralHolder.addRemoveParameters()
		// python method from the IECoreHoudini module.
		UT_String my_path;
		getFullPath( my_path );
		std::string cmd = "IECoreHoudini.FnOpHolder( hou.node( \"";
		cmd += my_path.buffer();
		cmd += "\") )";
		{
			IECorePython::ScopedGILLock lock;
			try
			{
				handle<> resultHandle( PyRun_String( cmd.c_str(), Py_eval_input,
					CoreHoudini::globalContext().ptr(), CoreHoudini::globalContext().ptr() ) );
				object fn( resultHandle );
				fn.attr("updateParameters")( proc, old_op );
			}
			catch( ... )
			{
				PyErr_Print();
			}
		}
	}
}

/// TODO: move this code into parameterised holder
/// Callback executed whenever the reload button is clicked
int SOP_OpHolder::reloadButtonCallback( void *data, int index, float time,
		const PRM_Template *tplate)
{
	SOP_OpHolder *sop = reinterpret_cast<SOP_OpHolder*>(data);
	if ( !sop )
	{
		return 0;
	}

	CoreHoudini::evalPython( "IECore.ClassLoader.defaultOpLoader().refresh()" );
	sop->loadOp( sop->m_className, sop->m_classVersion );

	return 1;
}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_OpHolder::cookMySop(OP_Context &context)
{
	// some defaults and useful variables
    Imath::Box3f bbox( Imath::V3f(-1,-1,-1), Imath::V3f(1,1,1) );
	float now = context.myTime;

	// force eval of our nodes parameters with our hidden parameter expression
	evalInt( "__opParmEval", 0, now );

	// get our op
	IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( getParameterised() );

	// check for a valid parameterised on this SOP
    if ( !op )
    {
    	UT_String msg( "Op Holder has no parameterised class to operate on!" );
    	addError( SOP_MESSAGE, msg );
    	return error();
    }

    if( lockInputs(context)>=UT_ERROR_ABORT )
    	return error();

	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building OpHolder Geometry...");
	gdp->clearAndDestroy();

	// loop through inputs getting the upstream geometry detail & putting it into our Op's parameters
	for ( unsigned int i=0; i<m_inputs.size(); ++i )
	{
		IECore::ParameterPtr input_parameter = m_inputs[i];
		GU_DetailHandle gdp_handle = inputGeoHandle(i);
		const GU_Detail *input_gdp = gdp_handle.readLock();

		if ( input_gdp )
		{
			if ( input_gdp->attribs().find("IECoreHoudini::NodePassData", GB_ATTRIB_MIXED) ) // looks like data passed from another OpHolder
			{
				GB_AttributeRef attrOffset = input_gdp->attribs().getOffset( "IECoreHoudini::NodePassData", GB_ATTRIB_MIXED );
				const NodePassData *pass_data = input_gdp->attribs().castAttribData<NodePassData>( attrOffset );
				if ( pass_data->type()==IECoreHoudini::NodePassData::CORTEX_OPHOLDER )
				{
					SOP_OpHolder *sop = dynamic_cast<SOP_OpHolder*>(const_cast<OP_Node*>(pass_data->nodePtr()));
					IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( sop->getParameterised() );
					const IECore::Parameter *result_parameter = op->resultParameter();
					const IECore::ConstObjectPtr result_object(result_parameter->getValue());

					try
					{
						input_parameter->setValidatedValue( IECore::constPointerCast<IECore::Object>( result_object ));
					}
					catch (const IECore::Exception &e)
					{
						addError( SOP_MESSAGE, e.what() );
					}
				}
			}
			else // otherwise looks like a regular Houdini detail
			{
				IECore::ObjectPtr converted;
				try
				{
					IECore::ObjectParameterPtr objectParameter = IECore::runTimeCast<IECore::ObjectParameter>( input_parameter );
					if ( objectParameter )
					{
						FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( gdp_handle, objectParameter->validTypes() );
						if ( converter )
						{
							converted = converter->convert();
						}
					}
				}
				catch (std::runtime_error &e)
				{
					addError( SOP_MESSAGE, e.what() );
				}

				if ( converted )
				{
					try
					{
						input_parameter->setValidatedValue( converted );
					}
					catch (const IECore::Exception &e)
					{
						addError( SOP_MESSAGE, e.what() );
					}
				}
			}
		}
		
		gdp_handle.unlock( input_gdp );
	}

	// update parameters from our op & flag as dirty if necessary
	bool req_update = updateParameters( op, now);
	if ( req_update )
	{
		dirty();
	}

	// if we have an op, make it do itself
	try
	{
		// make our Cortex op do it's thing...
		op->operate();

		// pass ourselves onto the GR_Cortex render hook
		IECoreHoudini::NodePassData data( this, IECoreHoudini::NodePassData::CORTEX_OPHOLDER );
		gdp->addAttrib( "IECoreHoudini::NodePassData",
				sizeof(IECoreHoudini::NodePassData), GB_ATTRIB_MIXED, &data );

		// if our result is a visible renderable then set our bounds on our output gdp
		const IECore::Object *result = op->resultParameter()->getValue();
		IECore::ConstVisibleRenderablePtr renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( result );
		if ( renderable )
		{
			Imath::Box3f bbox = renderable->bound();
			gdp->cube( bbox.min.x, bbox.max.x, bbox.min.y, bbox.max.y,
					bbox.min.z, bbox.max.z, 0, 0, 0, 1, 1 );
		}
	}
	catch( boost::python::error_already_set )
	{
	    addError( SOP_MESSAGE, "Error raised during Python evaluation!" );
	    IECorePython::ScopedGILLock lock;
		PyErr_Print();
	}
	catch( const IECore::Exception &e )
	{
		addError( SOP_MESSAGE, e.what() );
	}
	catch( const std::exception &e )
	{
		addError( SOP_MESSAGE, e.what() );
	}
	catch( ... )
	{
		std::cerr << "Caught unknown exception!" << std::endl;
	}

	// tidy up & go home!
	boss->opEnd();
	unlockInputs();
	return error();
}

/// This gets called when this SOP is loaded from Disk
/// It checks for type/version values on the node and attempts to reload
/// the procedural from disk
/// \todo: not entirely certain this is returning the correct thing...
bool SOP_OpHolder::load( UT_IStream &is,
		const char *ext,
		const char *path )
{
	m_haveParameterList = false;
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
		loadOp( m_className, m_classVersion, false );
	}

	return loaded;
}

const char *SOP_OpHolder::inputLabel( unsigned pos ) const
{
	if ( !m_parameters || pos>m_inputs.size()-1 )
	{
		return "";
	}
	const IECore::CompoundParameter::ParameterVector &params = m_parameters->orderedParameters();
	if ( pos>params.size()-1 ) // shouldn't happen
	{
		return "";
	}
	return params[pos]->name().c_str();
}

unsigned SOP_OpHolder::minInputs() const
{
	// TODO: need to check for 'required' inputs and increase this number accordingly
	return 0;
}

unsigned SOP_OpHolder::maxInputs() const
{
	// this makes sure when we first load we have 4 inputs
	// the wires get connected before the Op is loaded onto the Sop
	// so
	if ( !m_haveParameterList || m_inputs.size()>4 )
	{
		return 4;
	}
	return m_inputs.size();
}
