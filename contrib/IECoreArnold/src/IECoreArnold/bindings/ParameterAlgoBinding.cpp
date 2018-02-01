//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECoreArnold/bindings/ParameterAlgoBinding.h"

#include "IECoreArnold/ParameterAlgo.h"

using namespace boost::python;
using namespace IECoreArnold;
using namespace IECoreArnoldBindings;

namespace
{

AtNode *atNodeFromPythonObject( object &o )
{
	object ctypes = import( "ctypes" );
	object ctypesPointer = ctypes.attr( "POINTER" );
	object arnoldAtNode = import( "arnold" ).attr( "AtNode" );
	object atNodePtrType = ctypesPointer( arnoldAtNode );

	if( !PyObject_IsInstance( o.ptr(), atNodePtrType.ptr() ) )
	{
		PyErr_SetString( PyExc_TypeError, "Expected an AtNode" );
		throw_error_already_set();
	}

	object oContents = o.attr( "contents" );
	object pythonAddress = ctypes.attr( "addressof" )( oContents );
	const size_t address = extract<size_t>( pythonAddress );
	return reinterpret_cast<AtNode *>( address );
}

void setParameter( object &pythonNode, const char *name, const IECore::Data *data )
{
	AtNode *node = atNodeFromPythonObject( pythonNode );
	ParameterAlgo::setParameter( node, name, data );
}

IECore::DataPtr getParameter( object &pythonNode, const char *name )
{
	AtNode *node = atNodeFromPythonObject( pythonNode );
	return ParameterAlgo::getParameter( node, name );
}

} // namespace

namespace IECoreArnoldBindings
{

void bindParameterAlgo()
{

	object parameterAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreArnold.ParameterAlgo" ) ) ) );
	scope().attr( "ParameterAlgo" ) = parameterAlgoModule;
	scope parameterAlgoModuleScope( parameterAlgoModule );

	def( "setParameter", &setParameter );
	def( "getParameter", &getParameter );

}

} // namespace IECoreArnoldBindings
