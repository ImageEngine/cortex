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

#include <boost/python.hpp>

#include <OP/OP_Director.h>
#include <OP/OP_Node.h>
#include <HOM/HOM_SopNode.h>
#include <HOM/HOM_NodeType.h>
#include <SOP/SOP_Node.h>

#include <IECore/Object.h>
#include <IECore/Parameterised.h>
#include <IECorePython/PointerFromSWIG.h>

#include "CoreHoudini.h"
#include "FnOpHolderBinding.h"
#include "FnProceduralHolderBinding.h"
#include "FromHoudiniSopConverterBinding.h"

using namespace IECoreHoudini;
using namespace boost::python;

// returns a HOM_SopNode from a hou sop instance
static void *extractHomSopFromHOM( PyObject *o )
{
	if( !PyObject_HasAttrString( o, "this" ) ) // no this? certainly invalid!
		return 0;

	PyObject *thisAttr = PyObject_GetAttrString( o, "this" );
	if( !thisAttr ) // invalid this? Weird but check anyway
		return 0;

	// TODO: here we 'assume' we have a HOM_SopNode object
	// Only use this in controlled circumstances!
	HOM_SopNode *hom_sop = static_cast<HOM_SopNode*>(((IECorePython::Detail::PySwigObject*)thisAttr)->ptr);
	if ( !hom_sop )
		return 0;
	return hom_sop;
}

BOOST_PYTHON_MODULE(_IECoreHoudini)
{
	// setup our global python context
	CoreHoudini::initPython();

	//bindFnOpHolder();
	bindFnProceduralHolder();
	bindFromHoudiniSopConverter();

	// register our HOM SopNode converter function
	boost::python::converter::registry::insert( &extractHomSopFromHOM, boost::python::type_id<HOM_SopNode>() );
}
