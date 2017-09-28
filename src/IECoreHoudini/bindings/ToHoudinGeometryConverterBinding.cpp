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
#include "HOM/HOM_Geometry.h"
#include "HOM/HOM_GUDetailHandle.h"
#include "SOP/SOP_Node.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/bindings/ToHoudiniGeometryConverterBinding.h"

using namespace boost::python;
using namespace IECoreHoudini;

static bool convert( ToHoudiniGeometryConverter &c, GU_DetailHandle &handle, bool append )
{
	if ( handle.isNull() )
	{
		return false;
	}

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

	return c.convert( handle );
}

/// Extracts the GU_DetailHandle from a SOP_Node and converts it. If append is true,
/// the conversion will append to the existing GU_Detail. If append is false, the GU_Detail
/// will be cleared before conversion.
static bool convertToSop( ToHoudiniGeometryConverter &c, SOP_Node *sop, bool append )
{
	// create the work context
	OP_Context context;
	context.setTime( CHgetEvalTime() );

	GU_DetailHandle handle = sop->getCookedGeoHandle( context );

	if ( convert( c, handle, append ) )
	{
		sop->setModelLock( true );
		return true;
	}

	return false;
}

/// Extracts the GU_DetailHandle from the HOM_Geometry and converts it. If append is true,
/// the conversion will append to the existing GU_Detail. If append is false, the GU_Detail
/// will be cleared before conversion.
static bool convertToGeo( ToHoudiniGeometryConverter &c, HOM_Geometry *homGeo, bool append )
{
	if ( !homGeo )
	{
		return false;
	}

	// this HOM manipulation was provided by SideFx, with a warning
	// that it is safe but not really meant for HDK developers
	HOM_GUDetailHandle *gu_handle = homGeo->_guDetailHandle();
	if ( gu_handle->isReadOnly() )
	{
		return false;
	}

	GU_Detail *geo = (GU_Detail *)gu_handle->_asVoidPointer();

	GU_DetailHandle handle;
	handle.allocateAndSet( geo, false );

	return convert( c, handle, append );
}

static list supportedTypes()
{
	list result;
	std::set<IECore::TypeId> e;
	ToHoudiniGeometryConverter::supportedTypes( e );
	for ( std::set<IECore::TypeId>::const_iterator it = e.begin(); it != e.end(); ++it )
	{
		result.append( *it );
	}

	return result;
}

void IECoreHoudini::bindToHoudiniGeometryConverter()
{
	IECorePython::RunTimeTypedClass<ToHoudiniGeometryConverter>()
		.def( "convert", &convertToSop, ( arg_( "self" ), arg_( "sop" ), arg_( "append" ) = false ),
			"Extracts the GU_DetailHandle from a SOP_Node and converts it. The append flag defaults to False, which will clear the GU_Detail before conversion. If append is True, the conversion will append to the existing GU_Detail instead." )
		.def( "convertToGeo", &convertToGeo, ( arg_( "self" ), arg_( "geo" ), arg_( "append" ) = false ),
			"Extracts the GU_Detail from a hou.Geometry object and converts it. The append flag defaults to False, which will clear the GU_Detail before conversion. If append is True, the conversion will append to the existing GU_Detail instead." )
		.def( "create", &ToHoudiniGeometryConverter::create ).staticmethod( "create" )
		.def( "supportedTypes", &supportedTypes )
		.staticmethod( "supportedTypes" )
	;
}
