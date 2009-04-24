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

#ifndef IE_COREPYTHON_RUNTIMETYPEDBINDING_INL
#define IE_COREPYTHON_RUNTIMETYPEDBINDING_INL

#include "boost/algorithm/string/find.hpp"

#include "IECore/bindings/RefCountedBinding.h"
#include "IECore/RunTimeTyped.h"

namespace IECore
{

namespace Detail
{

static const char *nameWithoutNamespace( const char *name )
{
	boost::iterator_range<const char *> r = boost::find_last( name, ":" );
	if( !r )
	{
		return name;
	}
	return r.end();
}

} // namespace Detail

template<typename T, typename Ptr>
RunTimeTypedClass<T, Ptr>::RunTimeTypedClass( const char *docString )
	:	BaseClass( Detail::nameWithoutNamespace( T::staticTypeName() ), docString )
{
	
	BaseClass::BaseClass::def( "staticTypeName", &T::staticTypeName );
	BaseClass::BaseClass::staticmethod( "staticTypeName" );

	BaseClass::BaseClass::def( "staticTypeId", &T::staticTypeId );
	BaseClass::BaseClass::staticmethod( "staticTypeId" );

	BaseClass::BaseClass::def( "baseTypeId", ( TypeId (*)( TypeId ) )( &RunTimeTyped::baseTypeId ) );
	BaseClass::BaseClass::def( "baseTypeId", ( IECore::TypeId (*)() )&T::baseTypeId );
	BaseClass::BaseClass::staticmethod( "baseTypeId" );

	BaseClass::BaseClass::def( "baseTypeName", &T::baseTypeName );
	BaseClass::BaseClass::staticmethod( "baseTypeName" );

	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( const char * ) )&T::inheritsFrom );
	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( IECore::TypeId ) )&T::inheritsFrom );
	BaseClass::BaseClass::staticmethod( "inheritsFrom" );

}

} // namespace IECore

#endif // IE_COREPYTHON_RUNTIMETYPEDBINDING_INL
