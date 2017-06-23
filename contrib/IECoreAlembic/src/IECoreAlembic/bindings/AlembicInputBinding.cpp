//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECoreAlembic/AlembicInput.h"
#include "IECoreAlembic/bindings/AlembicInputBinding.h"

#include "IECorePython/RefCountedBinding.h"

using namespace boost::python;
using namespace IECoreAlembic;

static AlembicInputPtr getItem( AlembicInput *a, long index )
{
	long s = a->numChildren();

	if( index < 0 )
	{
		index += s;
	}

	if( index >= s || index < 0 )
	{
		PyErr_SetString( PyExc_IndexError, "AlembicInput index out of range" );
		throw_error_already_set();
	}

	return a->child( index );
}

static tuple sampleIntervalAtTime( AlembicInput *a, double time )
{
	size_t f, c;
	double l = a->sampleIntervalAtTime( time, f, c );
	return make_tuple( l, f, c );
}

void IECoreAlembicBindings::bindAlembicInput()
{
	IECorePython::RefCountedClass<AlembicInput, IECore::RefCounted>( "AlembicInput" )
		.def( init<const std::string &>() )
		.def( "name", &AlembicInput::name, return_value_policy<copy_const_reference>() )
		.def( "fullName", &AlembicInput::fullName, return_value_policy<copy_const_reference>() )
		.def( "metaData", &AlembicInput::metaData )
		.def( "numSamples", &AlembicInput::numSamples )
		.def( "timeAtSample", &AlembicInput::timeAtSample )
		.def( "sampleIntervalAtTime", &sampleIntervalAtTime )
		.def( "hasStoredBound", &AlembicInput::hasStoredBound )
		.def( "boundAtSample", &AlembicInput::boundAtSample, ( boost::python::arg_( "sampleIndex" ) = 0 ) )
		.def( "boundAtTime", &AlembicInput::boundAtTime )
		.def( "transformAtSample", &AlembicInput::transformAtSample, ( boost::python::arg_( "sampleIndex" ) = 0 ) )
		.def( "transformAtTime", &AlembicInput::transformAtTime, ( boost::python::arg_( "time" ) ) )
		.def( "objectAtSample", &AlembicInput::objectAtSample, ( boost::python::arg_( "sampleIndex" ) = 0, boost::python::arg_( "resultType" ) = IECore::ObjectTypeId ) )
		.def( "objectAtTime", &AlembicInput::objectAtTime, ( boost::python::arg_( "time" ), boost::python::arg_( "resultType" ) = IECore::ObjectTypeId ) )
		.def( "numChildren", &AlembicInput::numChildren )
		.def( "child", &getItem )
		.def( "__len__", &AlembicInput::numChildren )
		.def( "__getitem__", &getItem )
		.def( "childNames", &AlembicInput::childNames )
		.def( "child", (AlembicInputPtr (AlembicInput::*)( const std::string & ) const )&AlembicInput::child )
	;
}
