//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "CH/CH_Manager.h"
#include "SOP/SOP_Node.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/bindings/ToHoudiniGeometryConverterBinding.h"

using namespace boost::python;
using namespace IECoreHoudini;

/// Extracts the GU_DetailHandle from a SOP_Node and converts it. If append is true,
/// the conversion will append to the existing GU_Detail. If append is false, the GU_Detail
/// will be cleared before conversion.
static bool convert( ToHoudiniGeometryConverter &c, SOP_Node *sop, bool append )
{
	// create the work context
	OP_Context context;
	context.setTime( CHgetEvalTime() );
	
	GU_DetailHandle handle = sop->getCookedGeoHandle( context );
	
	if ( !append )
	{
		GU_DetailHandleAutoWriteLock writeHandle( handle );	
		GU_Detail *geo = writeHandle.getGdp();
		if ( !geo )
		{
			return false;
		}
		
		geo->clearAndDestroy();
	}
	
	if ( c.convert( handle ) )
	{
		sop->setModelLock( true );
		return true;
	}
	
	return false;
}

void IECoreHoudini::bindToHoudiniGeometryConverter()
{
	IECorePython::RunTimeTypedClass<ToHoudiniGeometryConverter>()
		.def( "convert", &convert, ( arg_( "self" ), arg_( "sop" ), arg_( "append" ) = false ),
			"Extracts the GU_DetailHandle from a SOP_Node and converts it. The append flag defaults to False, which will clear the GU_Detail before conversion. If append is True, the conversion will append to the existing GU_Detail instead." )
		.def( "create", &ToHoudiniGeometryConverter::create ).staticmethod( "create" )
	;
}
