//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/ObjectBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILLock.h"

#include "IECore/MurmurHash.h"
#include "IECore/Object.h"

#include <cassert>

using namespace boost::python;
using namespace IECore;

namespace
{

void registerType( TypeId typeId, const std::string &typeName, object creator )
{
	if( creator != object() )
	{
		Object::registerType(
			typeId, typeName,
			[creator]() -> ObjectPtr
			{
				IECorePython::ScopedGILLock gilLock;
				return extract<ObjectPtr>( creator() );
			}
		);
	}
	else
	{
		Object::registerType( typeId, typeName );
	}
}

ObjectPtr loadWrapper( ConstIndexedIOPtr ioInterface, const IndexedIO::EntryID &name, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	return Object::load( ioInterface, name, canceller );
}

} // namespace

namespace IECorePython
{

void bindObject()
{

	RunTimeTypedClass<Object>()
		.def( self == self )
		.def( self != self )
		.def( "copy", &Object::copy )
		.def( "copyFrom", (void (Object::*)( const Object * ) )&Object::copyFrom )
		.def( "isType", (bool (*)( const std::string &) )&Object::isType )
		.def( "isType", (bool (*)( TypeId) )&Object::isType )
		.staticmethod( "isType" )
		.def( "isAbstractType", (bool (*)( const std::string &) )&Object::isAbstractType )
		.def( "isAbstractType", (bool (*)( TypeId ) )&Object::isAbstractType )
		.staticmethod( "isAbstractType" )
		.def( "create", (ObjectPtr (*)( const std::string &) )&Object::create )
		.def( "create", (ObjectPtr (*)( TypeId ) )&Object::create )
		.staticmethod( "create" )
		.def( "load", loadWrapper, ( arg( "ioInterface" ), arg( "name" ), arg( "canceller" ) = object() ) )
		.staticmethod( "load" )
		.def( "save", (void (Object::*)( IndexedIOPtr, const IndexedIO::EntryID & )const )&Object::save )
		.def( "memoryUsage", (size_t (Object::*)()const )&Object::memoryUsage, "Returns the number of bytes this instance occupies in memory" )
		.def( "hash", (MurmurHash (Object::*)() const)&Object::hash )
		.def( "hash", (void (Object::*)( MurmurHash & ) const)&Object::hash )
		.def( "registerType", registerType, ( arg( "typeId" ), arg( "typeName" ), arg( "creator" ) = object() ) )
		.staticmethod( "registerType" )
	;

}

}
