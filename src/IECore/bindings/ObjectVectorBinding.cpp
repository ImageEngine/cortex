//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/ObjectVector.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/ObjectVectorBinding.h"

using namespace boost::python;

namespace IECore
{

static std::vector<ObjectVector>::size_type convertIndex( ObjectVector &o, long index )
{
	long s = o.members().size();

	if( index < 0 )
	{
		index += s;
	}

	if( index >= s || index < 0 )
	{
		PyErr_SetString( PyExc_IndexError, "Index out of range" );
		throw_error_already_set();
	}

	return index;
}

static size_t len( ObjectVector &o )
{
	return o.members().size();
}

static ObjectPtr getItem( ObjectVector &o, long index )
{
	return o.members()[ convertIndex( o, index ) ];
}

static void setItem( ObjectVector &o, long index, ObjectPtr value )
{
	o.members()[convertIndex( o, index )] = value;
}

static void delItem( ObjectVector &o, long index )
{
	o.members().erase( o.members().begin() + index );
}

static void append( ObjectVector &o, ObjectPtr value )
{
	o.members().push_back( value );
}

void bindObjectVector()
{
	RunTimeTypedClass<ObjectVector>()
		.def( init<>() )
		.def( "__len__", &len )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "append", &append )
	;
}

} // namespace IECore
