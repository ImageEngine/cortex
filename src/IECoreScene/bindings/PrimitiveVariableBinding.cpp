//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "PrimitiveVariableBinding.h"

#include "IECoreScene/PrimitiveVariable.h"

using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;

namespace
{

static DataPtr dataGetter( PrimitiveVariable &p )
{
	return p.data;
}

static void dataSetter( PrimitiveVariable &p, DataPtr d )
{
	p.data = d;
}

static IntVectorDataPtr indicesGetter( PrimitiveVariable &p )
{
	return p.indices;
}

static void indicesSetter( PrimitiveVariable &p, IntVectorDataPtr i )
{
	p.indices = i;
}

} // namespace

namespace IECoreSceneModule
{

void bindPrimitiveVariable()
{

	scope varScope = class_<PrimitiveVariable>( "PrimitiveVariable", no_init )
		.def( init<PrimitiveVariable::Interpolation, DataPtr>() )
		.def( init<PrimitiveVariable::Interpolation, DataPtr, IntVectorDataPtr>() )
		.def( init<const PrimitiveVariable &>() )
		.def( init<const PrimitiveVariable &, bool>() )
		.def_readwrite( "interpolation", &PrimitiveVariable::interpolation )
		.add_property( "data", &dataGetter, &dataSetter )
		.add_property( "indices", &indicesGetter, &indicesSetter )
		.def( "expandedData", &PrimitiveVariable::expandedData )
		.def( self == self )
		.def( self != self )
	;
	enum_<PrimitiveVariable::Interpolation>( "Interpolation" )
		.value( "Invalid", PrimitiveVariable::Invalid )
		.value( "Constant", PrimitiveVariable::Constant )
		.value( "Uniform", PrimitiveVariable::Uniform )
		.value( "Vertex", PrimitiveVariable::Vertex )
		.value( "Varying", PrimitiveVariable::Varying )
		.value( "FaceVarying", PrimitiveVariable::FaceVarying )
	;

}

}
