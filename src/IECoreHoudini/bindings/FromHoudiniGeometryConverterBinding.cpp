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

#include "HOM/HOM_Geometry.h"
#include "HOM/HOM_GUDetailHandle.h"

#include "IECore/Exception.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreHoudini/FromHoudiniGeometryConverter.h"
#include "IECoreHoudini/bindings/FromHoudiniGeometryConverterBinding.h"

using namespace boost::python;
using namespace IECoreHoudini;

static list supportedTypes()
{
	list result;
	std::set<IECore::TypeId> e;
	FromHoudiniGeometryConverter::supportedTypes( e );
	for ( std::set<IECore::TypeId>::const_iterator it = e.begin(); it != e.end(); ++it )
	{
		result.append( *it );
	}
	
	return result;
}

static FromHoudiniGeometryConverterPtr createFromGeo( HOM_Geometry *homGeo, IECore::TypeId resultType )
{
	// this HOM manipulation was provided by SideFx, with a warning
	// that it is safe but not really meant for HDK developers
	HOM_GUDetailHandle *gu_handle = homGeo->_guDetailHandle();
	GU_Detail *geo = (GU_Detail *)gu_handle->_asVoidPointer();
	
	GU_DetailHandle handle;
	handle.allocateAndSet( geo, false );
	
	return FromHoudiniGeometryConverter::create( handle, resultType );
}

static FromHoudiniGeometryConverterPtr createDummy( object ids )
{
	extract<IECore::TypeId> ex( ids );
	if( ex.check() )
	{
		return FromHoudiniGeometryConverter::create( GU_DetailHandle(), ex() );
	}
	
	std::set<IECore::TypeId> resultTypes;
	for ( long i = 0; i < IECorePython::len( ids ); i++ )
	{
		extract<IECore::TypeId> ex( ids[i] );
		if ( !ex.check() )
		{
			throw IECore::InvalidArgumentException( "FromHoudiniGeometryConverter.supportedTypes: List element is not an IECore.TypeId" );
		}
		
		resultTypes.insert( ex() );
	}
	
	return FromHoudiniGeometryConverter::create( GU_DetailHandle(), resultTypes );
}

void IECoreHoudini::bindFromHoudiniGeometryConverter()
{
	IECorePython::RunTimeTypedClass< FromHoudiniGeometryConverter >()
		.def( "create", (FromHoudiniGeometryConverterPtr (*)( const SOP_Node *, const std::string &, IECore::TypeId ) )&FromHoudiniGeometryConverter::create, ( arg_( "sop" ), arg_( "nameFilter" ) = "", arg_( "resultType" ) = IECore::InvalidTypeId ) ).staticmethod( "create" )
		.def( "createFromGeo", &createFromGeo, ( arg_( "geo" ), arg_( "resultType" ) = IECore::InvalidTypeId ) ).staticmethod( "createFromGeo" )
		.def( "createDummy", &createDummy, ( arg_( "resultTypes" ) ) ).staticmethod( "createDummy" )
		.def( "supportedTypes", &supportedTypes )
		.staticmethod( "supportedTypes" )
	;
}
