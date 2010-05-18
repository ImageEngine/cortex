//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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
#include <HOM/HOM_Module.h>
#include <GEO/GEO_PrimPart.h>

// Cortex
#include <IECore/MessageHandler.h>
#include <IECore/CompoundParameter.h>
#include <IECore/Object.h>
#include <IECore/Op.h>
#include <IECore/TypedParameter.h>
#include <IECore/Parameterised.h>
#include <IECore/CompoundParameter.h>
#include <IECore/ParameterisedProcedural.h>
#include <IECore/SimpleTypedData.h>
#include <IECorePython/ScopedGILLock.h>

// OpenEXR
#include <OpenEXR/ImathBox.h>

// Boost
#include "boost/python.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"
using namespace boost::python;
using namespace boost;

// IECoreHoudini
#include "CoreHoudini.h"
#include "SOP_ProceduralHolder.h"
#include "FnProceduralHolder.h"
using namespace IECoreHoudini;

/// Parameter names for non-dynamic SOP parameters
PRM_Name SOP_ProceduralHolder::opTypeParm( "__opType", "Op Type:" );
PRM_Name SOP_ProceduralHolder::opVersionParm( "__opVersion", "Op Version:" );
PRM_Name SOP_ProceduralHolder::opParmEval( "__opParmEval", "ParameterEval" );
PRM_Name SOP_ProceduralHolder::switcherName( "__switcher", "Switcher" );

/// Detail for our switcher
PRM_Default SOP_ProceduralHolder::switcherDefaults[] = {
		PRM_Default(3, "Class"),
		PRM_Default(0, "Parameters"),
};

/// Add parameters to SOP
PRM_Template SOP_ProceduralHolder::myParameters[] = {
		PRM_Template(PRM_SWITCHER, 2, &switcherName, switcherDefaults ),
		PRM_Template(PRM_STRING, 1, &opTypeParm),
		PRM_Template(PRM_STRING, 1, &opVersionParm),
		PRM_Template(PRM_INT, 1, &opParmEval),
        PRM_Template()
};

/// Don't worry about variables today
CH_LocalVariable SOP_ProceduralHolder::myVariables[] = {
		{ 0, 0, 0 },
};

/// Houdini's static creator method
OP_Node *SOP_ProceduralHolder::myConstructor( OP_Network *net,
										const char *name,
										OP_Operator *op )
{
    return new SOP_ProceduralHolder(net, name, op);
}

/// Ctor
SOP_ProceduralHolder::SOP_ProceduralHolder(OP_Network *net,
		const char *name,
		OP_Operator *op ) :
	SOP_ParameterisedHolder(net, name, op),
	mp_detail( new GU_ProceduralDetail )
{
	setString( "", CH_STRING_LITERAL, "__opType", 0, 0 );
	setString( "", CH_STRING_LITERAL, "__opVersion", 0, 0 );
	getParm("__opType").setLockedFlag( 0, 1 );
	getParm("__opVersion").setLockedFlag( 0, 1 );
	getParm("__opParmEval").getTemplatePtr()->setInvisible(true);
	getParm("__opParmEval").setExpression( 0, "val = 0\nreturn val", CH_PYTHON, 0 );
	getParm("__opParmEval").setLockedFlag( 0, 1 );

	// set our new detail on our gdphandle
	if ( myGdpHandle.deleteGdp() )
		myGdpHandle.allocateAndSet( mp_detail, false );
}

/// Dtor
SOP_ProceduralHolder::~SOP_ProceduralHolder()
{
}

