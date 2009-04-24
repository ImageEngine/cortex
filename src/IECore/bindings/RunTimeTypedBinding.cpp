//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/RunTimeTyped.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"

using namespace boost::python;

namespace IECore
{

static list baseTypeIds( TypeId typeId )
{
	list result;
	
	const std::vector<TypeId> &ids = RunTimeTyped::baseTypeIds( typeId );
	for( std::vector<TypeId>::const_iterator it = ids.begin(); it != ids.end(); ++it )
	{
		result.append( *it );
	}
	
	return result;
}

static list derivedTypeIds( TypeId typeId )
{
	list result;
	const std::set<TypeId> &ids = RunTimeTyped::derivedTypeIds( typeId );
	for( std::set<TypeId>::const_iterator it = ids.begin(); it != ids.end(); ++it )
	{
		result.append( *it );
	}
	
	return result;
}

void bindRunTimeTyped()
{
	typedef class_<RunTimeTyped, boost::noncopyable, RunTimeTypedPtr, bases<RefCounted> > RunTimeTypedPyClass;
	RunTimeTypedPyClass( "RunTimeTyped", no_init )
		.def( "typeName", &RunTimeTyped::typeName )
		.def( "typeId", &RunTimeTyped::typeId )
		.def( "isInstanceOf", (bool (RunTimeTyped::*)( const char * ) const)&RunTimeTyped::isInstanceOf )
		.def( "isInstanceOf", (bool (RunTimeTyped::*)( TypeId ) const)&RunTimeTyped::isInstanceOf )
		
		// Not defined as staticmethod as IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS does this after
		// definition of overloaded baseTypeId(TypeId) function (staticmethod must be called after
		// all the other definitions of that function)
		.def("baseTypeId", ( TypeId (*)( TypeId ) )( &RunTimeTyped::baseTypeId ) )
		.def( "baseTypeIds", &baseTypeIds ).staticmethod( "baseTypeIds" )
		.def( "derivedTypeIds", &derivedTypeIds ).staticmethod( "derivedTypeIds" )
		.def( "typeIdFromTypeName", &RunTimeTyped::typeIdFromTypeName )
		.staticmethod( "typeIdFromTypeName" )
		.def( "typeNameFromTypeId", &RunTimeTyped::typeNameFromTypeId )
		.staticmethod( "typeNameFromTypeId" )		
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( RunTimeTyped )
	;
	
	INTRUSIVE_PTR_PATCH( RunTimeTyped, RunTimeTypedPyClass );

	implicitly_convertible<RunTimeTypedPtr, RefCountedPtr>();
	implicitly_convertible<RunTimeTypedPtr, ConstRunTimeTypedPtr>();

}

}
