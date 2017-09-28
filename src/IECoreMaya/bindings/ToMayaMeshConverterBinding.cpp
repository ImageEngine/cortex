//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "maya/MObject.h"
#include "maya/MSelectionList.h"

#include "boost/python.hpp"

#include "IECore/MessageHandler.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/ToMayaMeshConverter.h"
#include "IECoreMaya/bindings/ToMayaMeshConverterBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

// we use the shape name instead of MObject
static bool setMeshInterpolationAttribute( std::string n, std::string interpolation )
{
	MSelectionList l;
	StatusException::throwIfError( l.add( MString( n.c_str() ) ) );

	MStatus s;
	MObject o;

	s = l.getDependNode( 0, o );
	if( !s )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::setMeshInterpolationAttribute",  "Could not get depedency node: " + n );
		return false;
	}

	if ( !ToMayaMeshConverter::setMeshInterpolationAttribute( o, interpolation ) )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::setMeshInterpolationAttribute",  "Failed to set interpolation attribute in " + n );
		return false;
	}

	return true;
}

void IECoreMaya::bindToMayaMeshConverter()
{
	IECorePython::RunTimeTypedClass<ToMayaMeshConverter>()
		.def( init<IECore::ConstObjectPtr>() )
		.def( "setMeshInterpolationAttribute", setMeshInterpolationAttribute,  ( arg_( "nodeName" ), arg_( "interpolation" ) = "linear" )  ).staticmethod( "setMeshInterpolationAttribute" )
	;
}