///
//unsigned SOP_ProceduralHolder::needToCook( OP_Context &context, bool queryOnly )
//{
//	return true;
//}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_ProceduralHolder::cookMySop(OP_Context &context)
{
	// some defaults and useful variables
    Imath::Box3f bbox( Imath::V3f(-1,-1,-1), Imath::V3f(1,1,1) );
	float now = context.myTime;

    // lock our inputs - not used currently
	/*
    if ( lockInputs(context) >= UT_ERROR_ABORT)
    	error();
	*/

    // force eval of our nodes parameters with our hidden parameter expression
    int parm_eval_result = evalInt( "__opParmEval", 0, now );

    // check for a valid parameterised on this SOP
    if ( !hasParameterised() )
    {
    	UT_String msg( "Procedural Holder has no parameterised class to operate on!" );
    	addError( SOP_MESSAGE, msg );
    	return error();
    }

    // ensure we have a valid GU_ProceduralDetail on this SOP
	GU_ProceduralDetail *proc_gdp = dynamic_cast<GU_ProceduralDetail *>(gdp);
	if ( !proc_gdp )
	{
		myGdpHandle.unlock( gdp );
		proc_gdp = mp_detail;
		GU_Detail *old_handle = myGdpHandle.setGdp( proc_gdp );
		if ( old_handle == proc_gdp )
        {
        	UT_String msg( "SERIOUS ERROR! Could not set "
        			"IECoreHoudini::ProceduralDetail on this SOP!" );
        	addError( SOP_MESSAGE, msg );
        	return error();
        }
		myGdpHandle.writeLock();
	}

	// Here we create a new ProceduralPrimitive and attach it to our detail.
	IECore::ParameterisedProceduralPtr procedural =
			IECore::runTimeCast<IECore::ParameterisedProcedural>(
					getParameterised() );
	proc_gdp->m_procedural = procedural;

	// check we got a parameterised procedural
	if ( !procedural )
	{
		UT_String msg( "IECore::ParameterisedPtr is not a valid "
				"ParameterisedProcedural!\n" );
		addError( SOP_MESSAGE, msg );
		return error();
	}

	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building OpHolder Geometry...");
	proc_gdp->clearAndDestroy();

	// update parameters on procedural from our Houdini parameters
	bool do_update = updateParameters( procedural, now);
	if ( do_update )
		proc_gdp->dirty();

	// calculate our bounding box
	IECorePython::ScopedGILLock gilLock;
	try
	{
		object pythonProcedural( procedural );
		bbox = extract<Imath::Box3f>( pythonProcedural.attr("bound")() );
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
		std::cerr << "Procedural::bound() Caught unknown exception!"
			<< std::endl;
	}

	// put a box in our gdp to represent our bounds
	proc_gdp->cube( bbox.min.x, bbox.max.x, bbox.min.y, bbox.max.y,
			bbox.min.z, bbox.max.z, 0, 0, 0, 1, 1 );

	// tidy up & go home!
	boss->opEnd();

	return error();
}

/// This gets called when this SOP is loaded from Disk
/// It checks for type/version values on the node and attempts to reload
/// the procedural from disk
bool SOP_ProceduralHolder::load( UT_IStream &is,
		const char *ext,
		const char *path )
{
	OP_Node::load( is, ext, path );

	// if type && version != ""
	//    create new procedural from type/version
	//    set parameterised with new procedural
	UT_String opType, opVersion;
	evalString( opType, "__opType", 0, 0 );
	evalString( opVersion, "__opVersion", 0, 0 );
	if ( opType!="" && opVersion!="" )
	{
		std::string type( opType.buffer() );
		int version = boost::lexical_cast<int>(std::string(opVersion.buffer()));

		IECore::RunTimeTypedPtr proc = loadParameterised( type, version );
		if ( proc )
		{
			FnProceduralHolder fn;
			fn.setParameterisedDirectly( proc, type, version, this );
		}
	}
}

/// Utility class which loads a ParameterisedProcedural from Disk
/// TODO - This should probably be moved to FnParameterisedHolder?
IECore::RunTimeTypedPtr SOP_ProceduralHolder::loadParameterised(
		const std::string &type,
		int version,
		const std::string &search_path )
{
	IECore::RunTimeTypedPtr new_procedural;
	IECorePython::ScopedGILLock gilLock;

	string python_cmd = boost::str( format(
			"IECore.ClassLoader.defaultLoader( \"%s\" ).load( \"%s\", %d )()\n"
		) % search_path % type % version );

	try
	{
		boost::python::handle<> resultHandle( PyRun_String(
			python_cmd.c_str(),
			Py_eval_input, CoreHoudini::globalContext().ptr(),
			CoreHoudini::globalContext().ptr() )
		);
		boost::python::object result( resultHandle );
		new_procedural =
				boost::python::extract<IECore::RunTimeTypedPtr>(result)();
	}
	catch( ... )
	{
		PyErr_Print();
	}
	return new_procedural;
}
